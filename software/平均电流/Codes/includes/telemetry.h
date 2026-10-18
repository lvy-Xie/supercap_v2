#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <stdint.h>

#define TELEMETRY_CHANNEL_COUNT 4U // V_IN(VBUS)、I_IN、V_CAP(VCAP)、I_CAP。
#define TELEMETRY_FRAME_OCTETS (TELEMETRY_CHANNEL_COUNT * 4U + 4U)
#define TELEMETRY_SAMPLE_DIVIDER 10U // 49.950 kHz / 10 = 4995.005 Hz。
#define TELEMETRY_QUEUE_DEPTH 16U // 单生产者/单消费者环形队列，可缓冲 15 帧。
extern volatile uint16_t Telemetry_FramesQueued, Telemetry_DroppedFrames;
extern volatile uint16_t Telemetry_QueueOverruns; // 队列满丢弃新采样的计数；正常运行应不增长。
void Telemetry_Init(void);
void Telemetry_Publish(void); // 控制中断调用；分频采样入队，不发送串口。
void Telemetry_Poll(void);    // 前台调用；仅非阻塞发送四路数据，不处理串口命令。
#endif
