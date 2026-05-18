#include "lan9252.h"
#include "lan9252_regs.h"
#include "fsmc_drv.h"
#include "board_delay.h"
#include "apm32f4xx.h"   // GPIO, EXTI, NVIC
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_rcm.h"
#include "apm32f4xx_syscfg.h"
#include "apm32f4xx_eint.h"
#include "apm32f4xx_misc.h"

/* 硬件复位引脚定义（根据实际修改） */
//#define LAN9252_RST_PORT   GPIO
//#define LAN9252_RST_PIN    GPIO_PIN_

/* 中断引脚定义 */
#define LAN9252_IRQ_PORT   GPIOB
#define LAN9252_IRQ_PIN    GPIO_PIN_0
#define LAN9252_IRQ_LINE   EINT_LINE_0
#define LAN9252_IRQ_IRQn   EINT0_IRQn

/* 静态函数：硬件复位 */
/*
static void lan9252_hard_reset(void)
{
    GPIO_InitTypeDef gpio_init;
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIO);
    gpio_init.pin = LAN9252_RST_PIN;
    gpio_init.mode = GPIO_MODE_OUT;
    gpio_init.speed = GPIO_SPEED_2MHz;
    gpio_init.otype = GPIO_OTYPE_PP;
    gpio_init.pupd = GPIO_PUPD_UP;
    GPIO_Config(LAN9252_RST_PORT, &gpio_init);

    GPIO_WriteBit(LAN9252_RST_PORT, LAN9252_RST_PIN, Bit_RESET);
    BOARD_DelayMs(10);
    GPIO_WriteBit(LAN9252_RST_PORT, LAN9252_RST_PIN, Bit_SET);
    BOARD_DelayMs(20);
}
*/

/* 变址模式读写内部寄存器（使用组0） */
void lan9252_idx_write_reg(uint32_t reg_addr, uint32_t value)
{
    lan9252_write_HalfWord(HBI_IDX_0, (uint16_t)reg_addr);
    lan9252_write_Word(HBI_DATA_0, value);
}

uint32_t lan9252_idx_read_reg(uint32_t reg_addr)
{
    lan9252_write_HalfWord(HBI_IDX_0, (uint16_t)reg_addr);
    return lan9252_read_Word(HBI_DATA_0);
}

/* EtherCAT CSR 间接访问 */
uint32_t lan9252_ecat_csr_read(uint16_t csr_addr, uint8_t size)
{
    uint32_t cmd = (1U << 31) | (1 << 30) | ((size & 0x7) << 16) | csr_addr;
    lan9252_idx_write_reg(ECAT_CSR_CMD_REG, cmd);
    while (lan9252_idx_read_reg(ECAT_CSR_CMD_REG) & (1U << 31));
    return lan9252_idx_read_reg(ECAT_CSR_DATA_REG);
}

void lan9252_ecat_csr_write(uint16_t csr_addr, uint32_t value, uint8_t size)
{
    lan9252_idx_write_reg(ECAT_CSR_DATA_REG, value);
    uint32_t cmd = (1U << 31) | (0 << 30) | ((size & 0x7) << 16) | csr_addr;
    lan9252_idx_write_reg(ECAT_CSR_CMD_REG, cmd);
    while (lan9252_idx_read_reg(ECAT_CSR_CMD_REG) & (1U << 31));
}

/* 过程数据批量读写 */
/*
int lan9252_pram_write(uint16_t dst_addr, uint8_t *src_data, uint16_t length)
{
    // 注意：需要实现完整的 FIFO 握手逻辑
    return 0;
}

int lan9252_pram_read(uint16_t src_addr, uint8_t *dst_data, uint16_t length)
{
    return 0;
}
*/

/* LAN9252 主初始化函数 */
int lan9252_init(void)
{
    uint32_t ready, timeout;

//    lan9252_hard_reset();
    SMC_LAN9252_Init();

    /* 验证 BYTE_TEST */
    uint32_t test_val = lan9252_idx_read_reg(BYTE_TEST_REG);
    if (test_val != 0x87654321 && test_val != 0x43218765) {
		return -1;
    }

    /* 配置 HBI 字节序（小端） */
    lan9252_write_Word(HBI_CFG, 0x0000);

    /* 等待 READY */
    timeout = 1000;
    do {
        ready = (lan9252_idx_read_reg(HW_CFG_REG) >> 27) & 1;
        BOARD_DelayMs(1);
    } while ((!ready) && (--timeout));
    if (!ready) return -1;

    /* 设置 PDI 模式：HBI 变址 16 位 (0x8D) */
    lan9252_ecat_csr_write(0x0140, 0x8D, 1);

    /* 配置中断寄存器（推挽，高有效） */
    uint32_t irq_cfg = (1 << 8) | (1 << 4) | (1 << 0);
    lan9252_idx_write_reg(IRQ_CFG_REG, irq_cfg);
    lan9252_idx_write_reg(INT_EN_REG, (1 << 0) | (1 << 26) | (1 << 27));
    lan9252_idx_write_reg(INT_STS_REG, 0xFFFFFFFF);

    /* 请求进入预运行状态 */
    lan9252_ecat_csr_write(0x0120, 0x02, 2);
    timeout = 100;
    do {
        uint32_t al_status = lan9252_ecat_csr_read(0x0130, 2);
        if ((al_status & 0x0F) == 0x02) break;
        BOARD_DelayMs(1);
    } while (--timeout);

    /* 看门狗初始化 */
    lan9252_ecat_csr_write(0x0410, 0x2710, 2);
    lan9252_ecat_csr_write(0x0420, 0x2710, 2);

    return 0;
}

/* 中断服务统一入口（由外部中断调用） */
void lan9252_irq_handler(void)
{
    uint32_t int_sts = lan9252_idx_read_reg(INT_STS_REG);

    if (int_sts & (1 << 0)) {
        uint32_t al_event = lan9252_ecat_csr_read(0x0220, 4);
        if (al_event & (1 << 8)) {
            /* SyncManager 0 事件：输出数据到达 */
            // 读取过程数据...
        }
        if (al_event & (1 << 9)) {
            /* SyncManager 1 事件：输入数据已发送 */
            // 更新输入数据...
        }
        /* 清除 AL 事件 */
        lan9252_ecat_csr_read(0x0120, 2);
    }

    /* 清除中断状态（写1清零） */
    lan9252_idx_write_reg(INT_STS_REG, int_sts);
}

/* 配置外部中断（IRQ 引脚） */
void lan9252_irq_config(void)
{
    GPIO_Config_T gpio_init;
    EINT_Config_T exti_init;

    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOB);
    gpio_init.pin = LAN9252_IRQ_PIN;
    gpio_init.mode = GPIO_MODE_IN;
    gpio_init.pupd = GPIO_PUPD_UP;
    GPIO_Config(LAN9252_IRQ_PORT, &gpio_init);

    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_SYSCFG);
    SYSCFG_ConfigEINTLine(SYSCFG_PORT_GPIOB, SYSCFG_PIN_0);

    exti_init.line = LAN9252_IRQ_LINE;
    exti_init.mode = EINT_MODE_INTERRUPT;
    exti_init.trigger = EINT_TRIGGER_FALLING;
    exti_init.lineCmd = ENABLE;
    EINT_Config(&exti_init);

    NVIC_EnableIRQ(LAN9252_IRQ_IRQn);
    NVIC_SetPriority(LAN9252_IRQ_IRQn, 5);
}
