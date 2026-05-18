// lan9252_hal.c
#include "lan9252.h"
#include "fsmc_drv.h"
#include <string.h>

/* ========== EEPROM 读写函数（供 esc_eep.c 使用）========== */
/* 注意：函数名必须与 esc_eep.c 中的 extern 声明一致 */

int EEP_read(uint16_t addr, uint8_t *data, uint16_t len)
{
    // TODO: 实现实际的 EEPROM 读取（通过 LAN9252 的 EEPROM 控制寄存器）
    // 如果暂时不需要 EEPROM 功能，先返回 0 让编译通过
    (void)addr;
    (void)data;
    (void)len;
    return 0;
}

int EEP_write(uint16_t addr, uint8_t *data, uint16_t len)
{
    // TODO: 实现实际的 EEPROM 写入
    (void)addr;
    (void)data;
    (void)len;
    return 0;
}
