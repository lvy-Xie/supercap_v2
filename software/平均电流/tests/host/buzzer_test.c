#include <assert.h>
#include <stdio.h>
const void *AdcDma_Destination; // 音频隔离测试不启动 ADC/DMA，仅满足生成初始化的链接。
#include <stdint.h>

static void audio_delay(uint32_t microseconds);
#define DEVICE_DELAY_US(us) audio_delay(us)
#include "Codes/buzzer.c"

static unsigned played_samples, sfo_services;
static uint16_t loudest_compare;

// 主机用每轮 1 ms 的模拟时钟推动音频；真实前台由硬件 Timer1 调度 SFO。
void Supercap_BackgroundPoll(void) { ++sfo_services; audio_delay(1000); }

static void audio_delay(uint32_t microseconds)
{
    assert(microseconds == 1000);
    // 模拟每毫秒约 8 个采样中断，检查整段音频及播放结束边界。
    for (unsigned tick = 0; tick < 8 && buzzer_interrupt_enabled; ++tick) {
        uint16_t index = voiceIndex;
        Buzzer_PWM_ISR();
        if (index < BUZZER_VOICE_SAMPLE_COUNT) {
            uint16_t word = Buzzer_StartupVoicePCM[index / 2U];
            uint16_t sample = index % 2U ? word >> 8U : word & 255U;
            assert(cmp[3] == 1U + (sample * 1559U + 127U) / 255U);
            assert(cmp[3] > 0 && cmp[3] < period_active[3]);
            if (cmp[3] > loudest_compare) loudest_compare = cmp[3];
            ++played_samples;
        }
    }
}

int main(void)
{
    trip[0] = trip[1] = EPWM_TZ_FLAG_OST;
    cmp[0] = 114; cmp[1] = 28;
    Buzzer_Init();
    assert(!voicePlaying && !buzzer_interrupt_enabled);
    assert(buzzer_force == EPWM_AQ_SW_OUTPUT_LOW);
    assert(period_active[3] == 1561 && buzzer_irq_div == 8);
    assert(100000000U / (1562U * buzzer_irq_div) == 8002U);
    for (unsigned repeat = 1; repeat <= 2; ++repeat) {
        Buzzer_PlayStartupTone();
        assert(played_samples == repeat * BUZZER_VOICE_SAMPLE_COUNT);
        assert(voiceIndex == BUZZER_VOICE_SAMPLE_COUNT);
        assert(!voicePlaying && !buzzer_interrupt_enabled);
        assert(buzzer_force == EPWM_AQ_SW_OUTPUT_LOW);
        assert(trip[0] == EPWM_TZ_FLAG_OST && trip[1] == EPWM_TZ_FLAG_OST);
        assert(cmp[0] == 114 && cmp[1] == 28);
    }
    assert(sfo_services > 0);
    assert(loudest_compare == 1560U); // 满幅音频使用最大比较值，仍小于 TBPRD。
    puts("通过：开机语音逐样本播放、字节顺序、重播、结束静音、继续SFO及功率桥停波状态保持");
}
