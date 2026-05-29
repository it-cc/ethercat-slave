/*!
 * @file        main.c
 *
 * @brief       Main program body
 *
 * @version     V1.0.4
 *
 * @date        2025-02-15
 *
 * @attention
 *
 *  Copyright (C) 2021-2025 Geehy Semiconductor
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

/* Includes ***************************************************************/
#include "main.h"
#include "Board.h"
#include "apm32f4xx.h"
#include "lan9252.h"
#include "application.h"

/* Private includes *******************************************************/

/* Private macro **********************************************************/

/* Private typedef ********************************************************/

/* Private variables ******************************************************/

/* Private function prototypes ********************************************/

/* External variables *****************************************************/

/* External functions *****************************************************/

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 */
int main(void)
{
    /* 系统时钟配置 */
    // SystemClock_Config();

    if (lan9252_init() == 0) {
        /* 初始化成功 */
        lan9252_irq_config();
    } else {
        /* 初始化失败，死循环或报错 */
        while (1);
    }

    /* 初始化应用层 + SOES 协议栈 */
    app_init();

    /* 主循环：周期性轮询协议栈 */
    while (1) {
        ecat_slv();
    }
}
