#ifndef __FSMC_DRV_H
#define __FSMC_DRV_H

#include <stdint.h>

/* LAN9252 基地址（NE1） */
#define LAN9252_BASE_ADDR   ((uint32_t)0x60000000)

/* FSMC 初始化（16位数据总线） */
void SMC_LAN9252_Init(void);

/* 直接读写 16 位数据 */
uint16_t lan9252_read_HalfWord(uint32_t offset);
void lan9252_write_HalfWord(uint32_t offset, uint16_t data);

/* 读写 32 位数据（分两次16位操作） */
uint32_t lan9252_read_Word(uint32_t offset);
void lan9252_write_Word(uint32_t offset, uint32_t data);

#endif /* __FSMC_DRV_H */
