#include "fsmc_drv.h"
#include "apm32f4xx.h"
#include "apm32f4xx_rcm.h"
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_smc.h"

/* 静态函数：配置 FSMC 引脚 */
static void SMC_GPIO_Config(void)
{
    GPIO_Config_T gpio_init;
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOD | RCM_AHB1_PERIPH_GPIOE |
                              RCM_AHB1_PERIPH_GPIOF);

    /* 共用配置 */
    gpio_init.mode = GPIO_MODE_AF;
    gpio_init.speed = GPIO_SPEED_50MHz;
    gpio_init.otype = GPIO_OTYPE_PP;
    gpio_init.pupd = GPIO_PUPD_UP;

    /* ========== 端口 D ========== */
    /* 数据线：D0(PD14), D1(PD15), D2(PD0), D3(PD1), D13(PD8), D14(PD9), D15(PD10)
       控制线：NOE(PD4), NWE(PD5), NE1(PD7) */
    gpio_init.pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 |
                    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Config(GPIOD, &gpio_init);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_0, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_1, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_4, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_5, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_7, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_8, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_9, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_10, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_14, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_15, GPIO_AF_SMC);

    /* ========== 端口 E ========== */
    /* 数据线：D4(PE7), D5(PE8), D6(PE9), D7(PE10), D8(PE11), D9(PE12), D10(PE13), D11(PE14), D12(PE15) */
    gpio_init.pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                    GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Config(GPIOE, &gpio_init);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_7, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_8, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_9, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_10, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_11, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_12, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_13, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_14, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_15, GPIO_AF_SMC);

    /* ========== 端口 F ========== */
    /* 地址线 A[4:1]：PF1, PF2, PF3, PF4（A0 不连接） */
    gpio_init.pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_Config(GPIOF, &gpio_init);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_1, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_2, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_3, GPIO_AF_SMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_4, GPIO_AF_SMC);
}

void SMC_LAN9252_Init(void)
{
    SMC_NORSRAMConfig_T smcConfig;
    SMC_NORSRAMTimingConfig_T timing;

	SMC_GPIO_Config();
	
//	RCM_EnableAHB2PeriphClock(RCM_AHB2_PERIPH_SMC);
    RCM->AHB2CLKEN |= (1<<2);
	
    /* 时序配置（168MHz HCLK） */
    timing.addressSetupTime      = 3;
    timing.addressHoldTime       = 1;
    timing.dataSetupTime         = 6;
    timing.busTurnaroundTime     = 1;
    timing.clockDivision         = 0;
    timing.dataLatency           = 0;
    timing.accessMode            = SMC_ACCESS_MODE_A;
	
	SMC_ConfigNORSRAMStructInit(&smcConfig);

    smcConfig.bank                  = SMC_BANK1_NORSRAM_1;
    smcConfig.dataAddressMux        = SMC_DATA_ADDRESS_MUX_DISABLE;
    smcConfig.memoryType            = SMC_MEMORY_TYPE_SRAM;
    smcConfig.memoryDataWidth       = SMC_MEMORY_DATA_WIDTH_16BIT;
    smcConfig.readWriteTimingStruct = &timing;
    smcConfig.writeTimingStruct     = &timing;

    SMC_ConfigNORSRAM(&smcConfig);
    SMC_EnableNORSRAM(SMC_BANK1_NORSRAM_1);
}

/* 直接读写 16 位数据 */
uint16_t lan9252_read_HalfWord(uint32_t offset)
{
    return *(volatile uint16_t *)(LAN9252_BASE_ADDR + offset);
}

void lan9252_write_HalfWord(uint32_t offset, uint16_t data)
{
    *(volatile uint16_t *)(LAN9252_BASE_ADDR + offset) = data;
}

uint32_t lan9252_read_Word(uint32_t offset)
{
    return (uint32_t)lan9252_read_HalfWord(offset) |
           ((uint32_t)lan9252_read_HalfWord(offset + 2) << 16);
}

void lan9252_write_Word(uint32_t offset, uint32_t data)
{
    lan9252_write_HalfWord(offset,     (uint16_t)(data));
    lan9252_write_HalfWord(offset + 2, (uint16_t)(data >> 16));
}
