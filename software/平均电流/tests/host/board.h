#ifndef HOST_BOARD_H
#define HOST_BOARD_H
#include "driverlib.h"
#define PWM_BRIDGE_A_BASE EPWM1_BASE
#define PWM_BRIDGE_B_BASE EPWM2_BASE
#define PWM_ADC_TRIGGER_BASE 2
#define PWM_BUZZER_BASE 3
#define SCI_DEBUG_BASE 4
#define TIMER_BACKGROUND_1KHZ_BASE 6
#define DMA_ADC_CAPTURE_BASE DMA_CH1_BASE
extern const void *AdcDma_Destination;
void DMA_ADC_CAPTURE_init(void);
#define SCI_DEBUG_SCIRX_GPIO 17
#define SCI_DEBUG_init() (sci_initialized = true)
#define ADC_A_RESULT_BASE 0xB00
#define ADC_B_RESULT_BASE 0xB20
#define ADC_C_RESULT_BASE 0xB40
#define ADC_C_BASE 2
#define ADC_A_I_BRIDGE_A 0
#define ADC_A_I_IN 1
#define ADC_B_V_BAT 0
#define ADC_B_V_CAP 1
#define ADC_C_I_BRIDGE_B 0
#define ADC_C_I_CAP 1
#define ADC_C_I_CHASSIS 3
#define PWM_BRIDGE_A_EPWMA_PIN_CONFIG 0
#define PWM_BRIDGE_A_EPWMB_PIN_CONFIG 1
#define PWM_BRIDGE_B_EPWMA_PIN_CONFIG 2
#define PWM_BRIDGE_B_EPWMB_PIN_CONFIG 3
#define ASYSCTL_init() ((void)0)
#define ADC_init() ((void)0)
#include "generated_pwm_init.h"
#endif
