/**
 * @file        board_delay.h
 *
 * @brief       This file contains definitions for delay function
 *
 * @version     V1.0.0
 *
 * @date        2025-02-15
 *
 * @attention
 *
 *  Copyright (C) 2025 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Define to prevent recursive inclusion */
#ifndef BOARD_DELAY_H
#define BOARD_DELAY_H

#ifdef __cplusplus
  extern "C" {
#endif

/* Includes ***************************************************************/
#include "apm32f4xx.h"
#include "system_apm32f4xx.h"

/* Exported macro *********************************************************/

/* Exported typedef *******************************************************/

/* Exported variables *****************************************************/
extern volatile uint32_t delayCount;

/* Exported function prototypes *******************************************/
void BOARD_DelayConfig(void);
void BOARD_DelayMs(uint32_t count);
void BOARD_DelayUs(uint32_t count);
void BOARD_DelayIRQHandler(void);
uint32_t BOARD_ReadTick(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_DELAY_H */
