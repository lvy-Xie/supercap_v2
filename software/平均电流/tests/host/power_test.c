#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

static void near(float a, float b) { assert(fabsf(a - b) < 0.0001f); }

static void data(float vbus, float vcap, float inputI, float capI)
{
    SuperCap_Control.Electronic_Data.VBUS = vbus;
    SuperCap_Control.Electronic_Data.VCAP = vcap;
    SuperCap_Control.Electronic_Data.I_IN = inputI;
    SuperCap_Control.Electronic_Data.I_CAP = capI;
    SuperCap_Control.Electronic_Data.I_DCDC = NAN; // 功率模式也只用 I_CAP。
}

static void prepare(float vbus, float vcap, float inputI)
{
    Stop_PWM();
    data(vbus, vcap, inputI, 0);
    run_control();
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
}

// 简化双环对象，含 10% 损耗，验证没有效率前馈也能消除输入功率偏差。
static float settle(float loadPower, float capI)
{
    for (unsigned n = 0; n < 20000; ++n) {
        float converterPower = capI >= 0 ? capI * 12.0f / 0.9f : capI * 12.0f * 0.9f;
        data(24, 12, (loadPower + converterPower) / 24.0f, capI);
        run_control();
        capI += 0.04f * (60.0f * (SuperCap_Control.PWM_Control.outputDuty - 0.5f) - capI);
        assert(SuperCap_Control.PWM_Running && isfinite(capI));
    }
    assert(fabsf(power_feedback - set_power) < 0.01f);
    return capI;
}

int main(void)
{
    Supercap_Init();
    assert(Control_Mode == CONTROL_MODE_CHARGE);
    Control_Mode = CONTROL_MODE_POWER;
    set_current = NAN; // 功率模式不依赖充电测试的目标。
    set_voltage = 28;
    set_power = 30;
    prepare(24, 12, 1);
    near(Referee_Power_Loop.i10, 0);
    near(CurrentA_loop.i10, 0.5f);
    run_control();
    near(power_feedback, 24);
    assert(target_cap_current > 0 && SuperCap_Control.PWM_Control.outputDuty > 0.5f);
    near(target_cap_current, target_DCDC_Power / 12);
    near(current_feedback, 0);
    assert(SuperCap_Control.PWM_Running);
    prepare(24, 12, 2);
    run_control();
    assert(target_cap_current < 0 && SuperCap_Control.PWM_Control.outputDuty < 0.5f);

    // 同一个输入功率设定可随负载变化在充电和放电间切换。
    prepare(24, 12, 0);
    float capI = settle(6, 0);
    assert(capI > 0);
    capI = settle(54, capI);
    assert(capI < 0);
    settle(30, capI);

    // 电容低压、线性过渡、高压，以及母线侧 15 A 限幅。
    const float voltages[] = {0, 4, 5, 8.5f, 12, 28};
    const float chargeLimits[] = {0, 16, 75, 127.5f, 180, 360};
    const float dischargeLimits[] = {0, 0.4f, 0.5f, 51.425f, 144, 336};
    for (unsigned n = 0; n < sizeof(voltages) / sizeof(voltages[0]); ++n) {
        prepare(24, voltages[n], 0);
        set_power = 10000;
        run_control();
        near(Referee_Power_Loop.Umax, chargeLimits[n]);
        near(Referee_Power_Loop.Umin, -dischargeLimits[n]);
        near(target_DCDC_Power, chargeLimits[n]);
        assert(isfinite(target_cap_current) && target_cap_current <= MAX_CHARGE_CURRENT);
        prepare(24, voltages[n], 100);
        set_power = 0;
        run_control();
        near(target_DCDC_Power, -dischargeLimits[n]);
        assert(target_cap_current >= -MAX_CAP_IOUT);
    }
    prepare(2, 28, 0);
    run_control();
    near(Referee_Power_Loop.Umax, 30);
    near(Referee_Power_Loop.Umin, -30);

    // 电压限幅继续生效；停波后功率 PID 清零，电流积分跟踪电压比。
    set_voltage = 10; set_power = 10000;
    prepare(24, 12, 0);
    run_control();
    assert(Current_VoltageLimited);
    Stop_PWM(); run_control();
    near(Referee_Power_Loop.i10, 0); near(Referee_Power_Loop.d2, 0);
    near(Referee_Power_Loop.d3, 0); near(Referee_Power_Loop.i14, 1);
    near(target_cap_current, 0); near(target_DCDC_Power, 0);
    near(CurrentA_loop.i10, 0.5f);

    const float invalid[] = {NAN, INFINITY, -1};
    for (unsigned n = 0; n < sizeof(invalid) / sizeof(invalid[0]); ++n) {
        set_power = 30; prepare(24, 12, 0);
        set_power = invalid[n]; run_control();
        assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
        near(Referee_Power_Loop.i10, 0);
    }
    set_power = 30; prepare(24, 12, 0);
    data(0, 12, 0, 0); run_control();
    assert(!SuperCap_Control.PWM_Running);
    prepare(24, 12, 0);
    data(24, 12, NAN, 0); run_control();
    assert(!SuperCap_Control.PWM_Running);
    prepare(24, 12, 0);
    data(24, 12, 0, NAN); run_control();
    assert(!SuperCap_Control.PWM_Running);

    // 切回充电模式后，功率目标和输入侧观测量不影响原 I_CAP 控制。
    Control_Mode = CONTROL_MODE_CHARGE;
    set_current = 1; set_power = NAN; set_voltage = 28;
    prepare(24, 12, NAN);
    run_control();
    assert(SuperCap_Control.PWM_Running);
    near(current_feedback, 0);
    assert(SuperCap_Control.PWM_Control.outputDuty > 0.5f);

    // 实际 CLA Task1 经 ADC 校准进入功率模式，CPU 完成 ISR 不参与闭环计算。
    Stop_PWM(); Control_Mode = CONTROL_MODE_POWER;
    set_power = 30; set_current = NAN;
    adc[0][1] = 2040; adc[2][3] = 2010; adc[2][1] = 1966;
    adc[1][0] = 2987; adc[1][1] = 1460;
    for (unsigned n = 0; n < 200; ++n) sample_tick();
    SuperCap_Control.Supercap_Enable_Flag = true;
    sample_tick(); sample_tick();
    assert(SuperCap_Control.PWM_Running);
    assert(capture_adc_frame()); run_cla_frame();
    near(current_feedback, 1);
    near(power_feedback, SuperCap_Control.Electronic_Data.VBUS * SuperCap_Control.Electronic_Data.I_IN);
    assert(Referee_Power_Loop.i10 != 0);
    Control_Mode = 2; run_control();
    assert(!SuperCap_Control.PWM_Running && Control_LastFault == 1);
    puts("通过：独立功率模式、I_CAP反馈、无效率前馈的双环跟踪、双向动态限流、电压接管、停波复位、异常停波与真实CLA采样路径");
}
