#include "driverlib.h"
#include "device.h"
#include "Codes/includes/supercap.h"
#include "Codes/includes/buzzer.h"
#include "Codes/includes/telemetry.h"

void main(void)
 {
    uint16_t pin;
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();

    // 初始化期间保持四个栅极输入为低电平。
    GPIO_setPinConfig(GPIO_0_GPIO0);
    GPIO_setPinConfig(GPIO_1_GPIO1);
    GPIO_setPinConfig(GPIO_2_GPIO2);
    GPIO_setPinConfig(GPIO_3_GPIO3);
    for (pin = 0; pin < 4; ++pin) {
        GPIO_setPadConfig(pin, GPIO_PIN_TYPE_STD);
        GPIO_writePin(pin, 0);
        GPIO_setDirectionMode(pin, GPIO_DIR_MODE_OUT);
    }
    Buzzer_Init();
    Telemetry_Init();
    // V2 未使用的风扇输出保持低电平。
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_writePin(16, 0);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);

    Supercap_Init();
    EINT;
    ERTM;
    Buzzer_PlayStartupTone();
    while (1) {
        Supercap_BackgroundPoll();
    }
}
