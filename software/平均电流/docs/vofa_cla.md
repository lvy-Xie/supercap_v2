# 电容侧 VOFA 串口与 CLA

## 接线与 VOFA 设置

使用 SCIA，参数由 `c2000.syscfg` 配置：**1562500 baud**、8 位数据、无校验、1 位停止位。发送采用 [VOFA 官方 JustFloat 协议](https://www.vofa.plus/docs/learning/dataengines/justfloat/)，4 个小端 float32 后追加 `00 00 80 7F`，每帧 20 字节。

| 板上引脚 | 64PM 物理脚 | 连接 USB 转 3.3 V TTL 串口 |
| --- | --- | --- |
| GPIO24 / SCIA_TX | 35 | 转换器 RX |
| GND | — | 转换器 GND |
| GPIO17 / SCIA_RX | 34 | 保留引脚，本版不接收串口命令，无需接线 |

1. 重新烧录 `CPU1_FLASH/Supercap_V2.out` 并运行。CCS 下载后若停在 `main`，点击 Resume；CPU 暂停时不会继续发送。
2. VOFA 选择对应串口，将波特率输入为 **1562500**、8N1，协议引擎选择 **JustFloat**。USB 转串口设备需支持这个波特率。
3. 添加 4 个曲线通道，按下表命名，波形 **Δt 设为 0.2002 ms**。该值控制横轴和频谱的时间尺度，见 [VOFA 波形控件说明](https://www.vofa.plus/docs/learning/widgets/wave/)。上电保持功率桥停波，采样与串口仍更新；开机语音期间也继续发送。

## 通道顺序

| VOFA 通道（从 0 开始） | 数据 | 单位/范围 |
| --- | --- | --- |
| CH0 | 输入电压 `V_IN`（固件变量 `VBUS`），已校准及低通滤波 | V |
| CH1 | 输入电流 `I_IN`，按当前校准系数换算 | A |
| CH2 | 电容电压 `V_CAP`（固件变量 `VCAP`），已校准及低通滤波 | V |
| CH3 | 电容侧电流 `I_CAP`，按当前校准系数换算 | A |

通道固定为以上四路，不再切换调试视图。电流环反馈使用电容电流 `I_CAP`，`set_current` 为电容充电电流目标；其它控制变量可在 CCS Watch 观察，见 [`电流环调试`](current_loop_debug.md)。

第一组有效采样立即发布首帧；之后控制采样频率仍为 49.950 kHz，每 **10 次**采样发布一帧，目标 **4995.005 帧/秒**、间隔 **200.2 µs**，是原 500 分频、99.900 Hz 的 50 倍。PID 的执行频率及参数保持原值。

当前 LSPCLK 为 25 MHz，SCI 波特率分频值为 1，对应精确的 1562500 baud。[TI 给出的 SCI 最高波特率为 LSPCLK/16](https://e2e.ti.com/support/microcontrollers/c2000-microcontrollers-group/c2000/f/c2000-microcontrollers-forum/927073/tms320f280049c-max-sci-uart-baud-rate)，因此已达到当前串口时钟下的波特率上限。每帧 20 字节在 8N1 下占 200 位，理论极限为 7812.5 帧/秒；本版 4995.005 帧/秒占线速约 63.94%，保留约 36% 余量。

采样和发送不依赖 `Supercap_Enable_Flag`，停波也持续更新。CPU 的 CLA 完成通知中断只按分频复制四路数据到环形队列，不等待串口。队列有 16 个槽，可存 15 帧（约 3.0 ms），满时丢弃新采样并计数。前台持续调用 `Telemetry_Poll()`，每次最多写 16 个发送字节，FIFO 满立即返回。整帧使用同一次采样，发送期间不会混入下一组数据。NaN/Inf 帧丢弃，避免 Inf 与 JustFloat 帧尾混淆。

已移除前台固定 1 ms 等待；SFO 与 CLA 停采检查由 CPU Timer1 每 1 ms 调度，Timer1 不产生中断。开机语音的等待循环也使用同一后台轮询。

C28x 的 `char` 是 16 位；代码按 float32 的 32 位位型明确拆成 4 个 8 位数据写 SCI，不按 `sizeof(float)` 或字符指针直接发送。

可在 CCS 观察 `Telemetry_FramesQueued`（完整帧排入 FIFO 的计数）、`Telemetry_DroppedFrames`（异常浮点导致的丢帧计数）和 `Telemetry_QueueOverruns`（队列满导致的丢帧计数）。后两者正常运行应不增长。三个计数均为 16 位，会自然回卷；正常发送时帧计数约每 13.1 秒回卷一次。计数反映固件发送情况，不能检测 USB 转串口或电脑端丢失。

## 使能控制

两路功率桥只通过 CCS Watch 中的 `SuperCap_Control.Supercap_Enable_Flag` 控制：写 `1` 启动，写 `0` 停止。`SuperCap_Control.PWM_Running` 是实际运行状态，只读观察。上电默认停波；CLA/SFO 必须就绪，仍经过原有两轮采样装载比较值后再启波。

串口固定按 V_IN、I_IN、V_CAP、I_CAP 顺序发送四路数据，停波和开机语音期间持续更新。串口命令解析、启停接口和接收错误恢复已移除；收到的串口内容不会改变 使能控制状态，也不会因 RX 错误复位发送器或打断帧尾。只需 GPIO24 接转换器 RX 并共地，GPIO17 无需连接。

## CLA 承担的任务

流程为：ePWM3 触发采样 → ADCC1 → DMA 搬至 GS0 → CPU 提交七路原始码并软件触发 CLA → CLA Task1 完成采样换算、PI/PID、占空比分配及 HRPWM 写入 → CPU 完成通知处理启停监督与遥测。

- **CLA Task 1**：读取 CPU→CLA 消息中的七路 DMA 原始码，按 `Codes/includes/config.h` 的当前系数换算电流和电压，对 VBUS/VCAP 做原有低通滤波，计算 I_DCDC，执行电流 PI/电压 PID、占空比分配并直接更新两桥 HRPWM。
- **CLA Task 8**：启动时一次性初始化滤波、控制积分状态与共享消息。CPU 等待初始化完成，超时保持停波。
- **CPU**：初始化硬件和控制参数；DMA 完成后提交七路原始码并触发 CLA；在 CLA 完成通知中处理 使能控制、故障监督，并按分频发布遥测。前台执行 SFO 校准和 SCI 发送。CPU 不执行 PI/PID 或正常占空比更新。

采样、滤波、电流 PI、电压 PID 和 PWM 更新全部在 CLA 执行。CPU 在启动前设置参数、运行中提交 DMA 原始码，并保留控制参数供 Watch 停波调参。CLA 检测到控制异常时立即强制两桥 OST、发布停波请求；CPU 完成通知清除使能和运行状态。CPU 仅在释放输出瞬间暂时禁止新 Task1，确认 CLA 不忙且无停波请求后再释放，随后恢复 Task1。七路系数、滤波参数、功率桥频率、50 ns 死区及启停方式保持现值。电容电流仍属于当前记录的单点偏置校正，串口及 CLA 不会增加其校准覆盖范围。

| 内存 | 用途 |
| --- | --- |
| LS0～LS1，0x8000～0x8FFF | CLA 程序，从 Flash 复制 |
| LS2，0x9000～0x97FF | CLA 常量、滤波状态、scratchpad、CPU/CLA 共享控制参数和数据 |
| LS3～LS4，0x9800～0xA7FF | CPU 通知/停波、音频 ISR 和遥测发布 RAM 函数 |
| LS5，0xA800～0xAFFF | CPU 数据 |
| 0x1480～0x14FF | CLA → CPU 消息 RAM |
| 0x1500～0x157F | CPU → CLA 七路原始码、有效标记与序号 |
| GS0，0xC000 起 | DMA 的 12-word ADC 结果快照，CPU 读取 |

内存所有权、消息 RAM 初始化、Flash 复制、任务向量、ADCC1→DMA 触发源与 DMA 传输参数由 SysConfig 生成。共享结构只含固定宽度整数和 float，不共享 bool 或指针；共享对象在 CPU 源文件定义，CLA 引用；消息声明使用 `noblocked`。CPU/CLA 编译时均检查控制结构大小和偏移。消息 RAM 由 MemCfg 硬件初始化，链接时排除 CPU 的自动清零写入。配置参考 [TI CLA 软件指南](https://software-dl.ti.com/C2000/docs/cla_software_dev_guide/index.html) 及本机 C2000Ware 的 CLA 示例。

启动顺序必须是：停止并复位 CLA → 暂时将 LS0、LS1 交回 CPU（兼容 CCS Restart 保留的 MemCfg 状态）→ `CLA_init()` 从 Flash 复制程序/常量并配置任务 → `MEMCFG_init()` 设置最终 RAM 所有权并初始化消息 RAM → 触发 Task 8。旧版先调用 `MEMCFG_init()`，使 CPU 向 CLA 程序 RAM 的复制被硬件阻止，导致数据一直为零；现已修正。访问限制见 [F28004x TRM 的 Local Shared RAM 章节](https://www.ti.com/lit/ug/sprui33h/sprui33h.pdf)。最终 LS0/LS1/LS2 分配仍由 `c2000.syscfg` 维护。

## 状态与验证

`CLA_Ready=true` 表示初始化成功且未锁存故障。`Cla_AdcData.sequence` 在每次 CLA Task 1 完成时递增，可用于观察任务是否持续运行。

| `CLA_Fault` | 含义 |
| --- | --- |
| 0 | 无故障 |
| 1 | 初始化 Task 8 等待超时 |
| 2 | Task 1 触发溢出 |
| 3 | 连续 5 次后台检查采样序号未变化；由 Timer1 按约 1 ms 节拍检查 |
| 4 | DMA/ADC 采集链故障，详见 `AdcDma_Fault` |

DMA 提交计数 `AdcDma_Frames` 应持续递增，`AdcDma_Fault` 正常为 0；细分故障码见 [`ADC DMA 链路`](adc_dma.md)。

CLA 故障锁存后拉低两桥，使能标志不能绕过，重新初始化/启动固件后再检查。SFO 校准及其故障停波继续有效。

```sh
sh tools/build.sh
sh tests/run_host_checks.sh
```

主机测试直接执行同一份 CLA C 算法，模拟硬件触发与寄存器，覆盖程序 RAM 复制权限、Restart 重新装载、停波启动首帧、校准和滤波、初始化超时、任务溢出、停采、启波互锁，以及四通道字节顺序、帧尾、10 分频、FIFO 满、队列溢出、环形槽复用和异常浮点。线速模型覆盖 100 ms 连续采样，并模拟每 1 ms 有 60 µs 前台服务空档，验证该模型下无队列溢出；它不模拟 C28x 指令耗时。后台测试验证高速轮询不会误触发 CLA 停采，SFO 只在 1 ms 节拍调用。遥测回归覆盖 RX 错误/溢出及旧命令内容不会改变 使能控制、不会复位发送器或截断帧尾。新增测试验证 CLA 独立执行控制及 HRPWM 写入、PI/PID 与原 DCL 一致、启动时任务忙不会释放输出，以及 CLA 故障立即 OST 后由 CPU 确认。链接检查确认控制函数属于 CLA 程序区，CPU/CLA 的 RAM 地址与复制区间及串口命令模块已移除。CCS 和脚本统一使用 O2；O0 控制程序超过当前 CLA 程序区。新增 DMA 回归覆盖完整搬运屏障、七路映射、消息消费握手、序号回卷和 ADC/DMA 溢出。本次 DMA/CLA 控制版本尚未烧录，串口设备对新波特率的支持、实际接收速率、CLA 任务耗时及控制中断时序仍需实板验证。
