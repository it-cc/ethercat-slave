/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include "cc.h"

/**
 * @brief  计算 CRC-16-IBM 校验值
 * @param  data: 数据指针
 * @param  len:  数据长度
 * @return CRC 校验值
 */
uint16_t ccitt_crc16(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0x0000;
    uint16_t poly = 0x8005;
    uint16_t i, j;
    
    for (i = 0; i < len; i++) {
        crc ^= (data[i] << 8);
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}
