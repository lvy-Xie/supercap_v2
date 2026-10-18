#ifndef CONTROL_CLA_H
#define CONTROL_CLA_H
// 仅由 cla_tasks.cla 编译。CPU 不编译、不调用这些控制函数。
#include "control.h"
#include "cla_pid.h"
#pragma CODE_SECTION(Control_Loop, "Cla1Prog")
#pragma CODE_SECTION(Test_Charge_Loop, "Cla1Prog")
#pragma CODE_SECTION(Power_Control_Loop, "Cla1Prog")
#pragma CODE_SECTION(Cla_CurrentVoltageLoop, "Cla1Prog")
#pragma CODE_SECTION(Update_PWM_Duty, "Cla1Prog")
#pragma CODE_SECTION(Cla_ControlStop, "Cla1Prog")
#pragma CODE_SECTION(Cla_ControlInit, "Cla1Prog")
#pragma CODE_SECTION(Cla_ResetPID, "Cla1Prog")
#pragma CODE_SECTION(Cla_ResetPI, "Cla1Prog")
#pragma CODE_SECTION(Cla_RunPID, "Cla1Prog")
#pragma CODE_SECTION(DCL_runPI_L4, "Cla1Prog")
#pragma CODE_SECTION(Cla_RecordDiagnostics, "Cla1Prog")

static Control_Diagnostics_s diagWindow;
static uint16_t diagSamples;
static float diagFeedbackSum;

// path: 0=停波前馈，1=电流 PI，2=无效参数。只观测，不参与控制决策。
static void Cla_RecordDiagnostics(uint16_t path, float feedback)
{
    float integral = CurrentA_loop.i10;
    if (diagSamples == 0U) {
        diagWindow.pi_samples = diagWindow.tracking_samples = 0U;
        diagWindow.voltage_limited_samples = diagWindow.integral_blocked_samples = 0U;
        diagWindow.invalid_samples = 0U;
        diagWindow.feedback_min = 3.402823466e38f;
        diagWindow.feedback_max = -3.402823466e38f;
        diagWindow.integral_min = integral;
        diagWindow.integral_max = integral;
        diagWindow.peak_i_in = diagWindow.peak_i_out = 0.0f;
        diagWindow.peak_i_in_raw = diagWindow.peak_i_out_raw = 0U;
        diagFeedbackSum = 0.0f;
    }
    if (path == 2U) {
        ++diagWindow.invalid_samples;
    } else {
        if (path == 0U) ++diagWindow.tracking_samples;
        else {
            ++diagWindow.pi_samples;
            if (Current_VoltageLimited) ++diagWindow.voltage_limited_samples;
            if (CurrentA_loop.i6 == 0.0f) ++diagWindow.integral_blocked_samples;
        }
        diagFeedbackSum += feedback;
        if (feedback < diagWindow.feedback_min) diagWindow.feedback_min = feedback;
        if (feedback > diagWindow.feedback_max) {
            diagWindow.feedback_max = feedback;
            diagWindow.peak_i_in = SuperCap_Control.Electronic_Data.I_IN;
            diagWindow.peak_i_out = SuperCap_Control.Electronic_Data.I_OUT;
            diagWindow.peak_i_in_raw = SuperCap_Control.Electronic_Data.I_IN_Raw;
            diagWindow.peak_i_out_raw = SuperCap_Control.Electronic_Data.I_OUT_Raw;
        }
    }
    if (integral < diagWindow.integral_min) diagWindow.integral_min = integral;
    if (integral > diagWindow.integral_max) diagWindow.integral_max = integral;
    if (++diagSamples < CONTROL_DIAG_WINDOW_SAMPLES) return;

    Control_Diag.pi_samples = diagWindow.pi_samples;
    Control_Diag.tracking_samples = diagWindow.tracking_samples;
    Control_Diag.voltage_limited_samples = diagWindow.voltage_limited_samples;
    Control_Diag.integral_blocked_samples = diagWindow.integral_blocked_samples;
    Control_Diag.invalid_samples = diagWindow.invalid_samples;
    if (diagWindow.invalid_samples < CONTROL_DIAG_WINDOW_SAMPLES) {
        Control_Diag.feedback_mean = diagFeedbackSum /
            (float)(CONTROL_DIAG_WINDOW_SAMPLES - diagWindow.invalid_samples);
        Control_Diag.feedback_min = diagWindow.feedback_min;
        Control_Diag.feedback_max = diagWindow.feedback_max;
    } else {
        Control_Diag.feedback_mean = Control_Diag.feedback_min = Control_Diag.feedback_max = 0.0f;
    }
    Control_Diag.integral_min = diagWindow.integral_min;
    Control_Diag.integral_max = diagWindow.integral_max;
    Control_Diag.integral_last = integral;
    Control_Diag.outputDuty = SuperCap_Control.PWM_Control.outputDuty;
    Control_Diag.vcap = SuperCap_Control.Electronic_Data.VCAP;
    Control_Diag.peak_i_in = diagWindow.peak_i_in;
    Control_Diag.peak_i_out = diagWindow.peak_i_out;
    Control_Diag.peak_i_in_raw = diagWindow.peak_i_in_raw;
    Control_Diag.peak_i_out_raw = diagWindow.peak_i_out_raw;
    ++Control_Diag.sequence; // 最后发布；窗口之间约保持 1 秒供 Watch 观察。
    diagSamples = 0U;
}

static inline void Cla_ResetPID(DCL_PID_CLA *p)
{
    p->d2 = p->d3 = p->i10 = 0.0f;
    p->i14 = 1.0f;
}
static inline void Cla_ResetPI(DCL_PI_CLA *p)
{
    p->i10 = p->i11 = 0.0f;
    p->i6 = 1.0f;
}
static inline void Cla_ControlStop(uint16_t reason)
{
    Control_StopRequest = reason;
    Control_LastFault = reason;
    EPWM_forceTripZoneEvent(PWM_BRIDGE_A_BASE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(PWM_BRIDGE_B_BASE, EPWM_TZ_FORCE_EVENT_OST);
    Cla_ResetPID(&Referee_Power_Loop);
    Cla_ResetPID(&Voltage_loop);
    Cla_ResetPI(&CurrentA_loop);
    target_DCDC_Power = target_cap_current = pid_i_out = pid_v_out = 0.0f;
    // CPU 在完成通知中清除使能和运行状态；CLA 不释放 OST，避免跨核启停竞争。
}
static inline void Cla_ControlInit(void)
{
    Cla_ResetPID(&Voltage_loop);
    Cla_ResetPID(&Referee_Power_Loop);
    Cla_ResetPI(&CurrentA_loop);
    Control_StopRequest = Control_LastFault = 0;
    diagSamples = 0U;
}
static void Cla_CurrentVoltageLoop(float targetI, float feedback, float minCurrent)
{
    float vbus = SuperCap_Control.Electronic_Data.VBUS;
    float vcap = SuperCap_Control.Electronic_Data.VCAP;
    float targetV = set_voltage;
    float duty, voltageDuty, ratio;
    current_feedback = feedback;
    Current_VoltageLimited = 0;
    if (!(vbus > 0.0f && vbus < 40.0f && vcap >= 0.0f && vcap < 40.0f &&
          targetV > 0.0f && targetV <= MAX_CAP_VOLTAGE &&
          targetI >= minCurrent && targetI <= MAX_CHARGE_CURRENT &&
          feedback >= -3.402823466e38f && feedback <= 3.402823466e38f &&
          CurrentA_loop.Kp >= 0.0f && CurrentA_loop.Kp <= 3.402823466e38f &&
          CurrentA_loop.Ki >= 0.0f && CurrentA_loop.Ki <= 3.402823466e38f)) {
        Cla_ControlStop(1U);
        Cla_RecordDiagnostics(2U, feedback);
        return;
    }
    ratio = vcap / vbus;
    if (ratio < MIN_OUTPUT_DUTY) ratio = MIN_OUTPUT_DUTY;
    if (ratio > MAX_OUTPUT_DUTY) ratio = MAX_OUTPUT_DUTY;
    if (!SuperCap_Control.PWM_Running || !SuperCap_Control.Supercap_Enable_Flag) {
        Cla_ResetPID(&Referee_Power_Loop);
        Cla_ResetPID(&Voltage_loop);
        Voltage_loop.i10 = vcap;
        Cla_ResetPI(&CurrentA_loop);
        CurrentA_loop.i10 = ratio; // 停波时跟踪电压前馈，清除旧积分/饱和状态。
        pid_i_out = ratio;
        pid_v_out = vcap;
        Update_PWM_Duty(ratio);
        Cla_RecordDiagnostics(0U, feedback);
        return;
    }
    // 直接输出广义占空比；C3 为并联 PI，Ki 每次累加误差，不再乘 Kp/Ts。
    duty = DCL_runPI_L4(&CurrentA_loop, targetI, feedback);
    pid_i_out = duty;
    // 闭环启动即运行电压 PID，与电流 PI 选取较小占空比。
    pid_v_out = Cla_RunPID(&Voltage_loop, targetV, vcap, 1.0f);
    voltageDuty = pid_v_out / vbus;
    if (voltageDuty < MIN_OUTPUT_DUTY) voltageDuty = MIN_OUTPUT_DUTY;
    if (duty > voltageDuty) {
        duty = voltageDuty;
        Current_VoltageLimited = 1;
        // 电压环接管时让电流积分跟踪实际输出，避免未选中的电流环累积。
        CurrentA_loop.i10 = duty - CurrentA_loop.Kp * (targetI - feedback);
        CurrentA_loop.i6 = 0.0f;
    } else {
        Voltage_loop.i10 = duty * vbus;
        Voltage_loop.i14 = 0.0f;
    }
    Update_PWM_Duty(duty);
    Cla_RecordDiagnostics(1U, feedback);
}

void Test_Charge_Loop(void)
{
    Cla_CurrentVoltageLoop(set_current, SuperCap_Control.Electronic_Data.I_CAP, 0.0f);
}

void Power_Control_Loop(void)
{
    float vbus = SuperCap_Control.Electronic_Data.VBUS;
    float vcap = SuperCap_Control.Electronic_Data.VCAP;
    float inputI = SuperCap_Control.Electronic_Data.I_IN;
    float target = set_power;
    float capOut, capIn, busLimit, capLimit;
    power_feedback = vbus * inputI; // 与 Reborn 一致，控制电源总输入功率。
    if (!(vbus > 0.0f && vbus < 40.0f && vcap >= 0.0f && vcap < 40.0f &&
          target >= 0.0f && target <= 3.402823466e38f &&
          power_feedback >= -3.402823466e38f && power_feedback <= 3.402823466e38f)) {
        Cla_ControlStop(1U);
        return;
    }
    // 低压时限制放电，5～12 V 线性提高放电电流；充电上限 15 A，低于 5 V 为 4 A。
    capOut = MIN_CAP_IOUT + (MAX_CAP_IOUT - MIN_CAP_IOUT) *
        (vcap - CAP_V_CUTOFF) / (CAP_V_NORMAL - CAP_V_CUTOFF);
    if (capOut < MIN_CAP_IOUT) capOut = MIN_CAP_IOUT;
    if (capOut > MAX_CAP_IOUT) capOut = MAX_CAP_IOUT;
    capIn = vcap < CAP_V_CUTOFF ? 4.0f : MAX_CHARGE_CURRENT;
    busLimit = MAX_CHARGE_CURRENT * vbus;
    capLimit = capIn * vcap;
    Referee_Power_Loop.Umax = busLimit < capLimit ? busLimit : capLimit;
    capLimit = capOut * vcap;
    Referee_Power_Loop.Umin = -(busLimit < capLimit ? busLimit : capLimit);
    target_DCDC_Power = target_cap_current = 0.0f;
    if (SuperCap_Control.PWM_Running && SuperCap_Control.Supercap_Enable_Flag) {
        target_DCDC_Power = Cla_RunPID(&Referee_Power_Loop, target, power_feedback, 1.0f);
        // 不做效率前馈，损耗偏差由输入功率闭环修正；零电压时目标为零。
        target_cap_current = vcap > 0.0f ? target_DCDC_Power / vcap : 0.0f;
        if (target_cap_current > capIn) target_cap_current = capIn;
        if (target_cap_current < -capOut) target_cap_current = -capOut;
    }
    Cla_CurrentVoltageLoop(target_cap_current, SuperCap_Control.Electronic_Data.I_CAP, -MAX_CHARGE_CURRENT);
}

void Control_Loop(void)
{
    if (Control_Mode == CONTROL_MODE_CHARGE) Test_Charge_Loop();
    else if (Control_Mode == CONTROL_MODE_POWER) Power_Control_Loop();
    else Cla_ControlStop(1U);
}

void Update_PWM_Duty(float outputDuty)
{
    volatile PWM_Control_s *p = &SuperCap_Control.PWM_Control;
    if (!(outputDuty >= 0.0f && outputDuty <= 3.402823466e38f)) {
        Cla_ControlStop(1U);
        return;
    }
    if (outputDuty < MIN_OUTPUT_DUTY) outputDuty = MIN_OUTPUT_DUTY;
    if (outputDuty > MAX_OUTPUT_DUTY) outputDuty = MAX_OUTPUT_DUTY;
    p->outputDuty = outputDuty;
    if (p->BuckBoostMode) {
        if (outputDuty < 0.8f || outputDuty > 1.25f) p->BuckBoostMode = false;
    } else {
        if (outputDuty > 0.9f && outputDuty < 1.111f) p->BuckBoostMode = true;
    }
    // 沿用 49C 的广义占空比：VCAP / VBUS = dutyA / dutyB。
    if (p->BuckBoostMode) {
        p->dutyA = (outputDuty + 1.0f) * 0.4f;
        p->dutyB = (1.0f / outputDuty + 1.0f) * 0.4f;
    } else if (outputDuty < 1.0f) {
        p->dutyA = MAX_PWM_DUTY * outputDuty;
        p->dutyB = MAX_PWM_DUTY;
    } else {
        p->dutyA = MAX_PWM_DUTY;
        p->dutyB = MAX_PWM_DUTY / outputDuty;
    }
    // 沿用 49C/TI 例程的 Q8 比较值；保留小数，不能先截断为 TBCLK 整数。
    // 最短脉冲 17.16 拍，仍大于 5 拍死区及 HRPWM 的边界限制。
    p->COMPA_A_Q8 = (uint32_t)((1.0f - p->dutyA) * (PWM_COUNTS * 256.0f));
    p->COMPA_B_Q8 = (uint32_t)((1.0f - p->dutyB) * (PWM_COUNTS * 256.0f));
    // TI 互补死区例程要求 CMPxHR 非零；整数点只偏移 1/256 TBCLK。
    if ((p->COMPA_A_Q8 & 0xFFU) == 0U) p->COMPA_A_Q8 |= 1U;
    if ((p->COMPA_B_Q8 & 0xFFU) == 0U) p->COMPA_B_Q8 |= 1U;
    p->COMPA_A = (uint16_t)(p->COMPA_A_Q8 >> 8U);
    p->COMPA_B = (uint16_t)(p->COMPA_B_Q8 >> 8U);
    HRPWM_setCounterCompareValue(PWM_BRIDGE_A_BASE, HRPWM_COUNTER_COMPARE_A, p->COMPA_A_Q8);
    HRPWM_setCounterCompareValue(PWM_BRIDGE_A_BASE, HRPWM_COUNTER_COMPARE_B, p->COMPA_A_Q8);
    HRPWM_setCounterCompareValue(PWM_BRIDGE_B_BASE, HRPWM_COUNTER_COMPARE_A, p->COMPA_B_Q8);
    HRPWM_setCounterCompareValue(PWM_BRIDGE_B_BASE, HRPWM_COUNTER_COMPARE_B, p->COMPA_B_Q8);
}

#endif
