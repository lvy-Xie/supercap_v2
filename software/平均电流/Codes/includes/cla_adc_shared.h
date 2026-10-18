#ifndef CLA_ADC_SHARED_H
#define CLA_ADC_SHARED_H
#include <stdint.h>
#include <stddef.h>

#define CLA_ADC_INIT_MAGIC 0xA55AU
#define ADC_DMA_FRAME_MAGIC 0xD4A1U

// DMA 仅能写 GS RAM；CPU 将所需七路码提交到 CPU->CLA 消息 RAM，序号最后发布。
typedef struct {
    uint16_t raw[7];
    uint16_t valid;
    uint16_t sequence;
} AdcDma_Frame_s;
extern volatile AdcDma_Frame_s AdcDma_Frame;
typedef char DmaFrameSizeCheck[(sizeof(AdcDma_Frame_s) == 9U * sizeof(uint16_t)) ? 1 : -1];

// 跨 CPU/CLA 仅共享固定宽度整数和 float，不共享 bool、int 或指针。
// physical 顺序：I_A、I_B、I_IN、I_CAP、I_OUT、VBUS、VCAP。
typedef struct {
    float physical[7];
    uint16_t raw[7];
    uint16_t sequence;
    uint16_t initialized;
} Cla_AdcData_s;

typedef char ClaFloatWidthCheck[(sizeof(float) == sizeof(uint32_t)) ? 1 : -1];
typedef char ClaRawOffsetCheck[(offsetof(Cla_AdcData_s, raw) == 7U * sizeof(float)) ? 1 : -1];
#if defined(__TI_COMPILER_VERSION__) && !defined(__TMS320C28XX_CLA__)
// CLA 全局量默认不采用 C28x 的 64-word data-page blocking。
extern volatile Cla_AdcData_s Cla_AdcData __attribute__((noblocked));
#else
extern volatile Cla_AdcData_s Cla_AdcData;
#endif
#endif
