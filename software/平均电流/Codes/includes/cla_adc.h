#ifndef CLA_ADC_H
#define CLA_ADC_H
#include <stdbool.h>
#include <stdint.h>
#include "cla_adc_shared.h"

extern volatile bool CLA_Ready;
extern volatile uint16_t CLA_Fault; // 0=正常，1=初始化超时，2=任务溢出，3=采样停止，4=DMA采集链故障。
extern volatile uint16_t AdcDma_Buffer[12]; // GS0：A0..3、B0..3、C0..3；包含未使用的结果槽。
extern volatile uint16_t AdcDma_Frames;
extern volatile uint16_t AdcDma_Fault; // 0正常，1 DMA溢出，2 ADC溢出，3 CLA上一帧未完成，4 DMA未完成却中断。
void AdcDma_Init(void);
__interrupt void AdcDma_ISR(void);
bool ClaADC_Init(void);
void ClaADC_ArmMonitor(void);
void ClaADC_CheckProgress(void); // 前台约每 1 ms 调用一次。
#endif
