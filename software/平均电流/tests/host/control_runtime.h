#ifndef HOST_CONTROL_RUNTIME_H
#define HOST_CONTROL_RUNTIME_H
// 使用同一份 CLA 算法，在主机以 C 执行；仅寄存器和触发机制使用测试替身。
#include "Codes/cla_tasks.cla"
void host_memcfg_init(void)
{
    Cla_AdcData.initialized = 0;
    AdcDma_Frame.valid = AdcDma_Frame.sequence = 0;
    cla_task_overflow = false;
    cla_program_memory = true;
}
#include "Codes/cla_adc.c"
#include "Codes/telemetry.c"
#include "Codes/control.c"
#include "Codes/supercap.c"

static inline bool capture_adc_frame(void)
{
    if (!dma_capture_hardware()) return false;
    AdcDma_ISR();
    return cla_task_pending;
}

static inline void run_cla_frame(void)
{
    assert(cla_task_enabled && cla_task_pending);
    cla_task_pending = false;
    cla_task_running = true;
    Cla1Task1();
    cla_task_running = false;
}

static inline void sample_tick(void)
{
    if (!capture_adc_frame()) return;
    run_cla_frame();
    CLA1Task1_ISR();
}

// 单独测试 CLA 控制算法时，随后模拟 CPU 对停波请求的确认。
static inline void run_control(void)
{
    Control_Loop();
    if (Control_StopRequest) {
        Stop_PWM();
        Control_StopRequest = 0;
    }
}
static inline void run_duty(float duty)
{
    Update_PWM_Duty(duty);
    if (Control_StopRequest) { Stop_PWM(); Control_StopRequest = 0; }
}
#endif
