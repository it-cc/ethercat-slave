#include "esc_coe.h"
#include "esc.h"

/* ========== 对象描述符（子索引数据）========== */

/* 设备名称 0x1008 */
static const _objd obj1008_sub0 = {
    .subindex = 0,
    .datatype = DTYPE_VISIBLE_STRING,
    .bitlength = 8 * 11,  /* "SOES Slave" 长度 11 字节 */
    .flags = ATYPE_RO,
    .name = "Device name",
    .value = 0,
    .data = (void*)"SOES Slave"
};

/* 硬件版本 0x1009 */
static const _objd obj1009_sub0 = {
    .subindex = 0,
    .datatype = DTYPE_VISIBLE_STRING,
    .bitlength = 8 * 3,   /* "1.0" 长度 3 字节 */
    .flags = ATYPE_RO,
    .name = "Hardware version",
    .value = 0,
    .data = (void*)"1.0"
};

/* 软件版本 0x100A */
static const _objd obj100A_sub0 = {
    .subindex = 0,
    .datatype = DTYPE_VISIBLE_STRING,
    .bitlength = 8 * 3,   /* "1.0" 长度 3 字节 */
    .flags = ATYPE_RO,
    .name = "Software version",
    .value = 0,
    .data = (void*)"1.0"
};

/* 设备类型 0x1000 */
static const _objd obj1000_sub0 = {
    .subindex = 0,
    .datatype = DTYPE_UNSIGNED32,
    .bitlength = 32,
    .flags = ATYPE_RO,
    .name = "Device type",
    .value = 0x00000000,  /* 根据你的设备类型修改 */
    .data = NULL
};

/* 错误寄存器 0x1001 */
static uint8_t error_register = 0;
static const _objd obj1001_sub0 = {
    .subindex = 0,
    .datatype = DTYPE_UNSIGNED8,
    .bitlength = 8,
    .flags = ATYPE_RO,
    .name = "Error register",
    .value = 0,
    .data = &error_register
};

/* ========== 主对象列表 ========== */
const _objectlist SDOobjects[] = {
    /* 设备类型 */
    {0x1000, OTYPE_VAR, 0x01, 0, "Device type", &obj1000_sub0},
    
    /* 错误寄存器 */
    {0x1001, OTYPE_VAR, 0x01, 0, "Error register", &obj1001_sub0},
    
    /* 设备名称 */
    {0x1008, OTYPE_VAR, 0x01, 0, "Device name", &obj1008_sub0},
    
    /* 硬件版本 */
    {0x1009, OTYPE_VAR, 0x01, 0, "Hardware version", &obj1009_sub0},
    
    /* 软件版本 */
    {0x100A, OTYPE_VAR, 0x01, 0, "Software version", &obj100A_sub0},
    
    /* 结束标记 - index 必须为 0xFFFF */
    {0xFFFF, 0x0000, 0x00, 0, NULL, NULL}
};
