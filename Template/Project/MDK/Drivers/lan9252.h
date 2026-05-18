#ifndef __LAN9252_H
#define __LAN9252_H

#include <stdint.h>

/* 初始化 LAN9252（16位变址模式） */
int lan9252_init(void);

/* 变址模式读写内部寄存器（32位） */
void lan9252_idx_write_reg(uint32_t reg_addr, uint32_t value);
uint32_t lan9252_idx_read_reg(uint32_t reg_addr);

/* EtherCAT CSR 间接访问 */
uint32_t lan9252_ecat_csr_read(uint16_t csr_addr, uint8_t size);
void lan9252_ecat_csr_write(uint16_t csr_addr, uint32_t value, uint8_t size);

/* 过程数据批量读写（可选） */
int lan9252_pram_write(uint16_t dst_addr, uint8_t *src_data, uint16_t length);
int lan9252_pram_read(uint16_t src_addr, uint8_t *dst_data, uint16_t length);

/* 中断处理（由外部中断服务函数调用） */
void lan9252_irq_handler(void);

/* 配置 IRQ 引脚（外部中断） */
void lan9252_irq_config(void);

#endif /* __LAN9252_H */
