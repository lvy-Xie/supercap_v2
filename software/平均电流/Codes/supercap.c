#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "includes/supercap.h"
#include "includes/control.h"
#include "includes/config.h"
#include "SFO_V8.h"
#include "includes/cla_adc.h"
#include "includes/telemetry.h"

#pragma DATA_SECTION(SuperCap_Control, "ControlShared")
volatile SuperCap_Control_s SuperCap_Control;
// SFO V8 driverlib ABI，与 49C 和 TI hrpwm_ex1/9 例程一致。
int MEP_ScaleFactor;
volatile uint32_t ePWM[] = {0, PWM_BRIDGE_A_BASE, PWM_BRIDGE_B_BASE};
volatile uint16_t SFO_Status = SFO_INCOMPLETE;
volatile bool HRPWM_Ready;
static bool prepared;

void Supercap_ServiceHRPWM(void)
{
    ClaADC_CheckProgress();
    if (!CLA_Ready) Stop_PWM();
    // ERROR 锁存；后台 INCOMPLETE 时继续使用上一次有效标定值。
    if (SFO_Status != SFO_ERROR) {
        SFO_Status = (uint16_t)SFO();
    }
    if (SFO_Status == SFO_COMPLETE) HRPWM_Ready = true;
    if (SFO_Status == SFO_ERROR) {
        HRPWM_Ready = false;
        Stop_PWM();
    }
}
#pragma CODE_SECTION(Stop_PWM, ".TI.ramfunc")
#pragma CODE_SECTION(CLA1Task1_ISR, ".TI.ramfunc")

void Supercap_BackgroundPoll(void)
{
    Telemetry_Poll();
    if (CPUTimer_getTimerOverflowStatus(TIMER_BACKGROUND_1KHZ_BASE)) {
        CPUTimer_clearOverflowFlag(TIMER_BACKGROUND_1KHZ_BASE);
        Supercap_ServiceHRPWM();
    }
}

void Stop_PWM(void)
{
    // 先撤销启动请求，防止前台停止过程中被控制 ISR 抢占后再次释放 OST。
    SuperCap_Control.Supercap_Enable_Flag = false;
    prepared = false;
    EPWM_forceTripZoneEvent(PWM_BRIDGE_A_BASE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(PWM_BRIDGE_B_BASE, EPWM_TZ_FORCE_EVENT_OST);
    SuperCap_Control.PWM_Running = false;
}

__interrupt void CLA1Task1_ISR(void)
{
    if (CLA_getTaskOverflowFlag(CLA1_BASE, CLA_TASK_1)) {
        CLA_Fault = 2;
        CLA_Ready = false;
    }
    if (!CLA_Ready || !HRPWM_Ready || SFO_Status == SFO_ERROR || Control_StopRequest) {
        Stop_PWM();
    }
    if (SuperCap_Control.PWM_Running &&
        ((EPWM_getTripZoneFlagStatus(PWM_BRIDGE_A_BASE) |
          EPWM_getTripZoneFlagStatus(PWM_BRIDGE_B_BASE)) & EPWM_TZ_FLAG_OST)) {
        Stop_PWM();
    }
    if (!SuperCap_Control.Supercap_Enable_Flag) {
        Stop_PWM();
    } else if (!SuperCap_Control.PWM_Running) {
        // 先装入当前电压对应的占空比，下一次采样再释放输出。
        if (prepared && SuperCap_Control.Supercap_Enable_Flag) {
            // 仅启动这一瞬间屏蔽 Task1 新触发；若已有任务运行则下一轮再启动。
            // 确认 CLA 不会同时写入故障 OST 后，CPU 才释放输出。
            CLA_disableTasks(CLA1_BASE, CLA_TASKFLAG_1);
            if (!CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1) && !Control_StopRequest) {
                EPWM_clearTripZoneFlag(PWM_BRIDGE_A_BASE, EPWM_TZ_FLAG_OST);
                EPWM_clearTripZoneFlag(PWM_BRIDGE_B_BASE, EPWM_TZ_FLAG_OST);
                SuperCap_Control.PWM_Running = true;
            }
            CLA_enableTasks(CLA1_BASE, CLA_TASKFLAG_1);
        }
        prepared = SuperCap_Control.Supercap_Enable_Flag && !SuperCap_Control.PWM_Running;
    }
    Telemetry_Publish();
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

void Supercap_Init(void)
{
    uint32_t sfoCalls;
    HRPWM_Ready = false;
    SFO_Status = SFO_INCOMPLETE;
    // PWM/HRPWM/ADC 静态配置全部由 c2000.syscfg 生成。
    ASYSCTL_init();
    ADC_init();
    PWM_BRIDGE_A_init();
    PWM_BRIDGE_B_init();
    PWM_ADC_TRIGGER_init();
    Stop_PWM();
    Control_Init();
    if (!ClaADC_Init()) Stop_PWM();

    // 时基仍关闭、四个功率 GPIO 仍为低；完成首次校准后才允许启波。
    for (sfoCalls = 0; sfoCalls < HRPWM_SFO_MAX_STARTUP_CALLS &&
         SFO_Status == SFO_INCOMPLETE; ++sfoCalls) {
        Supercap_ServiceHRPWM();
    }
    if (!HRPWM_Ready) {
        SFO_Status = SFO_ERROR; // 超时同样锁存，禁止调试标志绕过校准。
        Stop_PWM();
    }

    ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptOverflowStatus(ADC_C_BASE, ADC_INT_NUMBER1);
    Interrupt_register(INT_CLA1_1, &CLA1Task1_ISR);
    Interrupt_enable(INT_CLA1_1);

    // 两桥已由 OST 拉低后，才切换四个功率引脚。
    GPIO_setPinConfig(PWM_BRIDGE_A_EPWMA_PIN_CONFIG);
    GPIO_setPinConfig(PWM_BRIDGE_A_EPWMB_PIN_CONFIG);
    GPIO_setPinConfig(PWM_BRIDGE_B_EPWMA_PIN_CONFIG);
    GPIO_setPinConfig(PWM_BRIDGE_B_EPWMB_PIN_CONFIG);
    AdcDma_Init(); // 先准备完整采集链，再打开 PWM 采样时基。
    ClaADC_ArmMonitor();
    // 发送前台不再延时；CLA/SFO 检查仍按 1 ms 节拍，不随轮询速度加快。
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TIMER1);
    TIMER_BACKGROUND_1KHZ_init();
    CPUTimer_clearOverflowFlag(TIMER_BACKGROUND_1KHZ_BASE);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}
