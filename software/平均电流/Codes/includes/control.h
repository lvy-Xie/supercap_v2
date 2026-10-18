#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include "DCLCLA.h"

extern DCL_PID_CLA Voltage_loop;
extern DCL_PID_CLA Referee_Power_Loop;
extern DCL_PI_CLA CurrentA_loop;
#define CONTROL_MODE_CHARGE 0U
#define CONTROL_MODE_POWER 1U
extern volatile uint16_t Control_Mode; // 停波后切换；0=电容恒流充电，1=输入功率闭环。
extern volatile float set_power, power_feedback, target_DCDC_Power, target_cap_current;
extern volatile float set_voltage;
extern volatile float set_current, current_feedback, pid_i_out, pid_v_out;
extern volatile uint16_t Current_VoltageLimited;
extern volatile uint16_t Control_StopRequest, Control_LastFault;

// 每 50000 次控制调用发布一次，只读诊断；约 1 秒窗口，避免用异步 Watch 判断瞬时反馈。
#define CONTROL_DIAG_WINDOW_SAMPLES 50000U
typedef struct {
    uint16_t sequence;
    uint16_t pi_samples, tracking_samples, voltage_limited_samples;
    uint16_t integral_blocked_samples, invalid_samples;
    float feedback_mean, feedback_min, feedback_max;
    float integral_min, integral_max, integral_last;
    float outputDuty, vcap;
    // 同一个最大反馈样本的两路电流及原始码，不受 Watch 分项刷新影响。
    float peak_i_in, peak_i_out;
    uint16_t peak_i_in_raw, peak_i_out_raw;
} Control_Diagnostics_s;
typedef char ControlDiagSizeCheck[(sizeof(Control_Diagnostics_s) == 14U * sizeof(uint32_t)) ? 1 : -1];
extern volatile Control_Diagnostics_s Control_Diag;
typedef char PIDSharedSizeCheck[(sizeof(DCL_PID_CLA) == 12U * sizeof(float)) ? 1 : -1];
typedef char PISharedSizeCheck[(sizeof(DCL_PI_CLA) == 9U * sizeof(float)) ? 1 : -1];
void Control_Init(void);
void Control_Loop(void);
void Test_Charge_Loop(void);
void Power_Control_Loop(void);
void Update_PWM_Duty(float outputDuty);
#endif
