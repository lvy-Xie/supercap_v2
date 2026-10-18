#include <assert.h>
#include <stdio.h>
#include "control_runtime.h"

int main(void)
{
    Supercap_Init();
    Telemetry_Init();
    assert(background_timer_running && background_period == 99999);
    unsigned calls = sfo_calls;
    // 快速串口轮询不应把五次停采检查从 5 ms 缩短成几微秒。
    for (unsigned n = 0; n < 100; ++n) Supercap_BackgroundPoll();
    assert(sfo_calls == calls && CLA_Ready);
    adc[1][0] = 2987;
    adc[1][1] = 1862;
    sample_tick();
    Supercap_BackgroundPoll();
    assert(sci_byte_count == 16 && sfo_calls == calls);
    sci_fifo_used = 0;
    Supercap_BackgroundPoll();
    assert(sci_byte_count == TELEMETRY_FRAME_OCTETS && Telemetry_FramesQueued == 1);
    for (unsigned n = 0; n < 6; ++n) {
        sample_tick();
        background_overflow = true;
        Supercap_BackgroundPoll();
        assert(!background_overflow && sfo_calls == ++calls);
        for (unsigned p = 0; p < 100; ++p) Supercap_BackgroundPoll();
        assert(sfo_calls == calls && CLA_Ready);
    }
    // 不再采样后，仍经过五个硬件 1 ms 节拍才触发停采故障。
    for (unsigned n = 0; n < 5; ++n) {
        background_overflow = true;
        Supercap_BackgroundPoll();
        assert(CLA_Ready == (n < 4));
    }
    assert(CLA_Fault == 3 && trip[0] && trip[1]);
    puts("通过：无延时发送轮询、1ms定时调度SFO/CLA、五节拍停采互锁");
}
