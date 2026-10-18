#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "includes/telemetry.h"
#include "includes/supercap.h"

// ISR 仅写 writeIndex 所指槽；前台复制完一个槽后才推进 readIndex。
// 16 位索引可原子读写，volatile 保证先写完四路数据再发布索引。
static volatile float samples[TELEMETRY_QUEUE_DEPTH][TELEMETRY_CHANNEL_COUNT];
static volatile uint16_t writeIndex, readIndex;
static uint16_t sampleDivider, frameIndex = TELEMETRY_FRAME_OCTETS;
// C28x 的 char 为 16 位：每个元素仅向 SCI 写出低 8 位，不能按 sizeof(float) 发字节。
static uint16_t frame[TELEMETRY_FRAME_OCTETS];
static bool initialized;
volatile uint16_t Telemetry_FramesQueued, Telemetry_DroppedFrames;
volatile uint16_t Telemetry_QueueOverruns;

void Telemetry_Init(void)
{
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_SCIA);
    SCI_DEBUG_init(); // 1562500、8N1、FIFO，由 SysConfig 生成。
    GPIO_setPinConfig(SCI_DEBUG_SCITX_PIN_CONFIG);
    writeIndex = readIndex = 0;
    // 第一组有效采样立即发布，之后每 10 次控制采样一帧。
    sampleDivider = TELEMETRY_SAMPLE_DIVIDER - 1U;
    frameIndex = TELEMETRY_FRAME_OCTETS;
    Telemetry_FramesQueued = 0;
    Telemetry_DroppedFrames = 0;
    Telemetry_QueueOverruns = 0;
    initialized = true;
}

#pragma CODE_SECTION(Telemetry_Publish, ".TI.ramfunc")
void Telemetry_Publish(void)
{
    uint16_t next;
    if (++sampleDivider < TELEMETRY_SAMPLE_DIVIDER) return;
    sampleDivider = 0;
    next = (writeIndex + 1U) % TELEMETRY_QUEUE_DEPTH;
    if (next == readIndex) {
        ++Telemetry_QueueOverruns;
        return; // 绝不在控制 ISR 中等串口，也不覆盖前台正在读取的槽。
    }
    samples[writeIndex][0] = SuperCap_Control.Electronic_Data.VBUS;
    samples[writeIndex][1] = SuperCap_Control.Electronic_Data.I_IN;
    samples[writeIndex][2] = SuperCap_Control.Electronic_Data.VCAP;
    samples[writeIndex][3] = SuperCap_Control.Electronic_Data.I_CAP;
    writeIndex = next;
}

void Telemetry_Poll(void)
{
    uint16_t i, slot, budget = 16U;
    const uint16_t tail = TELEMETRY_CHANNEL_COUNT * 4U;
    float values[TELEMETRY_CHANNEL_COUNT];
    union { float value; uint32_t bits; } encoded;
    if (!initialized) return;
    if (frameIndex == TELEMETRY_FRAME_OCTETS) {
        slot = readIndex;
        if (slot == writeIndex) return;
        for (i = 0; i < TELEMETRY_CHANNEL_COUNT; ++i) values[i] = samples[slot][i];
        readIndex = (slot + 1U) % TELEMETRY_QUEUE_DEPTH;
        for (i = 0; i < TELEMETRY_CHANNEL_COUNT; ++i) {
            if (!(values[i] >= -3.402823466e38f && values[i] <= 3.402823466e38f)) {
                ++Telemetry_DroppedFrames; // 避免 Inf 与 JustFloat 帧尾混淆。
                return;
            }
            encoded.value = values[i];
            frame[i * 4U] = (uint16_t)(encoded.bits & 0xFFU);
            frame[i * 4U + 1U] = (uint16_t)((encoded.bits >> 8U) & 0xFFU);
            frame[i * 4U + 2U] = (uint16_t)((encoded.bits >> 16U) & 0xFFU);
            frame[i * 4U + 3U] = (uint16_t)(encoded.bits >> 24U);
        }
        frame[tail] = 0; frame[tail + 1U] = 0;
        frame[tail + 2U] = 0x80U; frame[tail + 3U] = 0x7FU;
        frameIndex = 0;
    }
    while (budget-- && SCI_getTxFIFOStatus(SCI_DEBUG_BASE) < SCI_FIFO_TX16) {
        SCI_writeCharNonBlocking(SCI_DEBUG_BASE, frame[frameIndex++]);
        if (frameIndex == TELEMETRY_FRAME_OCTETS) {
            ++Telemetry_FramesQueued;
            break;
        }
    }
}
