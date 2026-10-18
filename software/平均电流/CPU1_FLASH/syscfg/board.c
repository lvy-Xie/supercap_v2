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

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules.
// Call this function in your application if you wish to do all module
// initialization.
// If you wish to not use some of the initializations, instead of the
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	INPUTXBAR_init();
	SYNC_init();
	ASYSCTL_init();
	CLA_init();
	MEMCFG_init();
	ADC_init();
	CAN_init();
	CMPSS_init();
	CPUTIMER_init();
	DMA_init();
	ECAP_init();
	EPWM_init();
	EPWMXBAR_init();
	GPIO_init();
	SCI_init();

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//

	//
	// CANA -> CAN_CHASSIS Pinmux
	//
	GPIO_setPinConfig(CAN_CHASSIS_CANRX_PIN_CONFIG);
	GPIO_setPadConfig(CAN_CHASSIS_CANRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(CAN_CHASSIS_CANRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(CAN_CHASSIS_CANTX_PIN_CONFIG);
	GPIO_setPadConfig(CAN_CHASSIS_CANTX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(CAN_CHASSIS_CANTX_GPIO, GPIO_QUAL_ASYNC);

	//
	// EPWM1 -> PWM_BRIDGE_A Pinmux
	//
	GPIO_setPinConfig(PWM_BRIDGE_A_EPWMA_PIN_CONFIG);
	GPIO_setPadConfig(PWM_BRIDGE_A_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_BRIDGE_A_EPWMA_GPIO, GPIO_QUAL_SYNC);

	GPIO_setPinConfig(PWM_BRIDGE_A_EPWMB_PIN_CONFIG);
	GPIO_setPadConfig(PWM_BRIDGE_A_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_BRIDGE_A_EPWMB_GPIO, GPIO_QUAL_SYNC);

	//
	// EPWM2 -> PWM_BRIDGE_B Pinmux
	//
	GPIO_setPinConfig(PWM_BRIDGE_B_EPWMA_PIN_CONFIG);
	GPIO_setPadConfig(PWM_BRIDGE_B_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_BRIDGE_B_EPWMA_GPIO, GPIO_QUAL_SYNC);

	GPIO_setPinConfig(PWM_BRIDGE_B_EPWMB_PIN_CONFIG);
	GPIO_setPadConfig(PWM_BRIDGE_B_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_BRIDGE_B_EPWMB_GPIO, GPIO_QUAL_SYNC);

	//
	// EPWM3 -> PWM_ADC_TRIGGER Pinmux
	//
	//
	// EPWM4 -> PWM_BUZZER Pinmux
	//
	GPIO_setPinConfig(PWM_BUZZER_EPWMA_PIN_CONFIG);
	GPIO_setPadConfig(PWM_BUZZER_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_BUZZER_EPWMA_GPIO, GPIO_QUAL_SYNC);

	//
	// EPWM5 -> PWM_FAN Pinmux
	//
	GPIO_setPinConfig(PWM_FAN_EPWMA_PIN_CONFIG);
	GPIO_setPadConfig(PWM_FAN_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PWM_FAN_EPWMA_GPIO, GPIO_QUAL_SYNC);

	// GPIO13 -> LED_1 Pinmux
	GPIO_setPinConfig(GPIO_13_GPIO13);
	// GPIO12 -> LED_2 Pinmux
	GPIO_setPinConfig(GPIO_12_GPIO12);
	// GPIO11 -> LED_3 Pinmux
	GPIO_setPinConfig(GPIO_11_GPIO11);
	// GPIO33 -> LED_4 Pinmux
	GPIO_setPinConfig(GPIO_33_GPIO33);
	// GPIO9 -> FAN_TACH Pinmux
	GPIO_setPinConfig(GPIO_9_GPIO9);
	//
	// SCIA -> SCI_DEBUG Pinmux
	//
	GPIO_setPinConfig(SCI_DEBUG_SCIRX_PIN_CONFIG);
	GPIO_setPadConfig(SCI_DEBUG_SCIRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(SCI_DEBUG_SCIRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(SCI_DEBUG_SCITX_PIN_CONFIG);
	GPIO_setPadConfig(SCI_DEBUG_SCITX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(SCI_DEBUG_SCITX_GPIO, GPIO_QUAL_ASYNC);


}

//*****************************************************************************
//
// ADC Configurations
//
//*****************************************************************************
void ADC_init(){
	ADC_A_init();
	ADC_B_init();
	ADC_C_init();
}

void ADC_A_init(){
	//
	// ADC Initialization: Write ADC configurations and power up the ADC
	//
	// Set the analog voltage reference selection and ADC module's offset trims.
	// This function sets the analog voltage reference to internal (with the reference voltage of 1.65V or 2.5V) or external for ADC
	// which is same as ASysCtl APIs.
	//
	ADC_setVREF(ADC_A_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_A_BASE, ADC_CLK_DIV_2_0);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_A_BASE, ADC_PULSE_END_OF_CONV);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_A_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(5000);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_A_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_A_BASE, ADC_PRI_ALL_ROUND_ROBIN);
	//
	// Start of Conversion 0 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 0
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCA
	//	  	Channel			: ADC_CH_ADCIN2
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_A_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN2, 20U);
	ADC_setInterruptSOCTrigger(ADC_A_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 1 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 1
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCB
	//	  	Channel			: ADC_CH_ADCIN6
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_A_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM3_SOCB, ADC_CH_ADCIN6, 20U);
	ADC_setInterruptSOCTrigger(ADC_A_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 2 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 2
	//	  	Trigger			: ADC_TRIGGER_SW_ONLY
	//	  	Channel			: ADC_CH_ADCIN1
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_A_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN1, 64U);
	ADC_setInterruptSOCTrigger(ADC_A_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
}

void ADC_B_init(){
	//
	// ADC Initialization: Write ADC configurations and power up the ADC
	//
	// Set the analog voltage reference selection and ADC module's offset trims.
	// This function sets the analog voltage reference to internal (with the reference voltage of 1.65V or 2.5V) or external for ADC
	// which is same as ASysCtl APIs.
	//
	ADC_setVREF(ADC_B_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_B_BASE, ADC_CLK_DIV_2_0);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_B_BASE, ADC_PULSE_END_OF_CONV);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_B_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(5000);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_B_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_B_BASE, ADC_PRI_ALL_ROUND_ROBIN);
	//
	// Start of Conversion 0 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 0
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCA
	//	  	Channel			: ADC_CH_ADCIN2
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_B_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN2, 20U);
	ADC_setInterruptSOCTrigger(ADC_B_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 1 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 1
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCB
	//	  	Channel			: ADC_CH_ADCIN4
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_B_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM3_SOCB, ADC_CH_ADCIN4, 20U);
	ADC_setInterruptSOCTrigger(ADC_B_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
}

void ADC_C_init(){
	//
	// ADC Initialization: Write ADC configurations and power up the ADC
	//
	// Set the analog voltage reference selection and ADC module's offset trims.
	// This function sets the analog voltage reference to internal (with the reference voltage of 1.65V or 2.5V) or external for ADC
	// which is same as ASysCtl APIs.
	//
	ADC_setVREF(ADC_C_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_C_BASE, ADC_CLK_DIV_2_0);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_C_BASE, ADC_PULSE_END_OF_CONV);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_C_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(5000);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_C_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_C_BASE, ADC_PRI_ALL_ROUND_ROBIN);
	//
	// Start of Conversion 0 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 0
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCA
	//	  	Channel			: ADC_CH_ADCIN1
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN1, 20U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 1 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 1
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCB
	//	  	Channel			: ADC_CH_ADCIN10
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM3_SOCB, ADC_CH_ADCIN10, 20U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 2 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 2
	//	  	Trigger			: ADC_TRIGGER_SW_ONLY
	//	  	Channel			: ADC_CH_ADCIN2
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN2, 64U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 3 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 3
	//	  	Trigger			: ADC_TRIGGER_EPWM3_SOCB
	//	  	Channel			: ADC_CH_ADCIN3
	//	 	Sample Window	: 20 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM3_SOCB, ADC_CH_ADCIN3, 20U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER3, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 4 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 4
	//	  	Trigger			: ADC_TRIGGER_SW_ONLY
	//	  	Channel			: ADC_CH_ADCIN4
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN4, 64U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER4, ADC_INT_SOC_TRIGGER_NONE);
	//
	// ADC Interrupt 1 Configuration
	// 		Source	: ADC_SOC_NUMBER3
	// 		Interrupt Source: enabled
	// 		Continuous Mode	: disabled
	//
	//
	ADC_setInterruptSource(ADC_C_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER3);
	ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
	ADC_disableContinuousMode(ADC_C_BASE, ADC_INT_NUMBER1);
	ADC_enableInterrupt(ADC_C_BASE, ADC_INT_NUMBER1);
}


//*****************************************************************************
//
// ASYSCTL Configurations
//
//*****************************************************************************
void ASYSCTL_init(){
	//
	// asysctl initialization
	//
	// Disables the temperature sensor output to the ADC.
	//
	ASysCtl_disableTemperatureSensor();
	//
	// Set the analog voltage reference selection to internal.
	//
	ASysCtl_setAnalogReferenceInternal( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
	//
	// Set the internal analog voltage reference selection to 1.65V.
	//
	ASysCtl_setAnalogReference1P65( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
}

//*****************************************************************************
//
// CAN Configurations
//
//*****************************************************************************
void CAN_init(){
	CAN_CHASSIS_init();
}

void CAN_CHASSIS_init(){
	CAN_initModule(CAN_CHASSIS_BASE);
	//
	// Refer to the Driver Library User Guide for information on how to set
	// tighter timing control. Additionally, consult the device data sheet
	// for more information about the CAN module clocking.
	//
	CAN_setBitTiming(CAN_CHASSIS_BASE, 3, 0, 15, 7, 3);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 1
	//      Message Identifier: 800
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_RX
	//      Message ID Mask: 0
	//      Message Object Flags: CAN_MSG_OBJ_USE_ID_FILTER
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(CAN_CHASSIS_BASE, 1, CAN_CHASSIS_MessageObj1_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_RX, 0, CAN_MSG_OBJ_USE_ID_FILTER,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 2
	//      Message Identifier: 801
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(CAN_CHASSIS_BASE, 2, CAN_CHASSIS_MessageObj2_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 3
	//      Message Identifier: 802
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(CAN_CHASSIS_BASE, 3, CAN_CHASSIS_MessageObj3_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 4
	//      Message Identifier: 803
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(CAN_CHASSIS_BASE, 4, CAN_CHASSIS_MessageObj4_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Start CAN module operations
	//
	CAN_startModule(CAN_CHASSIS_BASE);
}

//*****************************************************************************
//
// CLA Configurations
//
//*****************************************************************************

void CLA_ADC_init(){
	//
    // Configure all CLA task vectors
    // On Type-1 and Type-2 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
#pragma diag_suppress=770
    //
    // CLA Task 1
    //
    CLA_mapTaskVector(CLA_ADC_BASE, CLA_MVECT_1, (uint16_t)&Cla1Task1);
    CLA_setTriggerSource(CLA_TASK_1, CLA_TRIGGER_SOFTWARE);
    //
    // CLA Task 8
    //
    CLA_mapTaskVector(CLA_ADC_BASE, CLA_MVECT_8, (uint16_t)&Cla1Task8);
    CLA_setTriggerSource(CLA_TASK_8, CLA_TRIGGER_SOFTWARE);
    //
    // Disable background task
    //
    CLA_disableBackgroundTask(CLA_ADC_BASE);
#pragma diag_warning=770
	//
    // Enable the IACK instruction to start a task on CLA in software
    // for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
    // subset of tasks) by writing to their respective bits in the
    // MIER register
    //
	CLA_enableIACK(CLA_ADC_BASE);
    CLA_enableTasks(CLA_ADC_BASE, CLA_TASKFLAG_1 | CLA_TASKFLAG_8 );
}


void CLA_init()
{
#ifdef _FLASH
#ifndef CMDTOOL // Linker command tool is not used

    extern uint32_t Cla1ProgRunStart, Cla1ProgLoadStart, Cla1ProgLoadSize;
    extern uint32_t Cla1ConstRunStart, Cla1ConstLoadStart, Cla1ConstLoadSize;

    //
    // Copy the program and constants from FLASH to RAM before configuring
    // the CLA
    //

    memcpy((uint32_t *)((uint32_t)&Cla1ProgRunStart ) , (uint32_t *)&Cla1ProgLoadStart,
           (uint32_t)&Cla1ProgLoadSize);
    memcpy((uint32_t *)((uint32_t)&Cla1ConstRunStart ), (uint32_t *)&Cla1ConstLoadStart,
        (uint32_t)&Cla1ConstLoadSize );

#endif //CMDTOOL
#endif //_FLASH

	CLA_ADC_init();
}

//*****************************************************************************
//
// CMPSS Configurations
//
//*****************************************************************************
void CMPSS_init(){
	CMPSS_I_BRIDGE_A_init();
	CMPSS_I_BRIDGE_B_init();
	CMPSS_V_BAT_init();
	CMPSS_V_CAP_init();
	CMPSS_I_CAP_init();
}

void CMPSS_I_BRIDGE_A_init(){
    //
    // Select the value for CMP1HPMXSEL.
    //
    ASysCtl_selectCMPHPMux(CMPSS_I_BRIDGE_A_CMPHPMUX_SELECT,1U);
    //
    // Select the value for CMP1LPMXSEL.
    //
    ASysCtl_selectCMPLPMux(CMPSS_I_BRIDGE_A_CMPLPMUX_SELECT,1U);
    //
    // Sets the configuration for the high comparator.
    //
    CMPSS_configHighComparator(CMPSS_I_BRIDGE_A_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the low comparator.
    //
    CMPSS_configLowComparator(CMPSS_I_BRIDGE_A_BASE,(CMPSS_INSRC_DAC | CMPSS_INV_INVERTED));
    //
    // Sets the configuration for the internal comparator DACs.
    //
    CMPSS_configDAC(CMPSS_I_BRIDGE_A_BASE,(CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW));
    //
    // Sets the value of the internal DAC of the high comparator.
    //
    CMPSS_setDACValueHigh(CMPSS_I_BRIDGE_A_BASE,2693U);
    //
    // Sets the value of the internal DAC of the low comparator.
    //
    CMPSS_setDACValueLow(CMPSS_I_BRIDGE_A_BASE,1402U);
    //
    //  Configures the digital filter of the high comparator.
    //
    CMPSS_configFilterHigh(CMPSS_I_BRIDGE_A_BASE, 3U, 5U, 3U);
    //
    // Configures the digital filter of the low comparator.
    //
    CMPSS_configFilterLow(CMPSS_I_BRIDGE_A_BASE, 3U, 5U, 3U);
    //
    // Initializes the digital filter of the high comparator.
    //
    CMPSS_initFilterHigh(CMPSS_I_BRIDGE_A_BASE);
    //
    // Initializes the digital filter of the low comparator.
    //
    CMPSS_initFilterLow(CMPSS_I_BRIDGE_A_BASE);
    //
    // Sets the output signal configuration for the high comparator.
    //
    CMPSS_configOutputsHigh(CMPSS_I_BRIDGE_A_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the output signal configuration for the low comparator.
    //
    CMPSS_configOutputsLow(CMPSS_I_BRIDGE_A_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the comparator hysteresis settings.
    //
    CMPSS_setHysteresis(CMPSS_I_BRIDGE_A_BASE,2U);
    //
    // Configures the comparator subsystem's ramp generator.
    //
    CMPSS_configRamp(CMPSS_I_BRIDGE_A_BASE,0U,0U,0U,1U,true);
    //
    // Disables reset of HIGH comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCHigh(CMPSS_I_BRIDGE_A_BASE);
    //
    // Disables reset of LOW comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCLow(CMPSS_I_BRIDGE_A_BASE);
    //
    // Sets the ePWM module blanking signal that holds trip in reset.
    //
    CMPSS_configBlanking(CMPSS_I_BRIDGE_A_BASE,1U);
    //
    // Disables an ePWM blanking signal from holding trip in reset.
    //
    CMPSS_disableBlanking(CMPSS_I_BRIDGE_A_BASE);
    //
    // Configures whether or not the digital filter latches are reset by PWMSYNC
    //
    CMPSS_configLatchOnPWMSYNC(CMPSS_I_BRIDGE_A_BASE,false,false);
    //
    // Enables the CMPSS module.
    //
    CMPSS_enableModule(CMPSS_I_BRIDGE_A_BASE);
    //
    // Delay for CMPSS DAC to power up.
    //
    DEVICE_DELAY_US(500);
}
void CMPSS_I_BRIDGE_B_init(){
    //
    // Select the value for CMP2HPMXSEL.
    //
    ASysCtl_selectCMPHPMux(CMPSS_I_BRIDGE_B_CMPHPMUX_SELECT,0U);
    //
    // Select the value for CMP2LPMXSEL.
    //
    ASysCtl_selectCMPLPMux(CMPSS_I_BRIDGE_B_CMPLPMUX_SELECT,0U);
    //
    // Sets the configuration for the high comparator.
    //
    CMPSS_configHighComparator(CMPSS_I_BRIDGE_B_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the low comparator.
    //
    CMPSS_configLowComparator(CMPSS_I_BRIDGE_B_BASE,(CMPSS_INSRC_DAC | CMPSS_INV_INVERTED));
    //
    // Sets the configuration for the internal comparator DACs.
    //
    CMPSS_configDAC(CMPSS_I_BRIDGE_B_BASE,(CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW));
    //
    // Sets the value of the internal DAC of the high comparator.
    //
    CMPSS_setDACValueHigh(CMPSS_I_BRIDGE_B_BASE,2693U);
    //
    // Sets the value of the internal DAC of the low comparator.
    //
    CMPSS_setDACValueLow(CMPSS_I_BRIDGE_B_BASE,1402U);
    //
    //  Configures the digital filter of the high comparator.
    //
    CMPSS_configFilterHigh(CMPSS_I_BRIDGE_B_BASE, 3U, 5U, 3U);
    //
    // Configures the digital filter of the low comparator.
    //
    CMPSS_configFilterLow(CMPSS_I_BRIDGE_B_BASE, 3U, 5U, 3U);
    //
    // Initializes the digital filter of the high comparator.
    //
    CMPSS_initFilterHigh(CMPSS_I_BRIDGE_B_BASE);
    //
    // Initializes the digital filter of the low comparator.
    //
    CMPSS_initFilterLow(CMPSS_I_BRIDGE_B_BASE);
    //
    // Sets the output signal configuration for the high comparator.
    //
    CMPSS_configOutputsHigh(CMPSS_I_BRIDGE_B_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the output signal configuration for the low comparator.
    //
    CMPSS_configOutputsLow(CMPSS_I_BRIDGE_B_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the comparator hysteresis settings.
    //
    CMPSS_setHysteresis(CMPSS_I_BRIDGE_B_BASE,2U);
    //
    // Configures the comparator subsystem's ramp generator.
    //
    CMPSS_configRamp(CMPSS_I_BRIDGE_B_BASE,0U,0U,0U,1U,true);
    //
    // Disables reset of HIGH comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCHigh(CMPSS_I_BRIDGE_B_BASE);
    //
    // Disables reset of LOW comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCLow(CMPSS_I_BRIDGE_B_BASE);
    //
    // Sets the ePWM module blanking signal that holds trip in reset.
    //
    CMPSS_configBlanking(CMPSS_I_BRIDGE_B_BASE,1U);
    //
    // Disables an ePWM blanking signal from holding trip in reset.
    //
    CMPSS_disableBlanking(CMPSS_I_BRIDGE_B_BASE);
    //
    // Configures whether or not the digital filter latches are reset by PWMSYNC
    //
    CMPSS_configLatchOnPWMSYNC(CMPSS_I_BRIDGE_B_BASE,false,false);
    //
    // Enables the CMPSS module.
    //
    CMPSS_enableModule(CMPSS_I_BRIDGE_B_BASE);
    //
    // Delay for CMPSS DAC to power up.
    //
    DEVICE_DELAY_US(500);
}
void CMPSS_V_BAT_init(){
    //
    // Select the value for CMP3HPMXSEL.
    //
    ASysCtl_selectCMPHPMux(CMPSS_V_BAT_CMPHPMUX_SELECT,0U);
    //
    // Select the value for CMP3LPMXSEL.
    //
    ASysCtl_selectCMPLPMux(CMPSS_V_BAT_CMPLPMUX_SELECT,0U);
    //
    // Sets the configuration for the high comparator.
    //
    CMPSS_configHighComparator(CMPSS_V_BAT_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the low comparator.
    //
    CMPSS_configLowComparator(CMPSS_V_BAT_BASE,(CMPSS_INSRC_DAC | CMPSS_INV_INVERTED));
    //
    // Sets the configuration for the internal comparator DACs.
    //
    CMPSS_configDAC(CMPSS_V_BAT_BASE,(CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW));
    //
    // Sets the value of the internal DAC of the high comparator.
    //
    CMPSS_setDACValueHigh(CMPSS_V_BAT_BASE,3847U);
    //
    // Sets the value of the internal DAC of the low comparator.
    //
    CMPSS_setDACValueLow(CMPSS_V_BAT_BASE,2606U);
    //
    //  Configures the digital filter of the high comparator.
    //
    CMPSS_configFilterHigh(CMPSS_V_BAT_BASE, 3U, 5U, 3U);
    //
    // Configures the digital filter of the low comparator.
    //
    CMPSS_configFilterLow(CMPSS_V_BAT_BASE, 3U, 5U, 3U);
    //
    // Initializes the digital filter of the high comparator.
    //
    CMPSS_initFilterHigh(CMPSS_V_BAT_BASE);
    //
    // Initializes the digital filter of the low comparator.
    //
    CMPSS_initFilterLow(CMPSS_V_BAT_BASE);
    //
    // Sets the output signal configuration for the high comparator.
    //
    CMPSS_configOutputsHigh(CMPSS_V_BAT_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the output signal configuration for the low comparator.
    //
    CMPSS_configOutputsLow(CMPSS_V_BAT_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_FILTER));
    //
    // Sets the comparator hysteresis settings.
    //
    CMPSS_setHysteresis(CMPSS_V_BAT_BASE,2U);
    //
    // Configures the comparator subsystem's ramp generator.
    //
    CMPSS_configRamp(CMPSS_V_BAT_BASE,0U,0U,0U,1U,true);
    //
    // Disables reset of HIGH comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCHigh(CMPSS_V_BAT_BASE);
    //
    // Disables reset of LOW comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCLow(CMPSS_V_BAT_BASE);
    //
    // Sets the ePWM module blanking signal that holds trip in reset.
    //
    CMPSS_configBlanking(CMPSS_V_BAT_BASE,1U);
    //
    // Disables an ePWM blanking signal from holding trip in reset.
    //
    CMPSS_disableBlanking(CMPSS_V_BAT_BASE);
    //
    // Configures whether or not the digital filter latches are reset by PWMSYNC
    //
    CMPSS_configLatchOnPWMSYNC(CMPSS_V_BAT_BASE,false,false);
    //
    // Enables the CMPSS module.
    //
    CMPSS_enableModule(CMPSS_V_BAT_BASE);
    //
    // Delay for CMPSS DAC to power up.
    //
    DEVICE_DELAY_US(500);
}
void CMPSS_V_CAP_init(){
    //
    // Select the value for CMP4HPMXSEL.
    //
    ASysCtl_selectCMPHPMux(CMPSS_V_CAP_CMPHPMUX_SELECT,0U);
    //
    // Select the value for CMP4LPMXSEL.
    //
    ASysCtl_selectCMPLPMux(CMPSS_V_CAP_CMPLPMUX_SELECT,0U);
    //
    // Sets the configuration for the high comparator.
    //
    CMPSS_configHighComparator(CMPSS_V_CAP_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the low comparator.
    //
    CMPSS_configLowComparator(CMPSS_V_CAP_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the internal comparator DACs.
    //
    CMPSS_configDAC(CMPSS_V_CAP_BASE,(CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW));
    //
    // Sets the value of the internal DAC of the high comparator.
    //
    CMPSS_setDACValueHigh(CMPSS_V_CAP_BASE,0U);
    //
    // Sets the value of the internal DAC of the low comparator.
    //
    CMPSS_setDACValueLow(CMPSS_V_CAP_BASE,0U);
    //
    //  Configures the digital filter of the high comparator.
    //
    CMPSS_configFilterHigh(CMPSS_V_CAP_BASE, 0U, 1U, 1U);
    //
    // Configures the digital filter of the low comparator.
    //
    CMPSS_configFilterLow(CMPSS_V_CAP_BASE, 0U, 1U, 1U);
    //
    // Sets the output signal configuration for the high comparator.
    //
    CMPSS_configOutputsHigh(CMPSS_V_CAP_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP));
    //
    // Sets the output signal configuration for the low comparator.
    //
    CMPSS_configOutputsLow(CMPSS_V_CAP_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP));
    //
    // Sets the comparator hysteresis settings.
    //
    CMPSS_setHysteresis(CMPSS_V_CAP_BASE,2U);
    //
    // Configures the comparator subsystem's ramp generator.
    //
    CMPSS_configRamp(CMPSS_V_CAP_BASE,0U,0U,0U,1U,true);
    //
    // Disables reset of HIGH comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCHigh(CMPSS_V_CAP_BASE);
    //
    // Disables reset of LOW comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCLow(CMPSS_V_CAP_BASE);
    //
    // Sets the ePWM module blanking signal that holds trip in reset.
    //
    CMPSS_configBlanking(CMPSS_V_CAP_BASE,1U);
    //
    // Disables an ePWM blanking signal from holding trip in reset.
    //
    CMPSS_disableBlanking(CMPSS_V_CAP_BASE);
    //
    // Configures whether or not the digital filter latches are reset by PWMSYNC
    //
    CMPSS_configLatchOnPWMSYNC(CMPSS_V_CAP_BASE,false,false);
    //
    // Enables the CMPSS module.
    //
    CMPSS_enableModule(CMPSS_V_CAP_BASE);
    //
    // Delay for CMPSS DAC to power up.
    //
    DEVICE_DELAY_US(500);
}
void CMPSS_I_CAP_init(){
    //
    // Select the value for CMP7HPMXSEL.
    //
    ASysCtl_selectCMPHPMux(CMPSS_I_CAP_CMPHPMUX_SELECT,0U);
    //
    // Select the value for CMP7LPMXSEL.
    //
    ASysCtl_selectCMPLPMux(CMPSS_I_CAP_CMPLPMUX_SELECT,0U);
    //
    // Sets the configuration for the high comparator.
    //
    CMPSS_configHighComparator(CMPSS_I_CAP_BASE,(CMPSS_INSRC_DAC));
    //
    // Sets the configuration for the low comparator.
    //
    CMPSS_configLowComparator(CMPSS_I_CAP_BASE,(CMPSS_INSRC_DAC | CMPSS_INV_INVERTED));
    //
    // Sets the configuration for the internal comparator DACs.
    //
    CMPSS_configDAC(CMPSS_I_CAP_BASE,(CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW));
    //
    // Sets the value of the internal DAC of the high comparator.
    //
    CMPSS_setDACValueHigh(CMPSS_I_CAP_BASE,0U);
    //
    // Sets the value of the internal DAC of the low comparator.
    //
    CMPSS_setDACValueLow(CMPSS_I_CAP_BASE,4095U);
    //
    //  Configures the digital filter of the high comparator.
    //
    CMPSS_configFilterHigh(CMPSS_I_CAP_BASE, 0U, 1U, 1U);
    //
    // Configures the digital filter of the low comparator.
    //
    CMPSS_configFilterLow(CMPSS_I_CAP_BASE, 0U, 1U, 1U);
    //
    // Sets the output signal configuration for the high comparator.
    //
    CMPSS_configOutputsHigh(CMPSS_I_CAP_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP));
    //
    // Sets the output signal configuration for the low comparator.
    //
    CMPSS_configOutputsLow(CMPSS_I_CAP_BASE,(CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP));
    //
    // Sets the comparator hysteresis settings.
    //
    CMPSS_setHysteresis(CMPSS_I_CAP_BASE,2U);
    //
    // Configures the comparator subsystem's ramp generator.
    //
    CMPSS_configRamp(CMPSS_I_CAP_BASE,0U,0U,0U,1U,true);
    //
    // Disables reset of HIGH comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCHigh(CMPSS_I_CAP_BASE);
    //
    // Disables reset of LOW comparator digital filter output latch on PWMSYNC
    //
    CMPSS_disableLatchResetOnPWMSYNCLow(CMPSS_I_CAP_BASE);
    //
    // Sets the ePWM module blanking signal that holds trip in reset.
    //
    CMPSS_configBlanking(CMPSS_I_CAP_BASE,1U);
    //
    // Disables an ePWM blanking signal from holding trip in reset.
    //
    CMPSS_disableBlanking(CMPSS_I_CAP_BASE);
    //
    // Configures whether or not the digital filter latches are reset by PWMSYNC
    //
    CMPSS_configLatchOnPWMSYNC(CMPSS_I_CAP_BASE,false,false);
    //
    // Enables the CMPSS module.
    //
    CMPSS_enableModule(CMPSS_I_CAP_BASE);
    //
    // Delay for CMPSS DAC to power up.
    //
    DEVICE_DELAY_US(500);
}

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
void CPUTIMER_init(){
	TIMER_BACKGROUND_1KHZ_init();
}

void TIMER_BACKGROUND_1KHZ_init(){
	CPUTimer_setEmulationMode(TIMER_BACKGROUND_1KHZ_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
	CPUTimer_setPreScaler(TIMER_BACKGROUND_1KHZ_BASE, 0U);
	CPUTimer_setPeriod(TIMER_BACKGROUND_1KHZ_BASE, 99999U);
	CPUTimer_disableInterrupt(TIMER_BACKGROUND_1KHZ_BASE);
	CPUTimer_stopTimer(TIMER_BACKGROUND_1KHZ_BASE);

	CPUTimer_reloadTimerCounter(TIMER_BACKGROUND_1KHZ_BASE);
	CPUTimer_startTimer(TIMER_BACKGROUND_1KHZ_BASE);
}

//*****************************************************************************
//
// DMA Configurations
//
//*****************************************************************************
void DMA_init(){
    DMA_initController();
	DMA_ADC_CAPTURE_init();
}

void DMA_ADC_CAPTURE_init(){
    DMA_setEmulationMode(DMA_EMULATION_STOP);
    DMA_configAddresses(DMA_ADC_CAPTURE_BASE, AdcDma_Destination, (const void *)2816);
    DMA_configBurst(DMA_ADC_CAPTURE_BASE, 4U, 1, 1);
    DMA_configTransfer(DMA_ADC_CAPTURE_BASE, 3U, 29, 1);
    DMA_configWrap(DMA_ADC_CAPTURE_BASE, 65535U, 0, 65535U, 0);
    DMA_configMode(DMA_ADC_CAPTURE_BASE, DMA_TRIGGER_ADCC1, DMA_CFG_ONESHOT_ENABLE | DMA_CFG_CONTINUOUS_DISABLE | DMA_CFG_SIZE_16BIT);
    DMA_setInterruptMode(DMA_ADC_CAPTURE_BASE, DMA_INT_AT_END);
    DMA_enableInterrupt(DMA_ADC_CAPTURE_BASE);
    DMA_enableOverrunInterrupt(DMA_ADC_CAPTURE_BASE);
    DMA_enableTrigger(DMA_ADC_CAPTURE_BASE);
    DMA_stopChannel(DMA_ADC_CAPTURE_BASE);
}

//*****************************************************************************
//
// ECAP Configurations
//
//*****************************************************************************
void ECAP_init(){
	FAN_TACH_CAPTURE_init();
}

void FAN_TACH_CAPTURE_init(){
	//
	// Disables time stamp capture.
	//
	ECAP_disableTimeStampCapture(FAN_TACH_CAPTURE_BASE);
	//
	// Stops Time stamp counter.
	//
	ECAP_stopCounter(FAN_TACH_CAPTURE_BASE);
	//
	// Sets eCAP in Capture mode.
	//
	ECAP_enableCaptureMode(FAN_TACH_CAPTURE_BASE);
	//
	// Sets the capture mode.
	//
	ECAP_setCaptureMode(FAN_TACH_CAPTURE_BASE,ECAP_CONTINUOUS_CAPTURE_MODE,ECAP_EVENT_4);
	//
	// Sets the Capture event prescaler.
	//
	ECAP_setEventPrescaler(FAN_TACH_CAPTURE_BASE, 0U);
	//
	// Sets the Capture event polarity.
	//
	ECAP_setEventPolarity(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_1,ECAP_EVNT_RISING_EDGE);
	ECAP_setEventPolarity(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_2,ECAP_EVNT_RISING_EDGE);
	ECAP_setEventPolarity(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_3,ECAP_EVNT_RISING_EDGE);
	ECAP_setEventPolarity(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_4,ECAP_EVNT_RISING_EDGE);
	//
	// Configure counter reset on events
	//
	ECAP_enableCounterResetOnEvent(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_1);
	ECAP_enableCounterResetOnEvent(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_2);
	ECAP_enableCounterResetOnEvent(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_3);
	ECAP_enableCounterResetOnEvent(FAN_TACH_CAPTURE_BASE,ECAP_EVENT_4);
	//
	// Select eCAP input.
	//
	ECAP_selectECAPInput(FAN_TACH_CAPTURE_BASE,ECAP_INPUT_INPUTXBAR7);
	//
	// Sets a phase shift value count.
	//
	ECAP_setPhaseShiftCount(FAN_TACH_CAPTURE_BASE,0U);
	//
	// Disable counter loading with phase shift value.
	//
	ECAP_disableLoadCounter(FAN_TACH_CAPTURE_BASE);
	//
	// Configures Sync out signal mode.
	//
	ECAP_setSyncOutMode(FAN_TACH_CAPTURE_BASE,ECAP_SYNC_OUT_DISABLED);
	//
	// Resets eCAP counters and flags.
	//
	ECAP_resetCounters(FAN_TACH_CAPTURE_BASE);
	//
	// Configures emulation mode.
	//
	ECAP_setEmulationMode(FAN_TACH_CAPTURE_BASE,ECAP_EMULATION_STOP);
	//
	// Starts Time stamp counter for FAN_TACH_CAPTURE.
	//
	ECAP_startCounter(FAN_TACH_CAPTURE_BASE);
	//
	// Enables time stamp capture for FAN_TACH_CAPTURE.
	//
	ECAP_enableTimeStampCapture(FAN_TACH_CAPTURE_BASE);
	//
	// Re-arms the eCAP module for FAN_TACH_CAPTURE.
	//
	ECAP_reArm(FAN_TACH_CAPTURE_BASE);

}

//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************
void EPWM_init(){
	PWM_BRIDGE_A_init();
	PWM_BRIDGE_B_init();
	PWM_ADC_TRIGGER_init();
	PWM_BUZZER_init();
	PWM_FAN_init();
}
void PWM_BRIDGE_A_init(){
    HRPWM_setClockPrescaler(PWM_BRIDGE_A_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);	
    HRPWM_setPeriodLoadMode(PWM_BRIDGE_A_BASE, EPWM_PERIOD_DIRECT_LOAD);	
    EPWM_setTimeBasePeriod(PWM_BRIDGE_A_BASE, 285);	
    HRPWM_setTimeBaseCounter(PWM_BRIDGE_A_BASE, 0);	
    HRPWM_setTimeBaseCounterMode(PWM_BRIDGE_A_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_disablePhaseShiftLoad(PWM_BRIDGE_A_BASE);
    EPWM_setPhaseShift(PWM_BRIDGE_A_BASE, 0);	
    HRPWM_setSyncOutPulseMode(PWM_BRIDGE_A_BASE, EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);	
    EPWM_setCounterCompareValue(PWM_BRIDGE_A_BASE, EPWM_COUNTER_COMPARE_A, 143);	
    HRPWM_setCounterCompareShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(PWM_BRIDGE_A_BASE, EPWM_COUNTER_COMPARE_B, 143);	
    HRPWM_setCounterCompareShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableActionQualifierShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_ACTION_QUALIFIER_A);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    HRPWM_disableActionQualifierShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_ACTION_QUALIFIER_B);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_A_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    HRPWM_setDeadBandDelayPolarity(PWM_BRIDGE_A_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);	
    HRPWM_setDeadBandDelayMode(PWM_BRIDGE_A_BASE, EPWM_DB_RED, true);	
    HRPWM_setRisingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableRisingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_A_BASE);	
    HRPWM_setRisingEdgeDelayCount(PWM_BRIDGE_A_BASE, 5);	
    HRPWM_setDeadBandDelayMode(PWM_BRIDGE_A_BASE, EPWM_DB_FED, true);	
    HRPWM_setFallingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableFallingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_A_BASE);	
    HRPWM_setFallingEdgeDelayCount(PWM_BRIDGE_A_BASE, 5);	
    HRPWM_setDeadBandControlShadowLoadMode(PWM_BRIDGE_A_BASE, EPWM_DB_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableDeadBandControlShadowLoadMode(PWM_BRIDGE_A_BASE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_DCAEVT1, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_DCAEVT2, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_DCBEVT1, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_A_BASE, EPWM_TZ_ACTION_EVENT_DCBEVT2, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_enableAutoConversion(PWM_BRIDGE_A_BASE);	
    HRPWM_setMEPEdgeSelect(PWM_BRIDGE_A_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_RISING_EDGE);	
    HRPWM_setMEPEdgeSelect(PWM_BRIDGE_A_BASE, HRPWM_CHANNEL_B, HRPWM_MEP_CTRL_FALLING_EDGE);	
}
void PWM_BRIDGE_B_init(){
    HRPWM_setClockPrescaler(PWM_BRIDGE_B_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);	
    HRPWM_setPeriodLoadMode(PWM_BRIDGE_B_BASE, EPWM_PERIOD_DIRECT_LOAD);	
    EPWM_setTimeBasePeriod(PWM_BRIDGE_B_BASE, 285);	
    HRPWM_setTimeBaseCounter(PWM_BRIDGE_B_BASE, 143);	
    HRPWM_setTimeBaseCounterMode(PWM_BRIDGE_B_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_enablePhaseShiftLoad(PWM_BRIDGE_B_BASE);
    EPWM_setPhaseShift(PWM_BRIDGE_B_BASE, 145);	
    HRPWM_setSyncOutPulseMode(PWM_BRIDGE_B_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);	
    EPWM_setCounterCompareValue(PWM_BRIDGE_B_BASE, EPWM_COUNTER_COMPARE_A, 143);	
    HRPWM_setCounterCompareShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(PWM_BRIDGE_B_BASE, EPWM_COUNTER_COMPARE_B, 143);	
    HRPWM_setCounterCompareShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableActionQualifierShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_ACTION_QUALIFIER_A);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    HRPWM_disableActionQualifierShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_ACTION_QUALIFIER_B);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    HRPWM_setActionQualifierAction(PWM_BRIDGE_B_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    HRPWM_setDeadBandDelayPolarity(PWM_BRIDGE_B_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);	
    HRPWM_setDeadBandDelayMode(PWM_BRIDGE_B_BASE, EPWM_DB_RED, true);	
    HRPWM_setRisingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableRisingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_B_BASE);	
    HRPWM_setRisingEdgeDelayCount(PWM_BRIDGE_B_BASE, 5);	
    HRPWM_setDeadBandDelayMode(PWM_BRIDGE_B_BASE, EPWM_DB_FED, true);	
    HRPWM_setFallingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableFallingEdgeDelayCountShadowLoadMode(PWM_BRIDGE_B_BASE);	
    HRPWM_setFallingEdgeDelayCount(PWM_BRIDGE_B_BASE, 5);	
    HRPWM_setDeadBandControlShadowLoadMode(PWM_BRIDGE_B_BASE, EPWM_DB_LOAD_ON_CNTR_ZERO);	
    HRPWM_disableDeadBandControlShadowLoadMode(PWM_BRIDGE_B_BASE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_DCAEVT1, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_DCAEVT2, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_DCBEVT1, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_setTripZoneAction(PWM_BRIDGE_B_BASE, EPWM_TZ_ACTION_EVENT_DCBEVT2, EPWM_TZ_ACTION_DISABLE);	
    HRPWM_enableAutoConversion(PWM_BRIDGE_B_BASE);	
    HRPWM_setMEPEdgeSelect(PWM_BRIDGE_B_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_RISING_EDGE);	
    HRPWM_setMEPEdgeSelect(PWM_BRIDGE_B_BASE, HRPWM_CHANNEL_B, HRPWM_MEP_CTRL_FALLING_EDGE);	
}
void PWM_ADC_TRIGGER_init(){
    EPWM_setClockPrescaler(PWM_ADC_TRIGGER_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);	
    EPWM_setPeriodLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_PERIOD_DIRECT_LOAD);	
    EPWM_setTimeBasePeriod(PWM_ADC_TRIGGER_BASE, 285);	
    EPWM_setTimeBaseCounter(PWM_ADC_TRIGGER_BASE, 0);	
    EPWM_setTimeBaseCounterMode(PWM_ADC_TRIGGER_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_enablePhaseShiftLoad(PWM_ADC_TRIGGER_BASE);	
    EPWM_setPhaseShift(PWM_ADC_TRIGGER_BASE, 4);	
    EPWM_setCounterCompareValue(PWM_ADC_TRIGGER_BASE, EPWM_COUNTER_COMPARE_A, 71);	
    EPWM_setCounterCompareShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(PWM_ADC_TRIGGER_BASE, EPWM_COUNTER_COMPARE_B, 214);	
    EPWM_setCounterCompareShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_ACTION_QUALIFIER_A);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_ACTION_QUALIFIER_B);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_ADC_TRIGGER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_setRisingEdgeDelayCountShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableRisingEdgeDelayCountShadowLoadMode(PWM_ADC_TRIGGER_BASE);	
    EPWM_setFallingEdgeDelayCountShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableFallingEdgeDelayCountShadowLoadMode(PWM_ADC_TRIGGER_BASE);	
    EPWM_setDeadBandControlShadowLoadMode(PWM_ADC_TRIGGER_BASE, EPWM_DB_LOAD_ON_CNTR_ZERO);	
    EPWM_disableDeadBandControlShadowLoadMode(PWM_ADC_TRIGGER_BASE);	
    EPWM_enableADCTrigger(PWM_ADC_TRIGGER_BASE, EPWM_SOC_A);	
    EPWM_setADCTriggerSource(PWM_ADC_TRIGGER_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);	
    EPWM_setADCTriggerEventPrescale(PWM_ADC_TRIGGER_BASE, EPWM_SOC_A, 7);	
    EPWM_enableADCTrigger(PWM_ADC_TRIGGER_BASE, EPWM_SOC_B);	
    EPWM_setADCTriggerSource(PWM_ADC_TRIGGER_BASE, EPWM_SOC_B, EPWM_SOC_TBCTR_U_CMPB);	
    EPWM_setADCTriggerEventPrescale(PWM_ADC_TRIGGER_BASE, EPWM_SOC_B, 7);	
}
void PWM_BUZZER_init(){
    EPWM_setClockPrescaler(PWM_BUZZER_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);	
    EPWM_setPeriodLoadMode(PWM_BUZZER_BASE, EPWM_PERIOD_DIRECT_LOAD);	
    EPWM_setTimeBasePeriod(PWM_BUZZER_BASE, 1561);	
    EPWM_setTimeBaseCounter(PWM_BUZZER_BASE, 0);	
    EPWM_setTimeBaseCounterMode(PWM_BUZZER_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_disablePhaseShiftLoad(PWM_BUZZER_BASE);	
    EPWM_setPhaseShift(PWM_BUZZER_BASE, 0);	
    EPWM_setCounterCompareValue(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_A, 781);	
    EPWM_setCounterCompareShadowLoadMode(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_B, 0);	
    EPWM_setCounterCompareShadowLoadMode(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setActionQualifierContSWForceShadowMode(PWM_BUZZER_BASE, EPWM_AQ_SW_IMMEDIATE_LOAD);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_BUZZER_BASE, EPWM_ACTION_QUALIFIER_A);	
    EPWM_setActionQualifierContSWForceAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_BUZZER_BASE, EPWM_ACTION_QUALIFIER_B);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_BUZZER_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_setRisingEdgeDelayCountShadowLoadMode(PWM_BUZZER_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableRisingEdgeDelayCountShadowLoadMode(PWM_BUZZER_BASE);	
    EPWM_setFallingEdgeDelayCountShadowLoadMode(PWM_BUZZER_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableFallingEdgeDelayCountShadowLoadMode(PWM_BUZZER_BASE);	
    EPWM_setDeadBandControlShadowLoadMode(PWM_BUZZER_BASE, EPWM_DB_LOAD_ON_CNTR_ZERO);	
    EPWM_disableDeadBandControlShadowLoadMode(PWM_BUZZER_BASE);	
    EPWM_enableInterrupt(PWM_BUZZER_BASE);	
    EPWM_setInterruptSource(PWM_BUZZER_BASE, EPWM_INT_TBCTR_ZERO);	
    EPWM_setInterruptEventCount(PWM_BUZZER_BASE, 8);	
}
void PWM_FAN_init(){
    EPWM_setClockPrescaler(PWM_FAN_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);	
    EPWM_setTimeBasePeriod(PWM_FAN_BASE, 3999);	
    EPWM_setTimeBaseCounter(PWM_FAN_BASE, 0);	
    EPWM_setTimeBaseCounterMode(PWM_FAN_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_disablePhaseShiftLoad(PWM_FAN_BASE);	
    EPWM_setPhaseShift(PWM_FAN_BASE, 0);	
    EPWM_setCounterCompareValue(PWM_FAN_BASE, EPWM_COUNTER_COMPARE_A, 0);	
    EPWM_setCounterCompareShadowLoadMode(PWM_FAN_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(PWM_FAN_BASE, EPWM_COUNTER_COMPARE_B, 0);	
    EPWM_setCounterCompareShadowLoadMode(PWM_FAN_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_FAN_BASE, EPWM_ACTION_QUALIFIER_A);	
    EPWM_setActionQualifierContSWForceAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_disableActionQualifierShadowLoadMode(PWM_FAN_BASE, EPWM_ACTION_QUALIFIER_B);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(PWM_FAN_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_setRisingEdgeDelayCountShadowLoadMode(PWM_FAN_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableRisingEdgeDelayCountShadowLoadMode(PWM_FAN_BASE);	
    EPWM_setFallingEdgeDelayCountShadowLoadMode(PWM_FAN_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableFallingEdgeDelayCountShadowLoadMode(PWM_FAN_BASE);	
    EPWM_setDeadBandControlShadowLoadMode(PWM_FAN_BASE, EPWM_DB_LOAD_ON_CNTR_ZERO);	
    EPWM_disableDeadBandControlShadowLoadMode(PWM_FAN_BASE);	
}

//*****************************************************************************
//
// EPWMXBAR Configurations
//
//*****************************************************************************
void EPWMXBAR_init(){
	EPWM_XBAR_PEAK_CURRENT_init();
	EPWM_XBAR_HARD_FAULT_init();
}

void EPWM_XBAR_PEAK_CURRENT_init(){
		
	XBAR_setEPWMMuxConfig(EPWM_XBAR_PEAK_CURRENT, XBAR_EPWM_MUX00_CMPSS1_CTRIPH_OR_L);
	XBAR_setEPWMMuxConfig(EPWM_XBAR_PEAK_CURRENT, XBAR_EPWM_MUX02_CMPSS2_CTRIPH_OR_L);
	XBAR_enableEPWMMux(EPWM_XBAR_PEAK_CURRENT, XBAR_MUX00 | XBAR_MUX02);
}
void EPWM_XBAR_HARD_FAULT_init(){
		
	XBAR_setEPWMMuxConfig(EPWM_XBAR_HARD_FAULT, XBAR_EPWM_MUX04_CMPSS3_CTRIPH);
	XBAR_setEPWMMuxConfig(EPWM_XBAR_HARD_FAULT, XBAR_EPWM_MUX06_CMPSS4_CTRIPH);
	XBAR_setEPWMMuxConfig(EPWM_XBAR_HARD_FAULT, XBAR_EPWM_MUX12_CMPSS7_CTRIPH_OR_L);
	XBAR_enableEPWMMux(EPWM_XBAR_HARD_FAULT, XBAR_MUX04 | XBAR_MUX06 | XBAR_MUX12);
}

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init(){
	LED_1_init();
	LED_2_init();
	LED_3_init();
	LED_4_init();
	FAN_TACH_init();
}

void LED_1_init(){
	GPIO_writePin(LED_1, 1);
	GPIO_setPadConfig(LED_1, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED_1, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED_1, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED_1, GPIO_CORE_CPU1);
}
void LED_2_init(){
	GPIO_writePin(LED_2, 1);
	GPIO_setPadConfig(LED_2, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED_2, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED_2, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED_2, GPIO_CORE_CPU1);
}
void LED_3_init(){
	GPIO_writePin(LED_3, 1);
	GPIO_setPadConfig(LED_3, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED_3, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED_3, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED_3, GPIO_CORE_CPU1);
}
void LED_4_init(){
	GPIO_writePin(LED_4, 1);
	GPIO_setPadConfig(LED_4, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED_4, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED_4, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED_4, GPIO_CORE_CPU1);
}
void FAN_TACH_init(){
	GPIO_setPadConfig(FAN_TACH, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(FAN_TACH, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(FAN_TACH, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(FAN_TACH, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
void INPUTXBAR_init(){
	FAN_TACH_INPUT_init();
}

void FAN_TACH_INPUT_init(){
	XBAR_setInputPin(FAN_TACH_INPUT_INPUT, FAN_TACH_INPUT_SOURCE);
}

//*****************************************************************************
//
// MEMCFG Configurations
//
//*****************************************************************************
void MEMCFG_init(){
	//
	// Initialize RAMs
	//
	MemCfg_initSections(MEMCFG_SECT_MSGCPUTOCLA1);
	MemCfg_initSections(MEMCFG_SECT_MSGCLA1TOCPU);
	while(!MemCfg_getInitStatus(MEMCFG_SECT_MSGCPUTOCLA1));
	while(!MemCfg_getInitStatus(MEMCFG_SECT_MSGCLA1TOCPU));
	//
	// Configure LSRAMs
	//
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS0, MEMCFG_LSRAMCONTROLLER_CPU_CLA1);
	MemCfg_setCLAMemType(MEMCFG_SECT_LS0, MEMCFG_CLA_MEM_PROGRAM);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS1, MEMCFG_LSRAMCONTROLLER_CPU_CLA1);
	MemCfg_setCLAMemType(MEMCFG_SECT_LS1, MEMCFG_CLA_MEM_PROGRAM);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS2, MEMCFG_LSRAMCONTROLLER_CPU_CLA1);
	MemCfg_setCLAMemType(MEMCFG_SECT_LS2, MEMCFG_CLA_MEM_DATA);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS3, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS4, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS5, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS6, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
	MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS7, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
	//
	// Configure GSRAMs
	//
	//
	// Configure Access Protection for RAMs
	//
	MemCfg_setProtection(MEMCFG_SECT_LS0, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS1, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS2, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS3, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS4, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS5, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS6, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_LS7, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE);
	MemCfg_setProtection(MEMCFG_SECT_GS0, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE | MEMCFG_PROT_ALLOWDMAWRITE);
	MemCfg_setProtection(MEMCFG_SECT_GS1, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE | MEMCFG_PROT_ALLOWDMAWRITE);
	MemCfg_setProtection(MEMCFG_SECT_GS2, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE | MEMCFG_PROT_ALLOWDMAWRITE);
	MemCfg_setProtection(MEMCFG_SECT_GS3, MEMCFG_PROT_ALLOWCPUFETCH | MEMCFG_PROT_ALLOWCPUWRITE | MEMCFG_PROT_ALLOWDMAWRITE);
	//
	// Lock/Commit Registers
	//
	//
	// Enable Access Violation Interrupt
	//
	//
	// Correctable error Interrupt
	//
	MemCfg_setCorrErrorThreshold(0);
	MemCfg_disableCorrErrorInterrupt(MEMCFG_CERR_CPUREAD);
}        
//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void SCI_init(){
	SCI_DEBUG_init();
}

void SCI_DEBUG_init(){
	SCI_clearInterruptStatus(SCI_DEBUG_BASE, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
	SCI_clearOverflowStatus(SCI_DEBUG_BASE);
	SCI_resetTxFIFO(SCI_DEBUG_BASE);
	SCI_resetRxFIFO(SCI_DEBUG_BASE);
	SCI_resetChannels(SCI_DEBUG_BASE);
	SCI_setConfig(SCI_DEBUG_BASE, DEVICE_LSPCLK_FREQ, SCI_DEBUG_BAUDRATE, (SCI_CONFIG_WLEN_8|SCI_CONFIG_STOP_ONE|SCI_CONFIG_PAR_NONE));
	SCI_disableLoopback(SCI_DEBUG_BASE);
	SCI_performSoftwareReset(SCI_DEBUG_BASE);
	SCI_enableFIFO(SCI_DEBUG_BASE);
	SCI_enableModule(SCI_DEBUG_BASE);
}

//*****************************************************************************
//
// SYNC Scheme Configurations
//
//*****************************************************************************
void SYNC_init(){
	SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
	//
	// For EPWM1, the sync input is: SYSCTL_SYNC_IN_SRC_EXTSYNCIN1
	//
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM7, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP1, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP6, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	//
	// SOCA
	//
	SysCtl_enableExtADCSOCSource(0);
	//
	// SOCB
	//
	SysCtl_enableExtADCSOCSource(0);
}
