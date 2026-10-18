#include <assert.h>
#include <stdio.h>
#include "control_runtime.h"

static void seed(uint16_t first)
{
    for (unsigned module = 0; module < 3; ++module)
        for (unsigned result = 0; result < 4; ++result)
            adc[module][result] = first + module * 100 + result;
}

static void check_fault(uint16_t reason)
{
    assert(AdcDma_Fault == reason && CLA_Fault == 4 && !CLA_Ready);
    assert(!dma.running && !dma.triggered && !cla_task_pending);
    assert(!SuperCap_Control.Supercap_Enable_Flag && !SuperCap_Control.PWM_Running);
    assert(trip[0] && trip[1]);
}

int main(void)
{
    Supercap_Init();
    assert(dma.running && AdcDma_Frames == 0 && AdcDma_Fault == 0);
    assert(AdcDma_Frame.valid == 0 && Cla_AdcData.sequence == 0);
    seed(1000);
    // DMA 尚未完成，CLA/消息不能提前看到下一组 ADC 数据。
    assert(dma_capture_hardware());
    assert(!dma.running && !cla_task_pending && AdcDma_Frame.valid == 0);
    assert(Cla_AdcData.sequence == 0 && SuperCap_Control.Electronic_Data.I_CAP_Raw == 0);
    for (unsigned module = 0; module < 3; ++module)
        for (unsigned result = 0; result < 4; ++result)
            assert(AdcDma_Buffer[module * 4 + result] == 1000 + module * 100 + result);
    AdcDma_ISR();
    assert(cla_task_pending && dma.running && AdcDma_Frames == 1);
    const uint16_t expected[] = {1000, 1200, 1001, 1201, 1203, 1100, 1101};
    for (unsigned n = 0; n < 7; ++n) assert(AdcDma_Frame.raw[n] == expected[n]);
    seed(2000); // CLA 应使用已经提交的旧帧，不能直接再读 ADC_RESULT。
    run_cla_frame(); CLA1Task1_ISR();
    assert(Cla_AdcData.sequence == 1);
    for (unsigned n = 0; n < 7; ++n) assert(Cla_AdcData.raw[n] == expected[n]);
    assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 1201);
    // 多帧重新 RUN 必须从 shadow 起始地址搬运，包含上电停波和新样本更新。
    for (unsigned n = 0; n < 100; ++n) {
        seed(2000 + n);
        sample_tick();
        assert(AdcDma_Frames == n + 2 && Cla_AdcData.sequence == n + 2);
        assert(SuperCap_Control.Electronic_Data.I_CAP_Raw == 2201 + n);
    }
    // 16 位序号回卷也必须被识别为新帧。
    AdcDma_Frame.sequence = Cla_AdcData.sequence = 65535;
    sample_tick();
    assert(AdcDma_Frame.sequence == 0 && Cla_AdcData.sequence == 0);
    uint32_t compare = cmp_q8[0][0];
    Cla1Task1(); // 重复/无新帧的软件触发不能再次执行控制。
    assert(Control_StopRequest == 5 && cmp_q8[0][0] == compare);
    CLA1Task1_ISR();

    // 上一帧尚未被 CLA 消费，下一帧不允许覆盖消息。
    Supercap_Init(); seed(1000);
    assert(capture_adc_frame());
    uint16_t old = AdcDma_Frame.raw[0];
    seed(2000);
    assert(dma_capture_hardware());
    cla_task_pending = false; // 模拟 pending/MRUN 尚未显现，也要由序号发现未消费。
    AdcDma_ISR(); check_fault(3);
    assert(AdcDma_Frame.raw[0] == old && AdcDma_Frames == 1);

    Supercap_Init(); seed(1000);
    assert(dma_capture_hardware());
    cla_task_running = true;
    AdcDma_ISR(); check_fault(3);
    cla_task_running = false;

    Supercap_Init(); seed(1000);
    assert(dma_capture_hardware());
    dma.overflow = true;
    AdcDma_ISR(); check_fault(1);

    Supercap_Init(); seed(1000);
    assert(dma_capture_hardware());
    overflow = true;
    AdcDma_ISR(); check_fault(2);
    assert(!overflow);

    Supercap_Init();
    dma.active = true; // 非 EOT 的提前中断不能发布半帧。
    AdcDma_ISR(); check_fault(4);
    assert(AdcDma_Frames == 0 && AdcDma_Frame.valid == 0);
    puts("通过：SysConfig生成DMA跨ADC步长、整帧屏障、七路映射、CLA只读快照、连续重新装载、序号回卷和过期帧保护");
    puts("通过：DMA/ADC溢出、CLA忙或未消费、DMA提前中断均禁止覆盖并锁存停波");
}
