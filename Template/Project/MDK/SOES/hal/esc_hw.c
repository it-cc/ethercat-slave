/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * ESC hardware layer functions for LAN9252.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 * 
 * Modified for APM32 + LAN9252 with FSMC interface
 */

#include "esc.h"
#include "lan9252.h"
#include "lan9252_regs.h"
#include "fsmc_drv.h"
#include <string.h>

/* BIT 宏定义 */
#ifndef BIT
#define BIT(x) (1UL << (x))
#endif

/* ========== LAN9252 寄存器定义 ========== */
#define ESC_PRAM_RD_FIFO_REG     0x000
#define ESC_PRAM_WR_FIFO_REG     0x020
#define ESC_PRAM_RD_ADDR_LEN_REG 0x308
#define ESC_PRAM_RD_CMD_REG      0x30C
#define ESC_PRAM_WR_ADDR_LEN_REG 0x310
#define ESC_PRAM_WR_CMD_REG      0x314

#define ESC_PRAM_CMD_BUSY        BIT(31)
#define ESC_PRAM_CMD_ABORT       BIT(30)

#define ESC_PRAM_CMD_CNT(x)      ((x >> 8) & 0x1F)
#define ESC_PRAM_CMD_AVAIL       BIT(0)

#define ESC_PRAM_SIZE(x)         ((x) << 16)
#define ESC_PRAM_ADDR(x)         ((x) << 0)

#define ESC_CSR_DATA_REG         0x300
#define ESC_CSR_CMD_REG          0x304

#define ESC_CSR_CMD_BUSY         BIT(31)
#define ESC_CSR_CMD_READ         (BIT(31) | BIT(30))
#define ESC_CSR_CMD_WRITE        BIT(31)
#define ESC_CSR_CMD_SIZE(x)      ((x) << 16)

#define ESC_RESET_CTRL_REG       0x1F8
#define ESC_RESET_CTRL_RST       BIT(6)

/* ========== 内部辅助函数 ========== */

/* 使用你的驱动进行 32 位读写 */
static inline void lan9252_write_32(uint16_t address, uint32_t val)
{
    lan9252_write_Word((uint32_t)address, val);
}

static inline uint32_t lan9252_read_32(uint16_t address)
{
    return lan9252_read_Word((uint32_t)address);
}

/* 缓冲区读写 - 使用你的驱动 */
static void lan9252_read_buffer(uint16_t address, uint8_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t aligned_len = len & ~3;
    uint32_t remaining = len & 3;
    
    /* 按 32 位读取 */
    for (i = 0; i < aligned_len; i += 4) {
        uint32_t val = lan9252_read_32(address + i);
        memcpy(buf + i, &val, 4);
    }
    
    /* 处理剩余字节 */
    if (remaining > 0) {
        uint32_t val = lan9252_read_32(address + aligned_len);
        memcpy(buf + aligned_len, &val, remaining);
    }
}

static void lan9252_write_buffer(uint16_t address, uint8_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t aligned_len = len & ~3;
    uint32_t remaining = len & 3;
    
    /* 按 32 位写入 */
    for (i = 0; i < aligned_len; i += 4) {
        uint32_t val;
        memcpy(&val, buf + i, 4);
        lan9252_write_32(address + i, val);
    }
    
    /* 处理剩余字节 */
    if (remaining > 0) {
        uint32_t val = 0;
        memcpy(&val, buf + aligned_len, remaining);
        lan9252_write_32(address + aligned_len, val);
    }
}

/* ========== ESC CSR 读写函数 ========== */

/* ESC read CSR function */
static void ESC_read_csr(uint16_t address, void *buf, uint16_t len)
{
    uint32_t value;
    uint32_t timeout = 10000;

    value = ESC_CSR_CMD_READ;
    value |= (uint32_t)ESC_CSR_CMD_SIZE(len);
    value |= address;
    lan9252_write_32(ESC_CSR_CMD_REG, value);

    do {
        value = lan9252_read_32(ESC_CSR_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_read_csr timeout! addr=0x%04X\n", address);
            break;
        }
    } while (value & ESC_CSR_CMD_BUSY);

    value = lan9252_read_32(ESC_CSR_DATA_REG);
    memcpy(buf, (uint8_t *)&value, len);
}

/* ESC write CSR function */
static void ESC_write_csr(uint16_t address, void *buf, uint16_t len)
{
    uint32_t value;
    uint32_t timeout = 10000;

    memcpy((uint8_t*)&value, buf, len);
    lan9252_write_32(ESC_CSR_DATA_REG, value);
    
    value = ESC_CSR_CMD_WRITE;
    value |= (uint32_t)ESC_CSR_CMD_SIZE(len);
    value |= address;
    lan9252_write_32(ESC_CSR_CMD_REG, value);

    do {
        value = lan9252_read_32(ESC_CSR_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_write_csr timeout! addr=0x%04X\n", address);
            break;
        }
    } while (value & ESC_CSR_CMD_BUSY);
}

/* ========== ESC PRAM 读写函数 ========== */

/* ESC read process data ram function */
static void ESC_read_pram(uint16_t address, void *buf, uint16_t len)
{
    uint32_t value;
    uint8_t *temp_buf = (uint8_t *)buf;
    uint16_t byte_offset = 0;
    uint8_t fifo_cnt, first_byte_position, temp_len;
    uint32_t timeout = 10000;

    /* Abort any pending command */
    value = ESC_PRAM_CMD_ABORT;
    lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

    do {
        value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_read_pram abort timeout!\n");
            return;
        }
    } while (value & ESC_PRAM_CMD_BUSY);

    /* Set address and length */
    value = (uint32_t)(ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address));
    lan9252_write_32(ESC_PRAM_RD_ADDR_LEN_REG, value);

    /* Start read command */
    value = ESC_PRAM_CMD_BUSY;
    lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

    /* Wait for data available */
    do {
        value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_read_pram data avail timeout!\n");
            return;
        }
    } while ((value & ESC_PRAM_CMD_AVAIL) == 0);

    /* FIFO count */
    fifo_cnt = (uint8_t)ESC_PRAM_CMD_CNT(value);

    /* Read first value from FIFO */
    value = lan9252_read_32(ESC_PRAM_RD_FIFO_REG);
    fifo_cnt--;

    /* Find out first byte position */
    first_byte_position = (address & 0x03);
    temp_len = ((4 - first_byte_position) > len) ? (uint8_t)len : (uint8_t)(4 - first_byte_position);

    memcpy(temp_buf, ((uint8_t *)&value + first_byte_position), temp_len);
    len = (uint16_t)(len - temp_len);
    byte_offset = (uint16_t)(byte_offset + temp_len);

    /* Read remaining data */
    if (len > 0) {
        uint32_t remaining_len = len;
        uint32_t read_len = (remaining_len + 3) & ~3;
        uint8_t *buffer;
        
        /* 使用静态缓冲区避免 malloc，如果不想用 malloc */
        static uint8_t buffer_static[256];  /* 根据 MAX_RXPDO_SIZE 调整 */
        buffer = buffer_static;
        
        if (read_len > sizeof(buffer_static)) {
            DPRINT("ESC_read_pram: buffer too small!\n");
            return;
        }
        
        lan9252_read_buffer(ESC_PRAM_RD_FIFO_REG, buffer, read_len);
        
        for (uint32_t i = 0; i < read_len && len > 0; i += 4) {
            temp_len = (len > 4) ? 4 : (uint8_t)len;
            memcpy(&value, buffer + i, 4);
            memcpy(temp_buf + byte_offset, &value, temp_len);
            len = (uint16_t)(len - temp_len);
            byte_offset = (uint16_t)(byte_offset + temp_len);
        }
    }
}

/* ESC write process data ram function */
static void ESC_write_pram(uint16_t address, void *buf, uint16_t len)
{
    uint32_t value;
    uint8_t *temp_buf = (uint8_t *)buf;
    uint16_t byte_offset = 0;
    uint8_t first_byte_position, temp_len;
    uint32_t timeout = 10000;

    /* Abort any pending command */
    value = ESC_PRAM_CMD_ABORT;
    lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

    do {
        value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_write_pram abort timeout!\n");
            return;
        }
    } while (value & ESC_PRAM_CMD_BUSY);

    /* Set address and length */
    value = (uint32_t)(ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address));
    lan9252_write_32(ESC_PRAM_WR_ADDR_LEN_REG, value);

    /* Start write command */
    value = ESC_PRAM_CMD_BUSY;
    lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

    /* Wait for FIFO ready */
    do {
        value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
        if (--timeout == 0) {
            DPRINT("ESC_write_pram FIFO ready timeout!\n");
            return;
        }
    } while ((value & ESC_PRAM_CMD_AVAIL) == 0);

    /* Find out first byte position */
    first_byte_position = (address & 0x03);
    temp_len = ((4 - first_byte_position) > len) ? (uint8_t)len : (uint8_t)(4 - first_byte_position);

    memcpy(((uint8_t *)&value + first_byte_position), temp_buf, temp_len);

    /* Write first value to FIFO */
    lan9252_write_32(ESC_PRAM_WR_FIFO_REG, value);

    len = (uint16_t)(len - temp_len);
    byte_offset = (uint16_t)(byte_offset + temp_len);

    /* Write remaining data */
    if (len > 0) {
        uint32_t remaining_len = len;
        uint32_t write_len = (remaining_len + 3) & ~3;
        uint8_t *buffer;
        
        /* 使用静态缓冲区 */
        static uint8_t buffer_static[256];
        buffer = buffer_static;
        
        if (write_len > sizeof(buffer_static)) {
            DPRINT("ESC_write_pram: buffer too small!\n");
            return;
        }
        
        memset(buffer, 0, write_len);
        
        for (uint32_t i = 0; i < write_len && len > 0; i += 4) {
            temp_len = (len > 4) ? 4 : (uint8_t)len;
            memcpy(&value, (temp_buf + byte_offset), temp_len);
            buffer[i] = (uint8_t)(value & 0xFF);
            buffer[i+1] = (uint8_t)((value >> 8) & 0xFF);
            buffer[i+2] = (uint8_t)((value >> 16) & 0xFF);
            buffer[i+3] = (uint8_t)((value >> 24) & 0xFF);
            
            len = (uint16_t)(len - temp_len);
            byte_offset = (uint16_t)(byte_offset + temp_len);
        }
        
        lan9252_write_buffer(ESC_PRAM_WR_FIFO_REG, buffer, write_len);
    }
}

/* ========== ESC 对外接口函数 ========== */

/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read(uint16_t address, void *buf, uint16_t len)
{
    /* Select Read function depending on address, process data ram or not */
    if (address >= 0x1000) {
        ESC_read_pram(address, buf, len);
    } else {
        uint16_t size;
        uint8_t *temp_buf = (uint8_t *)buf;
        uint16_t curr_addr = address;
        uint16_t curr_len = len;

        while (curr_len > 0) {
            /* We read maximum 4 bytes at the time */
            size = (curr_len > 4) ? 4 : curr_len;
            
            /* Make size aligned to address according to LAN9252 datasheet */
            if (curr_addr & BIT(0)) {
                size = 1;
            } else if (curr_addr & BIT(1)) {
                size = (size & BIT(0)) ? 1 : 2;
            } else if (size == 3) {
                size = 1;
            }
            
            ESC_read_csr(curr_addr, temp_buf, size);

            curr_len -= size;
            temp_buf += size;
            curr_addr += size;
        }
    }
    
    /* Read AL Event register to mimic ET1100 behavior */
    ESC_read_csr(ESCREG_ALEVENT, (void *)&ESCvar.ALevent, sizeof(ESCvar.ALevent));
    ESCvar.ALevent = etohs(ESCvar.ALevent);
}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write(uint16_t address, void *buf, uint16_t len)
{
    /* Select Write function depending on address, process data ram or not */
    if (address >= 0x1000) {
        ESC_write_pram(address, buf, len);
    } else {
        uint16_t size;
        uint8_t *temp_buf = (uint8_t *)buf;
        uint16_t curr_addr = address;
        uint16_t curr_len = len;

        while (curr_len > 0) {
            /* We write maximum 4 bytes at the time */
            size = (curr_len > 4) ? 4 : curr_len;
            
            /* Make size aligned to address according to LAN9252 datasheet */
            if (curr_addr & BIT(0)) {
                size = 1;
            } else if (curr_addr & BIT(1)) {
                size = (size & BIT(0)) ? 1 : 2;
            } else if (size == 3) {
                size = 1;
            }
            
            ESC_write_csr(curr_addr, temp_buf, size);

            curr_len -= size;
            temp_buf += size;
            curr_addr += size;
        }
    }

    /* Read AL Event register to mimic ET1100 behavior */
    ESC_read_csr(ESCREG_ALEVENT, (void *)&ESCvar.ALevent, sizeof(ESCvar.ALevent));
    ESCvar.ALevent = etohs(ESCvar.ALevent);
}

/* ESC reset function */
void ESC_reset(void)
{
    /* 使用你的驱动进行软件复位 */
    lan9252_write_32(ESC_RESET_CTRL_REG, ESC_RESET_CTRL_RST);
    
    /* 等待复位完成 */
    uint32_t timeout = 10000;
    uint32_t value;
    do {
        value = lan9252_read_32(ESC_RESET_CTRL_REG);
        if (--timeout == 0) {
            DPRINT("ESC_reset timeout!\n");
            break;
        }
    } while (value & ESC_RESET_CTRL_RST);
}

/* ESC initialization function */
void ESC_init(const esc_cfg_t *config)
{
    /* LAN9252 已经在主函数中初始化，这里只做必要的配置 */
    DPRINT("ESC_init completed\n");
}

/* 实现 __aeabi_assert 函数，解决链接错误 */
__attribute__((weak, noreturn))
void __aeabi_assert(const char *expr, const char *file, int line)
{
    (void)expr;
    (void)file;
    (void)line;
    /* 可以添加串口输出调试信息 */
    while(1);
}
