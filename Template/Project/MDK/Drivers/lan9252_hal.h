#ifndef __LAN9252_HAL_H
#define __LAN9252_HAL_H

#include <stdint.h>

/* EEPROM 读写函数声明 */
int EEP_read(uint16_t addr, uint8_t *data, uint16_t len);
int EEP_write(uint16_t addr, uint8_t *data, uint16_t len);

#endif
