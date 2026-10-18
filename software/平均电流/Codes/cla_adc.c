#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "includes/cla_adc.h"
#include "includes/supercap.h"
#include "includes/control.h"

// 共享对象由 CPU 定义；仅 CLA 初始化/写入这个方向的消息 RAM。
#pragma DATA_SECTION(Cla_AdcData, "Cla1ToCpuMsgRAM")
volatile Cla_AdcData_s Cla_AdcData;
#pragma DATA_SECTION(AdcDma_Frame, "CpuToCla1MsgRAM")
volatile AdcDma_Frame_s AdcDma_Frame;
#pragma DATA_SECTION(AdcDma_Buffer, "AdcDmaBuffer")
volatile uint16_t AdcDma_Buffer[12];
const void *AdcDma_Destination = (const void *)AdcDma_Buffer;
volatile uint16_t AdcDma_Frames, AdcDma_Fault;

volatile bool CLA_Ready;
volatile uint16_t CLA_Fault;
static bool monitorArmed;
static uint16_t lastSequence, stalledCalls;

#pragma CODE_SECTION(AdcDma_ISR, ".TI.ramfunc")
#pragma CODE_SECTION(AdcDma_Fail, ".TI.ramfunc")
static void AdcDma_Fail(uint16_t reason)
{
    AdcDma_Fault = reason;
    CLA_Fault = 4;
    CLA_Ready = false;
    Control_LastFault = 5;
    DMA_disableTrigger(DMA_ADC_CAPTURE_BASE);
    DMA_stopChannel(DMA_ADC_CAPTURE_BASE);
    Stop_PWM();
    DMA_clearErrorFlag(DMA_ADC_CAPTURE_BASE);
    DMA_clearTriggerFlag(DMA_ADC_CAPTURE_BASE);
    ADC_clearInterruptOverflowStatus(ADC_C_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
}

void AdcDma_Init(void)
{
    uint16_t i;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_DMA);
    DMA_init(); // 单通道跨 A/B/C 结果区的几何、触发和模式由 SysConfig 生成。
    for (i = 0; i < 12U; ++i) AdcDma_Buffer[i] = 0;
    for (i = 0; i < 7U; ++i) AdcDma_Frame.raw[i] = 0;
    AdcDma_Frame.valid = AdcDma_Frame.sequence = 0;
    AdcDma_Frames = AdcDma_Fault = 0;
    ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptOverflowStatus(ADC_C_BASE, ADC_INT_NUMBER1);
    DMA_clearErrorFlag(DMA_ADC_CAPTURE_BASE);
    DMA_clearTriggerFlag(DMA_ADC_CAPTURE_BASE);
    Interrupt_register(INT_DMA_CH1, &AdcDma_ISR);
    Interrupt_enable(INT_DMA_CH1);
    if (CLA_Ready) DMA_startChannel(DMA_ADC_CAPTURE_BASE);
}

__interrupt void AdcDma_ISR(void)
{
    if (!CLA_Ready || AdcDma_Fault) {
        DMA_stopChannel(DMA_ADC_CAPTURE_BASE);
        Stop_PWM();
    } else if (DMA_getOverflowFlag(DMA_ADC_CAPTURE_BASE)) {
        AdcDma_Fail(1U);
    } else if (DMA_getTransferStatusFlag(DMA_ADC_CAPTURE_BASE) ||
               DMA_getRunStatusFlag(DMA_ADC_CAPTURE_BASE)) {
        AdcDma_Fail(4U); // 只能在三个 burst 全部完成后提交，不能把半帧交给 CLA。
    } else if (ADC_getInterruptOverflowStatus(ADC_C_BASE, ADC_INT_NUMBER1)) {
        AdcDma_Fail(2U);
    } else if (CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1) ||
               AdcDma_Frame.sequence != Cla_AdcData.sequence) {
        AdcDma_Fail(3U); // CLA 正在读取或上一帧尚未消费，禁止覆盖消息。
    } else {
        // GS0 为 A0..3、B0..3、C0..3；只提交七路有效控制通道。
        AdcDma_Frame.raw[0] = AdcDma_Buffer[0];
        AdcDma_Frame.raw[1] = AdcDma_Buffer[8];
        AdcDma_Frame.raw[2] = AdcDma_Buffer[1];
        AdcDma_Frame.raw[3] = AdcDma_Buffer[9];
        AdcDma_Frame.raw[4] = AdcDma_Buffer[11];
        AdcDma_Frame.raw[5] = AdcDma_Buffer[4];
        AdcDma_Frame.raw[6] = AdcDma_Buffer[5];
        AdcDma_Frame.valid = ADC_DMA_FRAME_MAGIC;
        ++AdcDma_Frame.sequence; // 最后发布，CLA 完成后将相同序号写回。
        ++AdcDma_Frames;
        ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
        DMA_clearTriggerFlag(DMA_ADC_CAPTURE_BASE);
        // 非连续模式完成后自动停下，消息复制期间 GS0 不会被下一帧覆盖。
        // 再次 RUN 会从硬件 shadow 重新装载相同的起始地址和计数。
        DMA_startChannel(DMA_ADC_CAPTURE_BASE);
        CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_1);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP7);
}

bool ClaADC_Init(void)
{
    uint32_t attempts;
    CLA_Ready = false;
    CLA_Fault = 0;
    monitorArmed = false;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_DMA);
    DMA_stopChannel(DMA_ADC_CAPTURE_BASE); // Restart 装载 CLA 前先停止旧采集链。
    // 调试器 Restart 不一定复位 MemCfg；复制前先停止 CLA 并交回程序 RAM。
    // LS0 一旦成为 CLA 程序 RAM，CPU 读写都会被硬件阻止。
    CLA_performHardReset(CLA1_BASE);
    MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS0, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
    MemCfg_setLSRAMControllerSel(MEMCFG_SECT_LS1, MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
    // 必须先复制，再交给 CLA。最终 RAM 所有权、消息 RAM、任务配置仍由 SysConfig 生成。
    CLA_init();
    MEMCFG_init();
    CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_8);
    for (attempts = 0; attempts < 100000UL; ++attempts) {
        if (Cla_AdcData.initialized == CLA_ADC_INIT_MAGIC) {
            CLA_Ready = true;
            return true;
        }
    }
    CLA_Fault = 1;
    return false;
}

void ClaADC_ArmMonitor(void)
{
    lastSequence = Cla_AdcData.sequence;
    stalledCalls = 0;
    monitorArmed = CLA_Ready;
}

void ClaADC_CheckProgress(void)
{
    uint16_t current = Cla_AdcData.sequence;
    if (!monitorArmed || !CLA_Ready) return;
    if (CLA_getTaskOverflowFlag(CLA1_BASE, CLA_TASK_1)) {
        CLA_Fault = 2;
        CLA_Ready = false;
    } else if (current != lastSequence) {
        lastSequence = current;
        stalledCalls = 0;
    } else if (++stalledCalls >= 5U) {
        CLA_Fault = 3;
        CLA_Ready = false;
    }
}
