#include "esc.h"
#include "ecat_slv.h"
#include "application.h"

/* ========================================================================= */
/*                    OD 过程数据变量（定义在 objectlist.c）                      */
/* ========================================================================= */
extern uint8_t  servo[4];
extern int16_t  acc_x;
extern int16_t  acc_y;
extern int16_t  acc_z;

/* ========================================================================= */
/*                           ESC 配置                                         */
/* ========================================================================= */

static void app_safe_output_cb(void);

static esc_cfg_t esc_config = {
    .user_arg                  = NULL,
    .use_interrupt             = 1,
    .watchdog_cnt              = 2000,
    .set_defaults_hook         = NULL,
    .pre_state_change_hook     = NULL,
    .post_state_change_hook    = NULL,
    .application_hook          = NULL,
    .safeoutput_override       = app_safe_output_cb,
    .pre_object_download_hook  = NULL,
    .post_object_download_hook = NULL,
    .pre_object_upload_hook    = NULL,
    .post_object_upload_hook   = NULL,
    .rxpdo_override            = NULL,
    .txpdo_override            = NULL,
    .esc_hw_interrupt_enable   = NULL,
    .esc_hw_interrupt_disable  = NULL,
    .esc_hw_eep_handler        = NULL,
    .esc_check_dc_handler      = NULL,
    .get_device_id             = NULL,
};

/* ========================================================================= */
/*                          MPU6050 驱动 (I2C)                                */
/* ========================================================================= */

#define MPU_REG_ACCEL_XOUT_H  0x3B

static void mpu_i2c_init(void)
{
    
}

static void mpu_write_reg(uint8_t reg, uint8_t val)
{
   
}

static void mpu_read_bytes(uint8_t reg, uint8_t *buf, uint8_t len)
{
   
}

static void mpu_init(void)
{
    
}

static void mpu_read_accel(int16_t *x, int16_t *y, int16_t *z)
{
   
}

/* ========================================================================= */
/*                          舵机 PWM 驱动                                     */
/* ========================================================================= */

static void servo_pwm_init(void)
{
    
}

static void servo_set_angle(uint8_t ch, uint8_t angle)
{
    
}

/* ========================================================================= */
/*                        SOES 回调实现                                        */
/* ========================================================================= */

void app_hw_init(void)
{
    mpu_init();
    servo_pwm_init();
    

    
}

void app_init(void)
{
    app_hw_init();
    ecat_slv_init(&esc_config);
}

void cb_get_inputs(void)
{
    mpu_read_accel(&acc_x, &acc_y, &acc_z);
}

void cb_set_outputs(void)
{
    servo_set_angle(0, servo[0]);
    servo_set_angle(1, servo[1]);
    servo_set_angle(2, servo[2]);
    servo_set_angle(3, servo[3]);
}

