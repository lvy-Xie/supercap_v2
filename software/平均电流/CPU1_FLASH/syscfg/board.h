/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef BOARD_H
#define BOARD_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//
// Included Files
//

#include "driverlib.h"
#include "device.h"

//*****************************************************************************
//
// PinMux Configurations
//
//*****************************************************************************

//
// CANA -> CAN_CHASSIS Pinmux
//
//
// CANA_RX - GPIO Settings
//
#define GPIO_PIN_CANA_RX 5
#define CAN_CHASSIS_CANRX_GPIO 5
#define CAN_CHASSIS_CANRX_PIN_CONFIG GPIO_5_CANA_RX
//
// CANA_TX - GPIO Settings
//
#define GPIO_PIN_CANA_TX 4
#define CAN_CHASSIS_CANTX_GPIO 4
#define CAN_CHASSIS_CANTX_PIN_CONFIG GPIO_4_CANA_TX

//
// EPWM1 -> PWM_BRIDGE_A Pinmux
//
//
// EPWM1_A - GPIO Settings
//
#define GPIO_PIN_EPWM1_A 0
#define PWM_BRIDGE_A_EPWMA_GPIO 0
#define PWM_BRIDGE_A_EPWMA_PIN_CONFIG GPIO_0_EPWM1_A
//
// EPWM1_B - GPIO Settings
//
#define GPIO_PIN_EPWM1_B 1
#define PWM_BRIDGE_A_EPWMB_GPIO 1
#define PWM_BRIDGE_A_EPWMB_PIN_CONFIG GPIO_1_EPWM1_B

//
// EPWM2 -> PWM_BRIDGE_B Pinmux
//
//
// EPWM2_A - GPIO Settings
//
#define GPIO_PIN_EPWM2_A 2
#define PWM_BRIDGE_B_EPWMA_GPIO 2
#define PWM_BRIDGE_B_EPWMA_PIN_CONFIG GPIO_2_EPWM2_A
//
// EPWM2_B - GPIO Settings
//
#define GPIO_PIN_EPWM2_B 3
#define PWM_BRIDGE_B_EPWMB_GPIO 3
#define PWM_BRIDGE_B_EPWMB_PIN_CONFIG GPIO_3_EPWM2_B

//
// EPWM3 -> PWM_ADC_TRIGGER Pinmux
//

//
// EPWM4 -> PWM_BUZZER Pinmux
//
//
// EPWM4_A - GPIO Settings
//
#define GPIO_PIN_EPWM4_A 6
#define PWM_BUZZER_EPWMA_GPIO 6
#define PWM_BUZZER_EPWMA_PIN_CONFIG GPIO_6_EPWM4_A

//
// EPWM5 -> PWM_FAN Pinmux
//
//
// EPWM5_A - GPIO Settings
//
#define GPIO_PIN_EPWM5_A 16
#define PWM_FAN_EPWMA_GPIO 16
#define PWM_FAN_EPWMA_PIN_CONFIG GPIO_16_EPWM5_A
//
// GPIO13 - GPIO Settings
//
#define LED_1_GPIO_PIN_CONFIG GPIO_13_GPIO13
//
// GPIO12 - GPIO Settings
//
#define LED_2_GPIO_PIN_CONFIG GPIO_12_GPIO12
//
// GPIO11 - GPIO Settings
//
#define LED_3_GPIO_PIN_CONFIG GPIO_11_GPIO11
//
// GPIO33 - GPIO Settings
//
#define LED_4_GPIO_PIN_CONFIG GPIO_33_GPIO33
//
// GPIO9 - GPIO Settings
//
#define FAN_TACH_GPIO_PIN_CONFIG GPIO_9_GPIO9

//
// SCIA -> SCI_DEBUG Pinmux
//
//
// SCIA_RX - GPIO Settings
//
#define GPIO_PIN_SCIA_RX 17
#define SCI_DEBUG_SCIRX_GPIO 17
#define SCI_DEBUG_SCIRX_PIN_CONFIG GPIO_17_SCIA_RX
//
// SCIA_TX - GPIO Settings
//
#define GPIO_PIN_SCIA_TX 24
#define SCI_DEBUG_SCITX_GPIO 24
#define SCI_DEBUG_SCITX_PIN_CONFIG GPIO_24_SCIA_TX

//*****************************************************************************
//
// ADC Configurations
//
//*****************************************************************************
#define ADC_A_BASE ADCA_BASE
#define ADC_A_RESULT_BASE ADCARESULT_BASE
#define ADC_A_I_BRIDGE_A ADC_SOC_NUMBER0
#define ADC_A_FORCE_I_BRIDGE_A ADC_FORCE_SOC0
#define ADC_A_SAMPLE_WINDOW_I_BRIDGE_A 200
#define ADC_A_TRIGGER_SOURCE_I_BRIDGE_A ADC_TRIGGER_EPWM3_SOCA
#define ADC_A_CHANNEL_I_BRIDGE_A ADC_CH_ADCIN2
#define ADC_A_I_IN ADC_SOC_NUMBER1
#define ADC_A_FORCE_I_IN ADC_FORCE_SOC1
#define ADC_A_SAMPLE_WINDOW_I_IN 200
#define ADC_A_TRIGGER_SOURCE_I_IN ADC_TRIGGER_EPWM3_SOCB
#define ADC_A_CHANNEL_I_IN ADC_CH_ADCIN6
#define ADC_A_TEMP_FET_A ADC_SOC_NUMBER2
#define ADC_A_FORCE_TEMP_FET_A ADC_FORCE_SOC2
#define ADC_A_SAMPLE_WINDOW_TEMP_FET_A 640
#define ADC_A_TRIGGER_SOURCE_TEMP_FET_A ADC_TRIGGER_SW_ONLY
#define ADC_A_CHANNEL_TEMP_FET_A ADC_CH_ADCIN1
void ADC_A_init();

#define ADC_B_BASE ADCB_BASE
#define ADC_B_RESULT_BASE ADCBRESULT_BASE
#define ADC_B_V_BAT ADC_SOC_NUMBER0
#define ADC_B_FORCE_V_BAT ADC_FORCE_SOC0
#define ADC_B_SAMPLE_WINDOW_V_BAT 200
#define ADC_B_TRIGGER_SOURCE_V_BAT ADC_TRIGGER_EPWM3_SOCA
#define ADC_B_CHANNEL_V_BAT ADC_CH_ADCIN2
#define ADC_B_V_CAP ADC_SOC_NUMBER1
#define ADC_B_FORCE_V_CAP ADC_FORCE_SOC1
#define ADC_B_SAMPLE_WINDOW_V_CAP 200
#define ADC_B_TRIGGER_SOURCE_V_CAP ADC_TRIGGER_EPWM3_SOCB
#define ADC_B_CHANNEL_V_CAP ADC_CH_ADCIN4
void ADC_B_init();

#define ADC_C_BASE ADCC_BASE
#define ADC_C_RESULT_BASE ADCCRESULT_BASE
#define ADC_C_I_BRIDGE_B ADC_SOC_NUMBER0
#define ADC_C_FORCE_I_BRIDGE_B ADC_FORCE_SOC0
#define ADC_C_SAMPLE_WINDOW_I_BRIDGE_B 200
#define ADC_C_TRIGGER_SOURCE_I_BRIDGE_B ADC_TRIGGER_EPWM3_SOCA
#define ADC_C_CHANNEL_I_BRIDGE_B ADC_CH_ADCIN1
#define ADC_C_I_CAP ADC_SOC_NUMBER1
#define ADC_C_FORCE_I_CAP ADC_FORCE_SOC1
#define ADC_C_SAMPLE_WINDOW_I_CAP 200
#define ADC_C_TRIGGER_SOURCE_I_CAP ADC_TRIGGER_EPWM3_SOCB
#define ADC_C_CHANNEL_I_CAP ADC_CH_ADCIN10
#define ADC_C_TEMP_L ADC_SOC_NUMBER2
#define ADC_C_FORCE_TEMP_L ADC_FORCE_SOC2
#define ADC_C_SAMPLE_WINDOW_TEMP_L 640
#define ADC_C_TRIGGER_SOURCE_TEMP_L ADC_TRIGGER_SW_ONLY
#define ADC_C_CHANNEL_TEMP_L ADC_CH_ADCIN2
#define ADC_C_I_CHASSIS ADC_SOC_NUMBER3
#define ADC_C_FORCE_I_CHASSIS ADC_FORCE_SOC3
#define ADC_C_SAMPLE_WINDOW_I_CHASSIS 200
#define ADC_C_TRIGGER_SOURCE_I_CHASSIS ADC_TRIGGER_EPWM3_SOCB
#define ADC_C_CHANNEL_I_CHASSIS ADC_CH_ADCIN3
#define ADC_C_TEMP_FET_B ADC_SOC_NUMBER4
#define ADC_C_FORCE_TEMP_FET_B ADC_FORCE_SOC4
#define ADC_C_SAMPLE_WINDOW_TEMP_FET_B 640
#define ADC_C_TRIGGER_SOURCE_TEMP_FET_B ADC_TRIGGER_SW_ONLY
#define ADC_C_CHANNEL_TEMP_FET_B ADC_CH_ADCIN4
void ADC_C_init();


//*****************************************************************************
//
// ASYSCTL Configurations
//
//*****************************************************************************

//*****************************************************************************
//
// CAN Configurations
//
//*****************************************************************************
#define CAN_CHASSIS_BASE CANA_BASE

#define CAN_CHASSIS_MessageObj1_ID 800
#define CAN_CHASSIS_MessageObj2_ID 801
#define CAN_CHASSIS_MessageObj3_ID 802
#define CAN_CHASSIS_MessageObj4_ID 803
void CAN_CHASSIS_init();


//*****************************************************************************
//
// CLA Configurations
//
//*****************************************************************************
#define CLA_ADC_BASE CLA1_BASE

//
// The following are symbols defined in the CLA assembly code
// Including them in the shared header file makes them global
// and the main CPU can make use of them.
//
__attribute__((interrupt)) void Cla1Task1();
__attribute__((interrupt)) void Cla1Task8();
void CLA_ADC_init();


//*****************************************************************************
//
// CMPSS Configurations
//
//*****************************************************************************
#define CMPSS_I_BRIDGE_A_BASE CMPSS1_BASE
#define CMPSS_I_BRIDGE_A_HIGH_COMP_BASE CMPSS1_BASE    
#define CMPSS_I_BRIDGE_A_LOW_COMP_BASE CMPSS1_BASE    
#define CMPSS_I_BRIDGE_A_CMPHPMUX_SELECT ASYSCTL_CMPHPMUX_SELECT_1
#define CMPSS_I_BRIDGE_A_CMPLPMUX_SELECT ASYSCTL_CMPLPMUX_SELECT_1
void CMPSS_I_BRIDGE_A_init();
#define CMPSS_I_BRIDGE_B_BASE CMPSS2_BASE
#define CMPSS_I_BRIDGE_B_HIGH_COMP_BASE CMPSS2_BASE    
#define CMPSS_I_BRIDGE_B_LOW_COMP_BASE CMPSS2_BASE    
#define CMPSS_I_BRIDGE_B_CMPHPMUX_SELECT ASYSCTL_CMPHPMUX_SELECT_2
#define CMPSS_I_BRIDGE_B_CMPLPMUX_SELECT ASYSCTL_CMPLPMUX_SELECT_2
void CMPSS_I_BRIDGE_B_init();
#define CMPSS_V_BAT_BASE CMPSS3_BASE
#define CMPSS_V_BAT_HIGH_COMP_BASE CMPSS3_BASE    
#define CMPSS_V_BAT_LOW_COMP_BASE CMPSS3_BASE    
#define CMPSS_V_BAT_CMPHPMUX_SELECT ASYSCTL_CMPHPMUX_SELECT_3
#define CMPSS_V_BAT_CMPLPMUX_SELECT ASYSCTL_CMPLPMUX_SELECT_3
void CMPSS_V_BAT_init();
#define CMPSS_V_CAP_BASE CMPSS4_BASE
#define CMPSS_V_CAP_HIGH_COMP_BASE CMPSS4_BASE    
#define CMPSS_V_CAP_LOW_COMP_BASE CMPSS4_BASE    
#define CMPSS_V_CAP_CMPHPMUX_SELECT ASYSCTL_CMPHPMUX_SELECT_4
#define CMPSS_V_CAP_CMPLPMUX_SELECT ASYSCTL_CMPLPMUX_SELECT_4
void CMPSS_V_CAP_init();
#define CMPSS_I_CAP_BASE CMPSS7_BASE
#define CMPSS_I_CAP_HIGH_COMP_BASE CMPSS7_BASE    
#define CMPSS_I_CAP_LOW_COMP_BASE CMPSS7_BASE    
#define CMPSS_I_CAP_CMPHPMUX_SELECT ASYSCTL_CMPHPMUX_SELECT_7
#define CMPSS_I_CAP_CMPLPMUX_SELECT ASYSCTL_CMPLPMUX_SELECT_7
void CMPSS_I_CAP_init();

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
#define TIMER_BACKGROUND_1KHZ_BASE CPUTIMER1_BASE
void TIMER_BACKGROUND_1KHZ_init();

//*****************************************************************************
//
// DMA Configurations
//
//*****************************************************************************
#define DMA_ADC_CAPTURE_SRCADDRESS 2816 
extern const void *AdcDma_Destination;
#define DMA_ADC_CAPTURE_BASE DMA_CH1_BASE 
#define DMA_ADC_CAPTURE_BURSTSIZE 4U
#define DMA_ADC_CAPTURE_TRANSFERSIZE 3U
#define DMA_ADC_CAPTURE_SRC_WRAPSIZE 65535U
#define DMA_ADC_CAPTURE_DEST_WRAPSIZE 65535U
void DMA_ADC_CAPTURE_init();

//*****************************************************************************
//
// ECAP Configurations
//
//*****************************************************************************
#define FAN_TACH_CAPTURE_BASE ECAP1_BASE
#define FAN_TACH_CAPTURE_SIGNAL_MUNIT_BASE ECAP1SIGNALMONITORING_BASE
void FAN_TACH_CAPTURE_init();

//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************
#define PWM_BRIDGE_A_BASE EPWM1_BASE
#define PWM_BRIDGE_A_TBPRD 285
#define PWM_BRIDGE_A_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define PWM_BRIDGE_A_TBPHS 0
#define PWM_BRIDGE_A_CMPA 143
#define PWM_BRIDGE_A_CMPB 143
#define PWM_BRIDGE_A_CMPC 0
#define PWM_BRIDGE_A_CMPD 0
#define PWM_BRIDGE_A_DBRED 5
#define PWM_BRIDGE_A_DBFED 5
#define PWM_BRIDGE_A_TZA_ACTION EPWM_TZ_ACTION_LOW
#define PWM_BRIDGE_A_TZB_ACTION EPWM_TZ_ACTION_LOW
#define PWM_BRIDGE_A_INTERRUPT_SOURCE EPWM_INT_TBCTR_DISABLED
void PWM_BRIDGE_A_init();
#define PWM_BRIDGE_B_BASE EPWM2_BASE
#define PWM_BRIDGE_B_TBPRD 285
#define PWM_BRIDGE_B_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define PWM_BRIDGE_B_TBPHS 145
#define PWM_BRIDGE_B_CMPA 143
#define PWM_BRIDGE_B_CMPB 143
#define PWM_BRIDGE_B_CMPC 0
#define PWM_BRIDGE_B_CMPD 0
#define PWM_BRIDGE_B_DBRED 5
#define PWM_BRIDGE_B_DBFED 5
#define PWM_BRIDGE_B_TZA_ACTION EPWM_TZ_ACTION_LOW
#define PWM_BRIDGE_B_TZB_ACTION EPWM_TZ_ACTION_LOW
#define PWM_BRIDGE_B_INTERRUPT_SOURCE EPWM_INT_TBCTR_DISABLED
void PWM_BRIDGE_B_init();
#define PWM_ADC_TRIGGER_BASE EPWM3_BASE
#define PWM_ADC_TRIGGER_TBPRD 285
#define PWM_ADC_TRIGGER_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define PWM_ADC_TRIGGER_TBPHS 4
#define PWM_ADC_TRIGGER_CMPA 71
#define PWM_ADC_TRIGGER_CMPB 214
#define PWM_ADC_TRIGGER_CMPC 0
#define PWM_ADC_TRIGGER_CMPD 0
#define PWM_ADC_TRIGGER_DBRED 0
#define PWM_ADC_TRIGGER_DBFED 0
#define PWM_ADC_TRIGGER_TZA_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_ADC_TRIGGER_TZB_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_ADC_TRIGGER_INTERRUPT_SOURCE EPWM_INT_TBCTR_DISABLED
void PWM_ADC_TRIGGER_init();
#define PWM_BUZZER_BASE EPWM4_BASE
#define PWM_BUZZER_TBPRD 1561
#define PWM_BUZZER_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define PWM_BUZZER_TBPHS 0
#define PWM_BUZZER_CMPA 781
#define PWM_BUZZER_CMPB 0
#define PWM_BUZZER_CMPC 0
#define PWM_BUZZER_CMPD 0
#define PWM_BUZZER_DBRED 0
#define PWM_BUZZER_DBFED 0
#define PWM_BUZZER_TZA_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_BUZZER_TZB_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_BUZZER_INTERRUPT_SOURCE EPWM_INT_TBCTR_ZERO
void PWM_BUZZER_init();
#define PWM_FAN_BASE EPWM5_BASE
#define PWM_FAN_TBPRD 3999
#define PWM_FAN_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define PWM_FAN_TBPHS 0
#define PWM_FAN_CMPA 0
#define PWM_FAN_CMPB 0
#define PWM_FAN_CMPC 0
#define PWM_FAN_CMPD 0
#define PWM_FAN_DBRED 0
#define PWM_FAN_DBFED 0
#define PWM_FAN_TZA_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_FAN_TZB_ACTION EPWM_TZ_ACTION_HIGH_Z
#define PWM_FAN_INTERRUPT_SOURCE EPWM_INT_TBCTR_DISABLED
void PWM_FAN_init();

//*****************************************************************************
//
// EPWMXBAR Configurations
//
//*****************************************************************************
void EPWM_XBAR_PEAK_CURRENT_init();
#define EPWM_XBAR_PEAK_CURRENT XBAR_TRIP7
#define EPWM_XBAR_PEAK_CURRENT_ENABLED_MUXES (XBAR_MUX00 | XBAR_MUX02)
void EPWM_XBAR_HARD_FAULT_init();
#define EPWM_XBAR_HARD_FAULT XBAR_TRIP4
#define EPWM_XBAR_HARD_FAULT_ENABLED_MUXES (XBAR_MUX04 | XBAR_MUX06 | XBAR_MUX12)

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
#define LED_1 13
void LED_1_init();
#define LED_2 12
void LED_2_init();
#define LED_3 11
void LED_3_init();
#define LED_4 33
void LED_4_init();
#define FAN_TACH 9
void FAN_TACH_init();

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
#define FAN_TACH_INPUT_SOURCE 9
#define FAN_TACH_INPUT_INPUT XBAR_INPUT7
void FAN_TACH_INPUT_init();

//*****************************************************************************
//
// MEMCFG Configurations
//
//*****************************************************************************

//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
#define SCI_DEBUG_BASE SCIA_BASE
#define SCI_DEBUG_BAUDRATE 1562500
#define SCI_DEBUG_CONFIG_WLEN SCI_CONFIG_WLEN_8
#define SCI_DEBUG_CONFIG_STOP SCI_CONFIG_STOP_ONE
#define SCI_DEBUG_CONFIG_PAR SCI_CONFIG_PAR_NONE
void SCI_DEBUG_init();

//*****************************************************************************
//
// SYNC Scheme Configurations
//
//*****************************************************************************

//*****************************************************************************
//
// Board Configurations
//
//*****************************************************************************
void	Board_init();
void	ADC_init();
void	ASYSCTL_init();
void	CAN_init();
void	CLA_init();
void	CMPSS_init();
void	CPUTIMER_init();
void	DMA_init();
void	ECAP_init();
void	EPWM_init();
void	EPWMXBAR_init();
void	GPIO_init();
void	INPUTXBAR_init();
void	MEMCFG_init();
void	SCI_init();
void	SYNC_init();
void	PinMux_init();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // end of BOARD_H definition
