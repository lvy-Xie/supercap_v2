#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

static void start(void)
{
    SuperCap_Control.Supercap_Enable_Flag = true;
    sample_tick(); sample_tick();
}

int main(void)
{
    // 复现旧启动顺序：先划成 CLA 程序 RAM，CPU 复制被阻止，Task 8 无法初始化。
    MEMCFG_init();
    CLA_init();
    CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_8);
    assert(cla_copy_blocked && !cla_program_loaded);
    assert(Cla_AdcData.initialized != CLA_ADC_INIT_MAGIC);
    // 同时覆盖 Restart：进入初始化时 LS0 仍归 CLA，也必须能重新装载。
    Supercap_Init();
    assert(!cla_copy_blocked && cla_program_loaded && cla_program_memory);
    assert(CLA_Ready && CLA_Fault == 0);
    assert(Cla_AdcData.initialized == CLA_ADC_INIT_MAGIC);
    assert(Cla_AdcData.sequence == 0);
    adc[2][1] = 1966;
    adc[1][1] = 1862;
    adc[1][0] = 2987;
    sample_tick();
    assert(Cla_AdcData.sequence == 1);
    assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 1966);
    assert(fabsf(SuperCap_Control.Electronic_Data.I_CAP - 1.0f) < 0.00001f);
    assert(SuperCap_Control.Electronic_Data.VCAP > 0);
    assert(SuperCap_Control.Electronic_Data.VCAP < 15);
    assert(!SuperCap_Control.Supercap_Enable_Flag);
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    // 停波状态下继续采样，无需写 使能为 1 才能看到数据。
    adc[2][1] = 1970;
    sample_tick();
    assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 1970);
    assert(Cla_AdcData.sequence == 2);
    start();
    assert(SuperCap_Control.PWM_Running);
    // CLA 任务溢出必须锁存故障并关断两桥。
    cla_task_overflow = true;
    sample_tick();
    assert(!CLA_Ready && CLA_Fault == 2);
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);

    Supercap_Init();
    start();
    assert(SuperCap_Control.PWM_Running);
    Supercap_ServiceHRPWM(); // 记录本轮序号。
    for (unsigned n = 0; n < 5; ++n) Supercap_ServiceHRPWM();
    assert(!CLA_Ready && CLA_Fault == 3);
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    start();
    assert(!SuperCap_Control.PWM_Running); // 不可由 使能标志绕过。

    cla_init_timeout = true;
    Supercap_Init();
    assert(!CLA_Ready && CLA_Fault == 1);
    start();
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    puts("通过：复现CLA复制受阻、修正装载顺序、Restart重装、停波持续采样、校准/滤波和故障互锁");
}
