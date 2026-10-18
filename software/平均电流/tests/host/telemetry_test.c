#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "control_runtime.h"

static void publish(void)
{
    for (unsigned n = 0; n < TELEMETRY_SAMPLE_DIVIDER; ++n) Telemetry_Publish();
}

static void drain(void)
{
    for (unsigned n = 0; n < 2 * TELEMETRY_QUEUE_DEPTH + 2; ++n) {
        sci_fifo_used = 0;
        Telemetry_Poll();
    }
}

static float channel(unsigned offset, unsigned ch)
{
    union { uint32_t bits; float value; } decoded;
    unsigned p = offset + ch * 4;
    decoded.bits = (uint32_t)sci_bytes[p] | ((uint32_t)sci_bytes[p + 1] << 8) |
                   ((uint32_t)sci_bytes[p + 2] << 16) | ((uint32_t)sci_bytes[p + 3] << 24);
    return decoded.value;
}

int main(void)
{
    Telemetry_Init();
    assert(sci_initialized);
    Telemetry_Poll();
    assert(sci_byte_count == 0);
    SuperCap_Control.Electronic_Data.VCAP = 13.5f;
    SuperCap_Control.Electronic_Data.I_CAP = -1.25f;
    SuperCap_Control.Electronic_Data.VBUS = 24;
    SuperCap_Control.Electronic_Data.I_IN = 0.75f;
    SuperCap_Control.Electronic_Data.VCAP_Raw = 1642;
    SuperCap_Control.Electronic_Data.I_CAP_Raw = 1966;
    SuperCap_Control.PWM_Control.dutyB = 0.9f;
    SuperCap_Control.PWM_Control.outputDuty = 0.6f;
    SuperCap_Control.PWM_Running = true;
    set_voltage = 15;
    Telemetry_Publish(); // 上电第一组有效采样立即发布。
    assert(sci_byte_count == 0); // 中断发布绝不写串口。
    sci_fifo_used = 16;
    Telemetry_Poll();
    assert(sci_byte_count == 0); // FIFO 满立即返回，不等待硬件。
    sci_fifo_used = 0;
    Telemetry_Poll();
    assert(sci_byte_count == 16);
    // 发送半帧时有新快照，也不能混入当前帧。
    SuperCap_Control.Electronic_Data.VCAP = 20;
    SuperCap_Control.Electronic_Data.I_CAP = 2;
    SuperCap_Control.Electronic_Data.I_IN = 1.5f;
    uint16_t previousWrite = writeIndex;
    for (unsigned n = 0; n < TELEMETRY_SAMPLE_DIVIDER - 1; ++n) Telemetry_Publish();
    assert(writeIndex == previousWrite); // 后续每 10 个采样发布一次。
    Telemetry_Publish();
    drain();
    assert(sci_byte_count == 40 && Telemetry_FramesQueued == 2);
    const float expected[] = {24, 0.75f, 13.5f, -1.25f};
    for (unsigned ch = 0; ch < 4; ++ch) assert(channel(0, ch) == expected[ch]);
    assert(channel(0, 1) == 0.75f && channel(20, 1) == 1.5f);
    assert(channel(20, 0) == 24 && channel(20, 2) == 20 && channel(20, 3) == 2);
    // 24 的 IEEE-754 字节必须显式按小端发出。
    assert(sci_bytes[0] == 0 && sci_bytes[1] == 0 && sci_bytes[2] == 0xC0 && sci_bytes[3] == 0x41);
    for (unsigned offset = 0; offset <= 20; offset += 20) {
        assert(sci_bytes[offset + 16] == 0 && sci_bytes[offset + 17] == 0);
        assert(sci_bytes[offset + 18] == 0x80 && sci_bytes[offset + 19] == 0x7F);
    }
    // 其它控制观测值不参与遥测，固定只发送输入/电容侧四路物理量。
    current_feedback = NAN;
    set_current = NAN;
    SuperCap_Control.PWM_Control.outputDuty = NAN;
    publish(); drain();
    assert(sci_byte_count == 60);
    assert(channel(40, 0) == 24 && channel(40, 1) == 1.5f);
    assert(channel(40, 2) == 20 && channel(40, 3) == 2);
    set_current = 1.0f;
    SuperCap_Control.Electronic_Data.I_IN = INFINITY;
    publish(); drain();
    assert(Telemetry_DroppedFrames == 1 && sci_byte_count == 60);
    SuperCap_Control.Electronic_Data.VBUS = NAN;
    publish(); drain();
    assert(Telemetry_DroppedFrames == 2 && sci_byte_count == 60);
    SuperCap_Control.Electronic_Data.I_IN = 0.75f;
    SuperCap_Control.Electronic_Data.VBUS = 24;
    // 前台短暂停顿时，ISR 只缓存，不等待串口；满队列丢新帧并计数。
    sci_byte_count = sci_fifo_used = 0;
    Telemetry_Init();
    for (unsigned n = 0; n < TELEMETRY_QUEUE_DEPTH + 1; ++n) {
        SuperCap_Control.Electronic_Data.VCAP = 100 + n;
        publish();
    }
    assert(sci_byte_count == 0 && Telemetry_QueueOverruns == 2);
    drain();
    assert(sci_byte_count == (TELEMETRY_QUEUE_DEPTH - 1) * TELEMETRY_FRAME_OCTETS);
    for (unsigned n = 0; n < TELEMETRY_QUEUE_DEPTH - 1; ++n)
        assert(channel(n * TELEMETRY_FRAME_OCTETS, 2) == 100 + n);
    // 多次跨过队列末尾，检查槽位复用和采样顺序，不出现混帧。
    unsigned offset = sci_byte_count;
    for (unsigned n = 0; n < 64; ++n) {
        SuperCap_Control.Electronic_Data.VCAP = 200 + n;
        publish(); drain();
        assert(channel(offset + n * TELEMETRY_FRAME_OCTETS, 2) == 200 + n);
    }
    assert(Telemetry_QueueOverruns == 2);
    // 冷启动后保持 使能为 0，第一组 ADC -> CLA -> CPU 结果就必须发到 VOFA。
    sci_byte_count = 0;
    sci_fifo_used = 0;
    Telemetry_Init();
    Supercap_Init();
    adc[1][1] = 1862;
    adc[2][1] = 1966;
    adc[1][0] = 2987;
    adc[0][1] = 2049;
    sample_tick();
    drain();
    assert(sci_byte_count == TELEMETRY_FRAME_OCTETS);
    assert(channel(0, 2) > 0 && fabsf(channel(0, 3) - 1.0f) < 0.00001f);
    assert(channel(0, 0) == SuperCap_Control.Electronic_Data.VBUS);
    assert(channel(0, 1) == SuperCap_Control.Electronic_Data.I_IN);
    assert(!SuperCap_Control.Supercap_Enable_Flag);
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    // 串口已移除控制功能：旧启动命令和 RX 错误不得改变 使能标志。
    sci_byte_count = sci_fifo_used = 0;
    Telemetry_Init();
    const char enable[] = "EN=1\n";
    for (unsigned i = 0; i < sizeof(enable) - 1; ++i) sci_rx_bytes[i] = enable[i];
    sci_rx_count = sizeof(enable) - 1;
    sci_rx_index = 0;
    Telemetry_Poll();
    assert(!SuperCap_Control.Supercap_Enable_Flag);
    assert(!SuperCap_Control.PWM_Running);
    // 只有使能变量置位才启动；RX 故障、旧停止命令不能将其清零。
    SuperCap_Control.Supercap_Enable_Flag = true;
    sample_tick(); sample_tick();
    assert(SuperCap_Control.PWM_Running && !trip[0] && !trip[1]);
    // 接下来只检查四个手动发布的帧，先排空控制采样已发布的数据。
    Telemetry_Init();
    const char disable[] = "EN=0\n";
    for (unsigned i = 0; i < sizeof(disable) - 1; ++i) sci_rx_bytes[i] = disable[i];
    sci_rx_count = sizeof(disable) - 1;
    sci_rx_fault_persistent = true;
    sci_rx_status = SCI_RXSTATUS_ERROR;
    sci_rx_overflow = true;
    uint16_t resets = sci_software_resets;
    for (unsigned n = 0; n < 4; ++n) {
        publish();
        sci_fifo_used = 0;
        Telemetry_Poll();
        assert(sci_byte_count == n * 20 + 16);
        sci_fifo_used = 0;
        sci_shift_busy = true;
        Telemetry_Poll();
        assert(sci_byte_count == (n + 1) * 20);
        assert(sci_software_resets == resets);
        sci_fifo_used = 0;
        Telemetry_Poll();
        assert(sci_software_resets == resets);
        sci_shift_busy = false;
        Telemetry_Poll();
        assert(sci_software_resets == resets);
        assert(sci_bytes[n * 20 + 16] == 0 && sci_bytes[n * 20 + 17] == 0);
        assert(sci_bytes[n * 20 + 18] == 0x80 && sci_bytes[n * 20 + 19] == 0x7F);
    }
    assert(Telemetry_FramesQueued == 4 && sci_rx_index == 0);
    assert(SuperCap_Control.Supercap_Enable_Flag && SuperCap_Control.PWM_Running);
    assert(!trip[0] && !trip[1]);
    SuperCap_Control.Supercap_Enable_Flag = false;
    sample_tick();
    assert(!SuperCap_Control.PWM_Running && trip[0] && trip[1]);
    sci_rx_fault_persistent = false;
    // 按真实比率模拟 100 ms：控制采样每 2002 个 SYSCLK，UART 每字节 640 个
    // SYSCLK；前台每 10 us 轮询，并每 1 ms 留出 60 us 后台服务空档。
    // 这里只验证传输调度和缓冲容量，不模拟 C28x 指令执行耗时。
    sci_byte_count = sci_fifo_used = 0;
    Telemetry_Init();
    uint32_t nextControl = 2002, txCredit = 0, controlSamples = 0;
    for (uint32_t cycles = 0; cycles < 10000000; cycles += 1000) {
        txCredit += 1000;
        uint32_t drained = txCredit / 640;
        txCredit %= 640;
        sci_fifo_used = sci_fifo_used > drained ? sci_fifo_used - drained : 0;
        while (cycles >= nextControl) {
            ++controlSamples;
            SuperCap_Control.Electronic_Data.VCAP = (float)controlSamples;
            Telemetry_Publish();
            nextControl += 2002;
        }
        if (cycles % 100000 >= 6000) Telemetry_Poll();
    }
    drain();
    assert(Telemetry_QueueOverruns == 0 && Telemetry_FramesQueued == 500);
    assert(sci_byte_count == 500 * TELEMETRY_FRAME_OCTETS);
    for (unsigned n = 0; n < 500; ++n) {
        assert(channel(n * TELEMETRY_FRAME_OCTETS, 2) == (float)(1 + n * 10));
        assert(sci_bytes[n * 20 + 18] == 0x80 && sci_bytes[n * 20 + 19] == 0x7F);
    }
    puts("通过：停波首帧、四通道小端帧/帧尾、10分频、非阻塞、队列溢出和跨槽顺序");
    puts("通过：单一使能启停，旧串口命令/RX错误不改变状态、不复位TX，帧尾完整发送");
    puts("通过：1.5625Mbps线速模型下5kHz四路连续采样、后台服务空档与零队列溢出");
}
