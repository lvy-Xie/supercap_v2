#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "includes/buzzer.h"
#include "includes/supercap.h"
#include "includes/startup_voice.h"

static volatile bool voicePlaying;
static volatile uint16_t voiceIndex;

#pragma CODE_SECTION(Buzzer_PWM_ISR, ".TI.ramfunc")
static __interrupt void Buzzer_PWM_ISR(void)
{
    if (voicePlaying && voiceIndex < BUZZER_VOICE_SAMPLE_COUNT) {
        uint16_t index = voiceIndex;
        uint16_t packed = Buzzer_StartupVoicePCM[index >> 1U];
        uint16_t sample = (index & 1U) ? (packed >> 8U) : (packed & 0xFFU);
        // 最大软件音量：使用完整调制范围，保留一拍余量避免 0%/100% 边界。
        // PCM 已在打包时按 128 零点做峰值归一化，不在 ISR 中削顶放大。
        uint16_t compare = 1U +
            (uint16_t)(((uint32_t)sample * (PWM_BUZZER_TBPRD - 2U) + 127U) / 255U);
        EPWM_setCounterCompareValue(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_A, compare);
        voiceIndex = index + 1U;
    } else {
        // 最后一个样本已保持一个采样周期；停止音频中断并拉低 Q1。
        EPWM_setActionQualifierContSWForceAction(PWM_BUZZER_BASE,
            EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
        EPWM_disableInterrupt(PWM_BUZZER_BASE);
        voicePlaying = false;
    }
    EPWM_clearEventTriggerInterruptFlag(PWM_BUZZER_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

void Buzzer_Init(void)
{
    // Q1 高电平导通；切换到 PWM 前先让 GPIO 保持低电平。
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_writePin(PWM_BUZZER_EPWMA_GPIO, 0);
    GPIO_setDirectionMode(PWM_BUZZER_EPWMA_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(PWM_BUZZER_EPWMA_GPIO, GPIO_PIN_TYPE_STD);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM4);
    // 载波、八分频采样中断、比较值影子加载和默认静音由 SysConfig 生成。
    PWM_BUZZER_init();
    EPWM_disableInterrupt(PWM_BUZZER_BASE);
    EPWM_clearEventTriggerInterruptFlag(PWM_BUZZER_BASE);
    voicePlaying = false;
    voiceIndex = 0;
    Interrupt_register(INT_EPWM4, &Buzzer_PWM_ISR);
    Interrupt_enable(INT_EPWM4);
    GPIO_setPinConfig(PWM_BUZZER_EPWMA_PIN_CONFIG);
}

void Buzzer_PlayStartupTone(void)
{
    if (voicePlaying) return;
    voiceIndex = 0;
    EPWM_setCounterCompareValue(PWM_BUZZER_BASE, EPWM_COUNTER_COMPARE_A,
                               (PWM_BUZZER_TBPRD + 1U) / 2U);
    voicePlaying = true;
    EPWM_clearEventTriggerInterruptFlag(PWM_BUZZER_BASE);
    EPWM_setActionQualifierContSWForceAction(PWM_BUZZER_BASE,
        EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_DISABLED);
    EPWM_enableInterrupt(PWM_BUZZER_BASE);
    while (voicePlaying) {
        Supercap_BackgroundPoll();
    }
}
