/*
 * Custom options for SOES protocol stack
 * This file overrides the default settings in options.h
 */

#ifndef __ECAT_OPTIONS_H__
#define __ECAT_OPTIONS_H__

/* ========== 功能开关 ========== */
#define USE_FOE          0    /* File over EtherCAT - 如果不需要可以关闭 */
#define USE_EOE          0    /* Ethernet over EtherCAT - 通常关闭 */

/* ========== 邮箱大小配置 ========== */
#define MBXSIZE          256  /* 邮箱大小，根据你的应用需求调整 */
#define MBXSIZEBOOT      256  /* 启动模式邮箱大小 */
#define MBXBUFFERS       3    /* 邮箱缓冲区数量 */
#define PREALLOC_FACTOR  3    /* 预分配因子 */

/* ========== Sync Manager 配置 ========== */
/* MBX0 - 邮箱输出 (主站 -> 从站) */
#define MBX0_sma         0x1000
#define MBX0_sml         MBXSIZE
#define MBX0_smc         0x26   /* SM 控制：输出邮箱 */

/* MBX1 - 邮箱输入 (从站 -> 主站) */
#define MBX1_sma         MBX0_sma + MBX0_sml
#define MBX1_sml         MBXSIZE
#define MBX1_smc         0x22   /* SM 控制：输入邮箱 */

/* SM2 - 过程数据输出 (主站 -> 从站) */
#define SM2_sma          0x1200
#define SM2_smc          0x24   /* SM 控制：输出过程数据 */
#define SM2_act          1      /* 激活 */

/* SM3 - 过程数据输入 (从站 -> 主站) */
#define SM3_sma          0x1280
#define SM3_smc          0x20   /* SM 控制：输入过程数据 */
#define SM3_act          1      /* 激活 */

/* ========== 动态映射配置 ========== */
#define MAX_MAPPINGS_SM2 8      /* SM2 最大映射数，根据 PDO 数量调整 */
#define MAX_MAPPINGS_SM3 8      /* SM3 最大映射数 */
#define MAX_RXPDO_SIZE   128    /* RxPDO 最大字节数 */
#define MAX_TXPDO_SIZE   128    /* TxPDO 最大字节数 */

#endif /* __ECAT_OPTIONS_H__ */
