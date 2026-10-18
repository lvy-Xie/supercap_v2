#include <assert.h>
#include <stdio.h>
#include "control_runtime.h"

static void request_start(void)
{
    SuperCap_Control.Supercap_Enable_Flag = true;
    sample_tick();
    sample_tick();
}

int main(void)
{
    Supercap_Init();
    adc[1][0] = 2987;
    adc[1][1] = 1460;
    assert(HRPWM_Ready && SFO_Status == SFO_COMPLETE && sfo_calls == 3);
    for (unsigned b = 0; b < 2; ++b) {
        assert(auto_conversion[b]);
        assert(hr_edge[b][0] == HRPWM_MEP_CTRL_RISING_EDGE);
        assert(hr_edge[b][1] == HRPWM_MEP_CTRL_FALLING_EDGE);
        assert(hr_load[b][0] == HRPWM_LOAD_ON_CNTR_ZERO);
        assert(hr_load[b][1] == HRPWM_LOAD_ON_CNTR_ZERO);
        assert(coarse_load[b][0] == EPWM_COMP_LOAD_ON_CNTR_ZERO);
        assert(coarse_load[b][1] == EPWM_COMP_LOAD_ON_CNTR_ZERO);
        assert(db_red[b] == 5 && db_fed[b] == 5);
    }
    // 小于一个 TBCLK 的占空比变化必须写入 HR 部分。
    run_duty(0.5f);
    uint16_t coarse = cmp[0];
    uint32_t fine = cmp_q8[0][0];
    run_duty(0.5001f);
    assert(cmp[0] == coarse && cmp_q8[0][0] != fine);
    // 整数比较点也遵循 TI 例程的 CMPxHR 非零约定。
    run_duty(1.0f);
    run_duty(2.0f); // B 占空比 0.45，A 占空比 0.90。
    assert(cmp_q8[0][0] == cmp_q8[0][1]);
    request_start();
    assert(SuperCap_Control.PWM_Running);
    // 后台新一轮未完成时沿用已标定的 MEP，不能停波。
    sfo_result = SFO_INCOMPLETE;
    Supercap_ServiceHRPWM();
    sample_tick();
    assert(HRPWM_Ready && SuperCap_Control.PWM_Running);
    sfo_result = SFO_ERROR;
    Supercap_ServiceHRPWM();
    assert(!HRPWM_Ready && trip[0] && trip[1]);
    request_start();
    assert(!SuperCap_Control.PWM_Running);
    unsigned calls = sfo_calls;
    sfo_result = SFO_COMPLETE;
    Supercap_ServiceHRPWM();
    assert(sfo_calls == calls && SFO_Status == SFO_ERROR);

    // 首次校准失败以及永不完成均保持停波，不能由调试标志绕过。
    sfo_result = SFO_ERROR;
    Supercap_Init();
    request_start();
    assert(!HRPWM_Ready && !SuperCap_Control.PWM_Running);
    sfo_result = SFO_INCOMPLETE;
    calls = sfo_calls;
    Supercap_Init();
    assert(sfo_calls - calls == HRPWM_SFO_MAX_STARTUP_CALLS);
    assert(SFO_Status == SFO_ERROR && !HRPWM_Ready);
    request_start();
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    puts("通过：HRPWM边沿与死区、亚时钟精度、SFO首次/后台校准、失败锁存、超时和启波互锁");
}
