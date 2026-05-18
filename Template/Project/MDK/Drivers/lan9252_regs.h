#ifndef __LAN9252_REGS_H
#define __LAN9252_REGS_H

/* HBI 变址模式寄存器偏移（字节地址，16位模式下FSMC自动处理） */
#define HBI_IDX_0         0x00
#define HBI_DATA_0        0x04
#define HBI_IDX_1         0x08
#define HBI_DATA_1        0x0C
#define HBI_IDX_2         0x10
#define HBI_DATA_2        0x14
#define PROC_RAM_FIFO     0x18
#define HBI_CFG           0x1C

/* 可直接寻址的系统 CSR 偏移 */
#define BYTE_TEST_REG     0x64
#define HW_CFG_REG        0x74
#define PMT_CTRL_REG      0x84
#define RESET_CTL_REG     0x1F8
#define IRQ_CFG_REG       0x54
#define INT_STS_REG       0x58
#define INT_EN_REG        0x5C

/* EtherCAT CSR 间接访问寄存器偏移 */
#define ECAT_CSR_DATA_REG 0x300
#define ECAT_CSR_CMD_REG  0x304
#define ECAT_PRAM_RD_ADDR_LEN 0x308
#define ECAT_PRAM_RD_CMD  0x30C
#define ECAT_PRAM_WR_ADDR_LEN 0x310
#define ECAT_PRAM_WR_CMD  0x314

#endif /* __LAN9252_REGS_H */
