/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 *
 * Modified for Keil/ARMCC compiler compatibility
 */

#ifndef CC_H
#define CC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* ========== Keil 兼容的打包和对齐宏 ========== */
#define CC_PACKED_BEGIN
#define CC_PACKED_END
#define CC_PACKED               /* Keil 使用 __packed */
#define C_PACKED                /* SOES 的 ecs.h 使用 C_PACKED */
#define CC_ALIGNED(n)         /* Keil 使用 __align */

/* ========== 断言宏 ========== */
#define CC_ASSERT(exp) assert(exp)

/* ========== 静态断言 ========== */
/* Keil C99 模式使用编译期检查，不使用 _Static_assert */
#define C_STATIC_ASSERT(exp, msg) typedef char static_assert_failed_##__LINE__[(exp) ? 1 : -1]
#define CC_STATIC_ASSERT(exp, msg) typedef char static_assert_failed_##__LINE__[(exp) ? 1 : -1]

/* ========== 弃用标记 ========== */
#define CC_DEPRECATED   __declspec(deprecated)

/* ========== 字节交换函数（替代 GCC built-in）========== */
static inline uint32_t cc_bswap32(uint32_t x)
{
    return ((x >> 24) & 0x000000FF) |
           ((x >> 8)  & 0x0000FF00) |
           ((x << 8)  & 0x00FF0000) |
           ((x << 24) & 0xFF000000);
}

static inline uint16_t cc_bswap16(uint16_t x)
{
    return ((x >> 8) & 0x00FF) |
           ((x << 8) & 0xFF00);
}

#define CC_SWAP32(x) cc_bswap32(x)
#define CC_SWAP16(x) cc_bswap16(x)

/* ========== 原子操作（简易版本，禁用中断或使用 Keil 的 __atomic 扩展）========== */
/* 注意：如果不需要多任务/中断安全，这些宏可以简单实现 */
/* 如果需要真正的原子操作，可以使用 CMSIS 的 __disable_irq/__enable_irq */

#define CC_ATOMIC_SET(var,val)   do { var = (val); } while(0)
#define CC_ATOMIC_GET(var)       (var)
#define CC_ATOMIC_ADD(var,val)   do { var += (val); } while(0)
#define CC_ATOMIC_SUB(var,val)   do { var -= (val); } while(0)
#define CC_ATOMIC_AND(var,val)   do { var &= (val); } while(0)
#define CC_ATOMIC_OR(var,val)    do { var |= (val); } while(0)

/* 如果需要更安全的原子操作（例如在 RTOS 中），可以这样实现： */
/*
#define CC_ATOMIC_SET(var,val)   do { __disable_irq(); var = (val); __enable_irq(); } while(0)
#define CC_ATOMIC_GET(var)       (var)
#define CC_ATOMIC_ADD(var,val)   do { __disable_irq(); var += (val); __enable_irq(); } while(0)
#define CC_ATOMIC_SUB(var,val)   do { __disable_irq(); var -= (val); __enable_irq(); } while(0)
#define CC_ATOMIC_AND(var,val)   do { __disable_irq(); var &= (val); __enable_irq(); } while(0)
#define CC_ATOMIC_OR(var,val)   do { __disable_irq(); var |= (val); __enable_irq(); } while(0) 
*/

/* ========== 字节序处理 ========== */
/* 默认使用小端模式（ARM Cortex-M 通常是小端） */
#define EC_LITTLE_ENDIAN

/* 字节序转换宏 - 如果 ESC 寄存器需要特殊处理 */
#if defined(EC_LITTLE_ENDIAN)
#define htoes(x) ((uint16_t)(x))
#define htoel(x) ((uint32_t)(x))
#elif defined(EC_BIG_ENDIAN)
#define htoes(x) CC_SWAP16((uint16_t)(x))
#define htoel(x) CC_SWAP32((uint32_t)(x))
#else
#error "Endianness not defined"
#endif

#define etohs(x) htoes(x)
#define etohl(x) htoel(x)

/* ========== 调试输出宏 ========== */
#ifdef ESC_DEBUG

#ifdef __RTK__  
#include <rtthread.h>
#define DPRINT(...) rt_kprintf("soes: " __VA_ARGS__)
#else
/* Keil 标准输出，需要重定向 fputc */
#include <stdio.h>
#define DPRINT(...) printf("soes: " __VA_ARGS__)
#endif
#else
#define DPRINT(...)
#endif

/* ========== 必需的函数声明 ========== */
/* CRC 计算函数 - 需要在 cc.c 中实现 */
uint16_t ccitt_crc16(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* CC_H */
