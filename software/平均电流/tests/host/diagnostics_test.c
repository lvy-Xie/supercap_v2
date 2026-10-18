#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

int main(void)
{
    Supercap_Init();
    SuperCap_Control.Electronic_Data.VBUS = 24.0f;
    SuperCap_Control.Electronic_Data.VCAP = 5.26f;
    SuperCap_Control.Electronic_Data.I_CAP = -0.06f;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES - 1U; ++n) run_control();
    assert(Control_Diag.sequence == 0U); // 完整窗口后才发布，窗口内保留上一份结果。
    run_control();
    assert(Control_Diag.sequence == 1U);
    assert(Control_Diag.tracking_samples == CONTROL_DIAG_WINDOW_SAMPLES);
    assert(Control_Diag.pi_samples == 0U && Control_Diag.invalid_samples == 0U);
    assert(fabsf(Control_Diag.feedback_mean + 0.06f) < 0.0001f);
    assert(Control_Diag.feedback_min == -0.06f && Control_Diag.feedback_max == -0.06f);
    assert(fabsf(Control_Diag.integral_last - 5.26f / 24.0f) < 0.00001f);
    assert(Control_Diag.integral_min == Control_Diag.integral_max);

    SuperCap_Control.Supercap_Enable_Flag = SuperCap_Control.PWM_Running = 1U;
    SuperCap_Control.Electronic_Data.I_CAP = 0.9f;
    float oldIntegral = CurrentA_loop.i10;
    unsigned voltageLimited = 0U, integralBlocked = 0U;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES; ++n) {
        run_control();
        if (Current_VoltageLimited) ++voltageLimited;
        if (CurrentA_loop.i6 == 0.0f) ++integralBlocked;
    }
    assert(Control_Diag.sequence == 2U);
    assert(Control_Diag.pi_samples == CONTROL_DIAG_WINDOW_SAMPLES);
    assert(Control_Diag.tracking_samples == 0U && Control_Diag.invalid_samples == 0U);
    // 电压环从首拍参与选小，低电压长时间固定反馈也可能触发限幅。
    assert(voltageLimited > 0U && integralBlocked > 0U);
    assert(Control_Diag.voltage_limited_samples == voltageLimited);
    assert(Control_Diag.integral_blocked_samples == integralBlocked);
    assert(fabsf(Control_Diag.feedback_mean - 0.9f) < 0.001f);
    assert(Control_Diag.integral_last > oldIntegral);
    assert(Control_Diag.integral_last <= Voltage_loop.Umax / 24.0f);
    assert(Control_Diag.integral_max > Control_Diag.integral_min);
    assert(Control_Diag.integral_last == CurrentA_loop.i10);
    assert(Control_Diag.outputDuty == SuperCap_Control.PWM_Control.outputDuty);
    assert(Control_Diag.vcap == 5.26f);

    // 正负摆动的反馈即使平均接近目标，Watch 单次读数也可能接近零。
    Control_Init(); Cla_ControlInit();
    set_current = 1.0f; // 本用例验证均值为 1 A 的零平均误差，不依赖固件默认目标。
    CurrentA_loop.i10 = 0.22f;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES; ++n) {
        SuperCap_Control.Electronic_Data.I_CAP = n & 1U ? 2.0f : 0.0f;
        run_control();
    }
    assert(Control_Diag.pi_samples == CONTROL_DIAG_WINDOW_SAMPLES);
    assert(Control_Diag.feedback_mean == 1.0f);
    assert(Control_Diag.feedback_min == 0.0f && Control_Diag.feedback_max == 2.0f);
    assert(fabsf(CurrentA_loop.i10 - 0.22f) < 0.00001f);

    SuperCap_Control.Electronic_Data.I_CAP = NAN;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES; ++n) run_control();
    assert(Control_Diag.invalid_samples == CONTROL_DIAG_WINDOW_SAMPLES);
    assert(Control_Diag.pi_samples == 0U && Control_Diag.tracking_samples == 0U);
    assert(Control_Diag.feedback_mean == 0.0f && isfinite(Control_Diag.integral_last));

    // 真实 ADC -> DMA -> CLA 路径锁存最大反馈那一帧，不能混入下一帧原始码。
    Supercap_Init();
    adc[1][0] = 2987; adc[1][1] = 640;
    adc[2][3] = 2019;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES; ++n) {
        adc[2][1] = n == 321U ? 1837 : 1966;
        adc[0][1] = n == 321U ? 2195 : 2014;
        sample_tick();
    }
    assert(Control_Diag.sequence == 1U);
    assert(Control_Diag.peak_i_in_raw == 2195 && Control_Diag.peak_i_out_raw == 2019);
    assert(SuperCap_Control.Electronic_Data.I_IN_Raw == 2014);
    assert(fabsf(Control_Diag.feedback_max - (1.0f + 129.0f * 0.02014652081f)) < 0.00001f);
    // 下一窗口 I_CAP 峰值帧的旁路电流快照也必须替换，不能沿用旧窗口。
    adc[0][1] = 2014;
    for (unsigned n = 0; n < CONTROL_DIAG_WINDOW_SAMPLES; ++n) {
        adc[2][1] = n == 456U ? 1800 : 1966;
        adc[2][3] = n == 456U ? 1837 : 2019;
        sample_tick();
    }
    assert(Control_Diag.sequence == 2U);
    assert(Control_Diag.peak_i_in_raw == 2014 && Control_Diag.peak_i_out_raw == 1837);
    assert(fabsf(Control_Diag.feedback_max - (1.0f + 166.0f * 0.02014652081f)) < 0.00001f);
    puts("通过：约1秒诊断窗口、停波前馈/PI分支计数、反馈均值与峰值、积分变化及异常窗口隔离");
    puts("通过：I_CAP反馈峰值、同帧输入/底盘电流快照锁存与下一窗口替换");
}
