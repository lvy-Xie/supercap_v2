#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

static void near(float a, float b) { assert(fabsf(a - b) < 0.0001f); }
static void tick(void) { sample_tick(); }

int main(void)
{
    Supercap_Init();
    assert(!SuperCap_Control.Supercap_Enable_Flag && !SuperCap_Control.PWM_Running);
    assert(trip[0] && trip[1]);
    assert(signals[0] == 0 && signals[1] == 0);
    for (unsigned b = 0; b < 2; ++b) {
        assert(actions[b][EPWM_TZ_ACTION_EVENT_TZA] == EPWM_TZ_ACTION_LOW);
        assert(actions[b][EPWM_TZ_ACTION_EVENT_TZB] == EPWM_TZ_ACTION_LOW);
        for (unsigned e = 2; e < 6; ++e) assert(actions[b][e] == EPWM_TZ_ACTION_DISABLE);
    }
    // 两桥和触发器约 350 kHz，B 桥同步延迟补偿，七分频采样约 50 kHz。
    for (unsigned i = 0; i < 3; ++i) assert(period[i] == 285);
    assert(phase[1] == 145 && counter[1] == 143);
    assert(cmp[2] == 71 && cmpB[2] == 214);
    assert(prescale[0] == 7 && prescale[1] == 7);
    assert(100000000U / (PWM_COUNTS * prescale[1]) == 49950U);
    assert(HRPWM_Ready && SFO_Status == SFO_COMPLETE && sfo_calls == 3);

    // 扫过三种模式，检查电压比、脉宽和寄存器值。
    for (float ratio = 0.067f; ratio <= 10.0f; ratio += 0.003f) {
        run_duty(ratio);
        PWM_Control_s p = SuperCap_Control.PWM_Control;
        near(p.dutyA / p.dutyB, ratio);
        assert(p.dutyA >= MIN_PWM_DUTY && p.dutyA <= MAX_PWM_DUTY);
        assert(p.dutyB >= MIN_PWM_DUTY && p.dutyB <= MAX_PWM_DUTY);
        for (unsigned b = 0; b < 2; ++b) {
            assert(cmp[b] >= 28 && cmp[b] <= 268);
            assert(cmp_q8[b][0] == cmp_q8[b][1]);
            assert((cmp_q8[b][0] & 0xFFU) != 0);
            float duty = b == 0 ? p.dutyA : p.dutyB;
            assert(fabsf((1.0f - duty) * PWM_COUNTS - cmp_q8[b][0] / 256.0f) <= 1.0f / 256.0f);
        }
    }
    run_duty(0);
    near(SuperCap_Control.PWM_Control.outputDuty, MIN_OUTPUT_DUTY);
    run_duty(100);
    near(SuperCap_Control.PWM_Control.outputDuty, MAX_OUTPUT_DUTY);
    run_duty(1);
    assert(SuperCap_Control.PWM_Control.BuckBoostMode);
    run_duty(0.85f);
    assert(SuperCap_Control.PWM_Control.BuckBoostMode);
    run_duty(0.79f);
    assert(!SuperCap_Control.PWM_Control.BuckBoostMode);
    run_duty(0.85f);
    assert(!SuperCap_Control.PWM_Control.BuckBoostMode);
    run_duty(1);
    run_duty(1.26f);
    assert(!SuperCap_Control.PWM_Control.BuckBoostMode);
    const float invalid[] = {NAN, INFINITY, -INFINITY, -1};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) {
        SuperCap_Control.Supercap_Enable_Flag = true;
        run_duty(invalid[i]);
        assert(!SuperCap_Control.Supercap_Enable_Flag);
    }

    adc[0][0] = 2000; adc[2][0] = 2050; adc[0][1] = 2100;
    adc[2][1] = 2200; adc[2][3] = 2300;
    adc[1][0] = 2987; adc[1][1] = 1241;
    tick();
    // 停波时也更新具名原始码；电压原始码不经过低通滤波。
    assert(SuperCap_Control.Electronic_Data.I_A_Raw == 2000);
    assert(SuperCap_Control.Electronic_Data.I_B_Raw == 2050);
    assert(SuperCap_Control.Electronic_Data.I_IN_Raw == 2100);
    assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 2200);
    assert(SuperCap_Control.Electronic_Data.I_OUT_Raw == 2300);
    assert(SuperCap_Control.Electronic_Data.VBUS_Raw == 2987);
    assert(SuperCap_Control.Electronic_Data.VCAP_Raw == 1241);
    assert(SuperCap_Control.Electronic_Data.VBUS < 2987 * 0.008f);
    const uint16_t expected_raw[] = {2000, 2050, 2100, 2200, 2300, 2987, 1241};
    for (unsigned i = 0; i < 7; ++i) {
        assert(SuperCap_Control.Electronic_Data.Raw[i] == expected_raw[i]);
    }
    for (int i = 0; i < 200; ++i) tick();
    near(SuperCap_Control.Electronic_Data.I_A, 2000 * 0.020146520f - 41.25f);
    near(SuperCap_Control.Electronic_Data.I_B, -2050 * 0.020146520f + 41.25f);
    near(SuperCap_Control.Electronic_Data.I_IN, 2100 * 2.047242038e-02f - 4.110068512e+01f);
    near(SuperCap_Control.Electronic_Data.I_CAP, -2200 * 2.014652081e-02f + 4.060805893e+01f);
    near(SuperCap_Control.Electronic_Data.I_OUT, 2300 * 2.034800500e-02f - 4.084916306e+01f);
    near(SuperCap_Control.Electronic_Data.VBUS, 2987 * 8.047959767e-03f - 3.887796402e-02f);
    near(SuperCap_Control.Electronic_Data.VCAP, 1241 * 8.221379481e-03f + 5.106547382e-03f);
    // 用户本次实测点必须经真实采样/换算路径得到 1 A。
    adc[2][1] = 1966;
    tick();
    assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 1966);
    near(SuperCap_Control.Electronic_Data.I_CAP, 1.0f);
    adc[2][1] = 2200;
    tick();

    // 取消一次准备启动，再启动仍必须等两次中断。
    SuperCap_Control.Supercap_Enable_Flag = true; tick();
    assert(!SuperCap_Control.PWM_Running);
    SuperCap_Control.Supercap_Enable_Flag = false; tick();
    SuperCap_Control.Supercap_Enable_Flag = true; tick();
    assert(!SuperCap_Control.PWM_Running);
    tick();
    assert(SuperCap_Control.PWM_Running && !trip[0] && !trip[1]);
    tick();
    assert(SuperCap_Control.PWM_Control.outputDuty > 0);
    // 采样超过原 29 V 门限不再触发过压停机；手动停波仍有效。
    adc[1][1] = 3723;
    for (unsigned i = 0; i < 200; ++i) tick();
    assert(SuperCap_Control.Electronic_Data.VCAP > 29.0f);
    assert(SuperCap_Control.PWM_Running && !trip[0] && !trip[1]);
    SuperCap_Control.Supercap_Enable_Flag = false; tick();
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    adc[1][1] = 1241;
    for (unsigned i = 0; i < 200; ++i) tick();
    SuperCap_Control.Supercap_Enable_Flag = true; tick(); tick();
    trip[1] |= EPWM_TZ_FLAG_OST; tick();
    assert(!SuperCap_Control.Supercap_Enable_Flag && trip[0]);
    SuperCap_Control.Supercap_Enable_Flag = true; tick(); tick();
    overflow = true; tick();
    assert(!SuperCap_Control.PWM_Running);
    assert(!CLA_Ready && AdcDma_Fault == 2);
    Supercap_Init(); // 采集链故障锁存；后续参数测试从重新初始化开始。
    for (unsigned i = 0; i < 200; ++i) tick();
    set_voltage = NAN;
    SuperCap_Control.Supercap_Enable_Flag = true; tick(); tick();
    assert(!SuperCap_Control.PWM_Running);
    set_voltage = 29;
    SuperCap_Control.Supercap_Enable_Flag = true; tick();
    assert(!SuperCap_Control.Supercap_Enable_Flag);
    set_voltage = 16;
    SuperCap_Control.Electronic_Data.VBUS = 0;
    run_control();
    assert(!SuperCap_Control.PWM_Running);
    // 使用 TI 原版 DCL，检查停机重置及饱和恢复。
    SuperCap_Control.Electronic_Data.VBUS = 24;
    SuperCap_Control.Electronic_Data.VCAP = 10;
    run_control();
    near(Voltage_loop.i10, 10);
    SuperCap_Control.Supercap_Enable_Flag = true;
    SuperCap_Control.PWM_Running = true;
    set_voltage = 28;
    for (int i = 0; i < 10000; ++i) run_control();
    assert(Voltage_loop.i10 < 29);
    set_voltage = 5;
    for (int i = 0; i < 10000; ++i) run_control();
    assert(SuperCap_Control.PWM_Control.outputDuty <= MIN_OUTPUT_DUTY);
    puts("通过：350kHz时基、HRPWM小数精度、半周期相移、约50kHz采样、三工作区占空比分配、校准、启动取消、手动停波、异常参数、采样溢出和电压限幅");
}
