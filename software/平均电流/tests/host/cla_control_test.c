#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "DCLF32.h"
#include "control_runtime.h"

static void near(float a, float b) { assert(fabsf(a - b) < 0.00002f); }

int main(void)
{
    // CLA 适配的 PID 与原 CPU DCL C3 方程一致，覆盖 D 项/外部限幅/饱和。
    DCL_PID reference = PID_DEFAULTS;
    DCL_CSS referenceCss = DCL_CSS_DEFAULTS;
    reference.css = &referenceCss;
    DCL_PID_CLA moved;
    reference.Kp = 0.1f; reference.Ki = 0.005f; reference.Kd = 1.5f;
    reference.c1 = 0.7f; reference.c2 = 0.3f;
    reference.Umin = -2; reference.Umax = 2;
    moved.Kp = reference.Kp; moved.Ki = reference.Ki; moved.Kd = reference.Kd;
    moved.c1 = reference.c1; moved.c2 = reference.c2;
    moved.Umin = reference.Umin; moved.Umax = reference.Umax;
    Cla_ResetPID(&moved);
    for (unsigned n = 0; n < 10000; ++n) {
        float target = n < 5000 ? 6.0f : -1.0f;
        float feedback = (float)(n % 103) * 0.03f;
        float allow = n % 19 ? 1.0f : 0.0f;
        near(Cla_RunPID(&moved, target, feedback, allow),
             DCL_runPID_C3(&reference, target, feedback, allow));
        near(moved.i10, reference.i10); near(moved.d2, reference.d2);
        near(moved.d3, reference.d3); near(moved.i14, reference.i14);
    }
    DCL_PI oldPI = PI_DEFAULTS;
    oldPI.css = &referenceCss;
    DCL_PI_CLA movedPI = PI_CLA_DEFAULTS;
    oldPI.Kp = movedPI.Kp = 0.0085f;
    oldPI.Ki = movedPI.Ki = 0.0005f;
    oldPI.Umin = movedPI.Umin = MIN_OUTPUT_DUTY;
    oldPI.Umax = movedPI.Umax = MAX_OUTPUT_DUTY;
    for (unsigned n = 0; n < 10000; ++n) {
        float target = n < 5000 ? 15.0f : 0.0f;
        float feedback = (float)(n % 71) * 0.1f;
        near(DCL_runPI_L4(&movedPI, target, feedback), DCL_runPI_C3(&oldPI, target, feedback));
        near(movedPI.i10, oldPI.i10); near(movedPI.i6, oldPI.i6);
    }
    // 真实 CLA Task1 路径：不执行 CPU 完成 ISR 也必须完成电流 PI 和 HRPWM 写入。
    Supercap_Init();
    adc[0][1] = 2010; adc[2][3] = 2010;
    adc[2][1] = 2010;
    adc[1][0] = 2987; adc[1][1] = 1460;
    for (unsigned n = 0; n < 200; ++n) sample_tick();
    SuperCap_Control.Supercap_Enable_Flag = 1;
    sample_tick();
    cla_task_running = true; // 启动时若新 CLA 任务仍忙，CPU 不允许释放输出。
    CLA1Task1_ISR();
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    assert(cla_task_enabled);
    cla_task_running = false;
    sample_tick();
    assert(SuperCap_Control.PWM_Running);
    float integral = CurrentA_loop.i10;
    uint32_t oldCompare = cmp_q8[0][0];
    assert(capture_adc_frame());
    run_cla_frame(); // 特意不调用 CPU 完成 ISR；原始码由 DMA 链提交。
    assert(CurrentA_loop.i10 != integral && cmp_q8[0][0] != oldCompare);
    integral = CurrentA_loop.i10;
    uint32_t compare = cmp_q8[0][0];
    CLA1Task1_ISR();
    assert(CurrentA_loop.i10 == integral && cmp_q8[0][0] == compare);
    // 控制参数异常由 CLA 立即 OST，CPU 之后仅确认停波；不能重新释放。
    set_current = NAN;
    assert(capture_adc_frame()); run_cla_frame();
    assert(Control_StopRequest == 1 && Control_LastFault == 1);
    assert(trip[0] && trip[1]);
    assert(SuperCap_Control.PWM_Running); // CPU 尚未确认。
    CLA1Task1_ISR();
    assert(!SuperCap_Control.PWM_Running && !SuperCap_Control.Supercap_Enable_Flag);
    set_current = 1;
    sample_tick();
    assert(Control_StopRequest == 0 && Control_LastFault == 1);
    assert(trip[0] && trip[1]);
    puts("通过：CLA PI/PID与原DCL对照、独立完成HRPWM闭环、CPU不再算控制、任务忙禁止启波、CLA故障立即停波与CPU确认");
}
