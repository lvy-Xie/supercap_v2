#ifndef BUZZER_H
#define BUZZER_H

// 在 Device_initGPIO() 之后、Supercap_Init() 启动时基之前调用。
// GPIO6 / ePWM4A，约 64 kHz PWM 载波，初始化后保持静音。
void Buzzer_Init(void);

// 开机播放“我是顾子文”后静音；前台等待约 1.05 秒，不关闭控制中断，继续 SFO 和串口轮询。
// 在 Buzzer_Init()、Supercap_Init() 和 EINT 之后调用，勿在 ISR 中调用。
void Buzzer_PlayStartupTone(void);

#endif
