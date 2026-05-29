#include "esc_coe.h"
#include "esc.h"

/* ========================================================================= */
/*                           过程数据变量                                      */
/* ========================================================================= */
/* 内存布局必须与协议定义的 PDO 字节偏移一致：
 *   RxPDO (主→从): servo[0..3] 共 4 字节
 *   TxPDO (从→主): acc_x, acc_y, acc_z 共 6 字节
 */

/* 舵机角度（主站写入） */
uint8_t servo[4];

/* IMU 加速度（从站写入） */
int16_t acc_x;
int16_t acc_y;
int16_t acc_z;

/* ========================================================================= */
/*                     CiA 301 标准对象 (0x1000 - 0x100A)                      */
/* ========================================================================= */

static const _objd obj1000[] = {
    {0, DTYPE_UNSIGNED32, 32, ATYPE_RO, "Device type", 0x00000000, NULL}
};

static uint8_t error_register;
static const _objd obj1001[] = {
    {0, DTYPE_UNSIGNED8, 8, ATYPE_RO, "Error register", 0, &error_register}
};

static const _objd obj1008[] = {
    {0, DTYPE_VISIBLE_STRING, 88, ATYPE_RO, "Device name", 0, (void*)"APM32 Slave"}
};

static const _objd obj1009[] = {
    {0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, "Hardware version", 0, (void*)"1.0"}
};

static const _objd obj100A[] = {
    {0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, "Software version", 0, (void*)"1.0"}
};

/* ========================================================================= */
/*            输出对象 0x7000 - Servo (RxPDO 映射, 主站→从站)                   */
/* ========================================================================= */

static const _objd obj7000[] = {
    {0, DTYPE_UNSIGNED8, 8,  ATYPE_RO,              "Number of entries", 4, NULL},
    {1, DTYPE_UNSIGNED8, 8,  ATYPE_RW | ATYPE_RXPDO, "Servo 1",          0, &servo[0]},
    {2, DTYPE_UNSIGNED8, 8,  ATYPE_RW | ATYPE_RXPDO, "Servo 2",          0, &servo[1]},
    {3, DTYPE_UNSIGNED8, 8,  ATYPE_RW | ATYPE_RXPDO, "Servo 3",          0, &servo[2]},
    {4, DTYPE_UNSIGNED8, 8,  ATYPE_RW | ATYPE_RXPDO, "Servo 4",          0, &servo[3]},
};

/* ========================================================================= */
/*            输入对象 0x6000 - IMU (TxPDO 映射, 从站→主站)                     */
/* ========================================================================= */

static const _objd obj6000[] = {
    {0, DTYPE_UNSIGNED8, 8,  ATYPE_RO,               "Number of entries", 3, NULL},
    {1, DTYPE_INTEGER16, 16, ATYPE_RO | ATYPE_TXPDO, "Acc X",             0, &acc_x},
    {2, DTYPE_INTEGER16, 16, ATYPE_RO | ATYPE_TXPDO, "Acc Y",             0, &acc_y},
    {3, DTYPE_INTEGER16, 16, ATYPE_RO | ATYPE_TXPDO, "Acc Z",             0, &acc_z},
};

/* ========================================================================= */
/*            RxPDO 映射对象 0x1600 (主站→从站)                                 */
/* ========================================================================= */
/* 映射值: (index << 16) | (subindex << 8) | bitlength */

static const _objd obj1600[] = {
    {0, DTYPE_UNSIGNED8,  8,  ATYPE_RW,   "Number of mapped objects", 4, NULL},
    {1, DTYPE_UNSIGNED32, 32, ATYPE_RXPDO, "Mapping 1", 0x70000108},  /* servo[0] 8bit */
    {2, DTYPE_UNSIGNED32, 32, ATYPE_RXPDO, "Mapping 2", 0x70000208},  /* servo[1] 8bit */
    {3, DTYPE_UNSIGNED32, 32, ATYPE_RXPDO, "Mapping 3", 0x70000308},  /* servo[2] 8bit */
    {4, DTYPE_UNSIGNED32, 32, ATYPE_RXPDO, "Mapping 4", 0x70000408},  /* servo[3] 8bit */
};

/* ========================================================================= */
/*            TxPDO 映射对象 0x1A00 (从站→主站)                                 */
/* ========================================================================= */

static const _objd obj1A00[] = {
    {0, DTYPE_UNSIGNED8,  8,  ATYPE_RW,   "Number of mapped objects", 3, NULL},
    {1, DTYPE_UNSIGNED32, 32, ATYPE_TXPDO, "Mapping 1", 0x60000110},  /* acc_x 16bit */
    {2, DTYPE_UNSIGNED32, 32, ATYPE_TXPDO, "Mapping 2", 0x60000210},  /* acc_y 16bit */
    {3, DTYPE_UNSIGNED32, 32, ATYPE_TXPDO, "Mapping 3", 0x60000310},  /* acc_z 16bit */
};

/* ========================================================================= */
/*            SM 赋值对象                                                     */
/* ========================================================================= */

static const _objd obj1C12[] = {
    {0, DTYPE_UNSIGNED8,  8, ATYPE_RW, "Number of assigned PDOs", 1, NULL},
    {1, DTYPE_UNSIGNED16, 16, ATYPE_RW, "PDO assignment", 0x1600, NULL},
};

static const _objd obj1C13[] = {
    {0, DTYPE_UNSIGNED8,  8, ATYPE_RW, "Number of assigned PDOs", 1, NULL},
    {1, DTYPE_UNSIGNED16, 16, ATYPE_RW, "PDO assignment", 0x1A00, NULL},
};

/* ========================================================================= */
/*                           主对象列表                                        */
/* ========================================================================= */

const _objectlist SDOobjects[] = {
    {0x1000, OTYPE_VAR,    0x01, 0, "Device type",      obj1000},
    {0x1001, OTYPE_VAR,    0x01, 0, "Error register",   obj1001},
    {0x1008, OTYPE_VAR,    0x01, 0, "Device name",      obj1008},
    {0x1009, OTYPE_VAR,    0x01, 0, "Hardware version", obj1009},
    {0x100A, OTYPE_VAR,    0x01, 0, "Software version", obj100A},

    {0x1600, OTYPE_RECORD, 0x04, 0, "RxPDO mapping",   obj1600},
    {0x1A00, OTYPE_RECORD, 0x03, 0, "TxPDO mapping",   obj1A00},

    {0x1C12, OTYPE_ARRAY,  0x01, 0, "RxPDO assign",    obj1C12},
    {0x1C13, OTYPE_ARRAY,  0x01, 0, "TxPDO assign",    obj1C13},

    {0x6000, OTYPE_RECORD, 0x03, 0, "Inputs",          obj6000},
    {0x7000, OTYPE_RECORD, 0x04, 0, "Outputs",         obj7000},

    {0xFFFF, 0x0000, 0x00, 0, NULL, NULL}
};
