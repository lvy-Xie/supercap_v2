#ifndef SUPERCAP_H
#define SUPERCAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t BuckBoostMode; // 跨 CPU/CLA 不共享 bool。
    float outputDuty;
    float dutyA, dutyB;
    uint16_t COMPA_A, COMPA_B;
    uint32_t COMPA_A_Q8, COMPA_B_Q8; // HRPWM API 使用的 Q8 比较值。
} PWM_Control_s;

typedef struct {
    float VBUS, VCAP;
    float I_IN, I_OUT, I_CAP;
    float I_A, I_B;
    uint16_t Raw[7]; // 桥 A、桥 B、输入电流、电容电流、底盘电流、母线电压、电容电压。
    // 当前 ADC 原始码（0..4095），未经校准和滤波；沿用物理量名方便 CCS 观察。
    uint16_t VBUS_Raw, VCAP_Raw;
    uint16_t I_IN_Raw, I_OUT_Raw, I_CAP_Raw;
    uint16_t I_A_Raw, I_B_Raw;
    float I_DCDC; // 沿用 Reborn：输入侧变换器电流 = I_IN - I_OUT。
} Electronic_Data_s;

typedef struct {
    PWM_Control_s PWM_Control;
    Electronic_Data_s Electronic_Data;
    uint16_t Supercap_Enable_Flag; // 唯一使能入口：写 1 请求启动，写 0 停波；上电/故障为 0。
    uint16_t PWM_Running;          // CPU 维护的实际出波状态，只读观测，不是第二个使能入口。
} SuperCap_Control_s;

// 两个编译器均检查相同布局，避免 CLA 的 32 位 int/bool 改变调试结构偏移。
typedef char PWMSharedSizeCheck[(sizeof(PWM_Control_s) == 7U * sizeof(uint32_t)) ? 1 : -1];
typedef char ElectronicSharedSizeCheck[(sizeof(Electronic_Data_s) == 15U * sizeof(uint32_t)) ? 1 : -1];
typedef char ControlSharedSizeCheck[(sizeof(SuperCap_Control_s) == 23U * sizeof(uint32_t)) ? 1 : -1];
typedef char ElectronicSharedOffsetCheck[(offsetof(SuperCap_Control_s, Electronic_Data) == 7U * sizeof(uint32_t)) ? 1 : -1];

extern volatile SuperCap_Control_s SuperCap_Control;
extern int MEP_ScaleFactor;
extern volatile uint16_t SFO_Status;
extern volatile bool HRPWM_Ready;
void Supercap_Init(void);
void Supercap_ServiceHRPWM(void);
void Supercap_BackgroundPoll(void); // 快速轮询串口，Timer1 每 1 ms 调用一次 CLA/SFO 后台检查。
void Stop_PWM(void);
#endif
