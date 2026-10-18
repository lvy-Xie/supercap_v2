#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

static void near(float a, float b) { assert(fabsf(a - b) < 0.0001f); }

static void data(float vbus, float vcap, float current)
{
    SuperCap_Control.Electronic_Data.VBUS = vbus;
    SuperCap_Control.Electronic_Data.VCAP = vcap;
    SuperCap_Control.Electronic_Data.I_CAP = current;
}

// 简化电流一阶对象验证闭环方向、跟踪和恢复，不代表实板稳定性。
static float settle(float target, float initial)
{
    float measured = initial;
    set_current = target;
    for (unsigned n = 0; n < 10000; ++n) {
        data(24, 12, measured);
        run_control();
        float duty = SuperCap_Control.PWM_Control.outputDuty;
        measured += 0.04f * (60.0f * (duty - 0.5f) - measured);
        assert(isfinite(measured) && fabsf(measured) < 5.0f);
    }
    return measured;
}

int main(void)
{
    Supercap_Init();
    near(CurrentA_loop.Kp, 0.0085f); near(CurrentA_loop.Ki, 0.0005f);
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    // 低于原 90% 阈值时，启动第一拍也运行电压 PID，并按两路输出选小。
    set_voltage = 28; set_current = 1;
    data(24, 12, 0); run_control();
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
    run_control();
    near(pid_v_out, Voltage_loop.Umax);
    assert(Voltage_loop.d2 != 0.0f && !Current_VoltageLimited);
    near(SuperCap_Control.PWM_Control.outputDuty, pid_i_out);
    Stop_PWM(); run_control();
    CurrentA_loop.i10 = 2.0f;
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
    run_control();
    assert(Current_VoltageLimited);
    near(SuperCap_Control.PWM_Control.outputDuty, pid_v_out / 24.0f);
    near(CurrentA_loop.i10 + CurrentA_loop.Kp * set_current,
         SuperCap_Control.PWM_Control.outputDuty);
    Supercap_Init();
    // CLA 校准数据 -> I_CAP -> 电流环；I_DCDC 仍作为独立观测量计算。
    adc[0][1] = 2100; adc[2][3] = 2040;
    adc[2][1] = 1966;
    adc[1][0] = 2987; adc[1][1] = 1460;
    for (unsigned n = 0; n < 200; ++n) sample_tick();
    near(SuperCap_Control.Electronic_Data.I_DCDC,
         SuperCap_Control.Electronic_Data.I_IN - SuperCap_Control.Electronic_Data.I_OUT);
    near(current_feedback, SuperCap_Control.Electronic_Data.I_CAP);
    near(SuperCap_Control.Electronic_Data.I_CAP, 1.0f);
    assert(fabsf(current_feedback - SuperCap_Control.Electronic_Data.I_DCDC) > 0.1f);
    adc[2][1] = 1956; sample_tick();
    near(current_feedback, 1.0f + 10.0f * 0.02014652081f);
    SuperCap_Control.Supercap_Enable_Flag = true;
    sample_tick(); assert(!SuperCap_Control.PWM_Running);
    sample_tick(); assert(SuperCap_Control.PWM_Running);
    // 停波预置前馈；第一拍误差为正提升占空比，为负则降低。
    Stop_PWM(); data(24, 12, 0); run_control();
    near(CurrentA_loop.i10, 0.5f);
    CurrentA_loop.i6 = 0;
    run_control(); near(CurrentA_loop.i6, 1);
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
    run_control(); assert(SuperCap_Control.PWM_Control.outputDuty > 0.5f);
    float result = settle(1, 0);
    assert(fabsf(result - 1.0f) < 0.001f && !Current_VoltageLimited);
    result = settle(0.3f, result);
    assert(fabsf(result - 0.3f) < 0.001f);
    result = settle(0, result);
    assert(fabsf(result) < 0.001f);
    // 电压已高于设定时电压环接管，未选中的 PI 不继续积累。
    Stop_PWM(); set_voltage = 16; set_current = 1;
    data(24, 17, 0); run_control();
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
    for (unsigned n = 0; n < 1000; ++n) run_control();
    assert(Current_VoltageLimited);
    assert(SuperCap_Control.PWM_Control.outputDuty <= pid_i_out);
    near(CurrentA_loop.i10 + CurrentA_loop.Kp * (set_current - current_feedback),
         SuperCap_Control.PWM_Control.outputDuty);
    result = settle(1, 0);
    assert(fabsf(result - 1.0f) < 0.001f && !Current_VoltageLimited);
    // 长时间不可达目标受电压环限幅且不会无限累加，反向误差后能到达下限。
    set_current = MAX_CHARGE_CURRENT;
    data(24, 12, -50);
    for (unsigned n = 0; n < 10000; ++n) run_control();
    near(SuperCap_Control.PWM_Control.outputDuty, Voltage_loop.Umax / 24.0f);
    assert(isfinite(CurrentA_loop.i10) && CurrentA_loop.i10 < MAX_OUTPUT_DUTY + 1);
    set_current = 0; data(24, 12, 50);
    for (unsigned n = 0; n < 10000; ++n) run_control();
    near(SuperCap_Control.PWM_Control.outputDuty, MIN_OUTPUT_DUTY);
    assert(isfinite(CurrentA_loop.i10));
    // I_DCDC 不参与电流 PI；即使很大或无效，也不能改变 I_CAP 反馈。
    SuperCap_Control.Electronic_Data.I_DCDC = 100.0f;
    data(24, 12, 0); run_control();
    near(current_feedback, 0);
    SuperCap_Control.Electronic_Data.I_DCDC = NAN;
    run_control();
    near(current_feedback, 0);
    assert(SuperCap_Control.PWM_Running);
    const float invalid[] = {NAN, INFINITY, -1, MAX_CHARGE_CURRENT + 1};
    for (unsigned n = 0; n < sizeof(invalid) / sizeof(invalid[0]); ++n) {
        SuperCap_Control.Supercap_Enable_Flag = true;
        SuperCap_Control.PWM_Running = true;
        set_current = invalid[n];
        run_control();
        assert(!SuperCap_Control.Supercap_Enable_Flag && trip[0] && trip[1]);
        near(CurrentA_loop.i10, 0); near(CurrentA_loop.i6, 1);
    }
    set_current = 1;
    data(24, 12, NAN); run_control();
    assert(!SuperCap_Control.PWM_Running);
    data(0, 12, 0); run_control();
    assert(!SuperCap_Control.PWM_Running);
    data(24, 12, 0); CurrentA_loop.Ki = NAN; run_control();
    assert(!SuperCap_Control.PWM_Running);
    CurrentA_loop.Ki = 0.0005f;
    puts("通过：启动首拍双环选小、低电压接管、I_CAP校准反馈、I_DCDC独立观测、单一使能、双采样启波、前馈复位、电流阶跃、饱和恢复、CC/CV接管和异常停波");
}
