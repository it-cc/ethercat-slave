#include "esc.h"
#include "ecat_slv.h"
#include "application.h"

/* 输入/输出数据缓冲区（根据你的实际 PDO 大小） */
static uint8_t input_data[128];
static uint8_t output_data[128];

/* 读取输入数据 - 主站读取从站输入时调用 */
void cb_get_inputs(void)
{
    /* 1. 更新你的实际硬件输入数据到 input_data */
    /* TODO: 读取 GPIO、ADC、编码器等 */
    
    /* 2. 将输入数据写入过程数据 RAM（SM3 区域） */
    /* ESC_write(地址, 数据指针, 长度) */
    ESC_write(0x1180, input_data, sizeof(input_data));
}

/* 设置输出数据 - 主站发送输出到从站时调用 */
void cb_set_outputs(void)
{
    /* 1. 从过程数据 RAM 读取输出数据（SM2 区域） */
    ESC_read(0x1100, output_data, sizeof(output_data));
    
    /* 2. 根据 output_data 控制你的硬件 */
    /* TODO: 设置 GPIO、PWM、电机等 */
}

/* 应用层初始化 - 需要在 main() 中调用 */
void app_init(void)
{
    /* 初始化 ESC 硬件和 SOES 协议栈 */
    ecat_slv_init(NULL);
}
