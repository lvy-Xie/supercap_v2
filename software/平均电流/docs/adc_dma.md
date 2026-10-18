# ADC DMA 与 CLA 控制链

本版链路为：**ePWM3 SOCA/SOCB → ADC A/B/C → ADCC1 → DMA CH1 → GS0 缓冲 → CPU 提交七路码 → CLA Task1 → PI/PID 与 HRPWM → CPU 完成通知/遥测**。

采样和控制目标仍为 49950.05 Hz，每帧 20.02 µs。保留原 SOCA/SOCB 时相，不做新增平均或多周期凑帧；七路码属于同一采样周期，不意味着七路同时转换。控制参数、校准系数、滤波、功率桥约 350 kHz、50 ns 死区、VOFA 四通道及发送分频保持现值。

## 为什么保留 CPU 提交

F28004x 的 DMA 可访问 GS RAM，CLA 可访问 LS RAM 和消息 RAM，不能直接读取 GS RAM；此型号的 CLA 触发源也不包含 DMA 完成中断。见 [TI F28004x 数据手册的内存映射及 DMA 章节](https://www.ti.com/lit/ds/symlink/tms320f280049.pdf)、本机 `driverlib/f28004x/driverlib/cla.h`。

因此，DMA 负责从 ADC 结果寄存器搬到 GS0，CPU 的 DMA 完成 ISR 只从 GS0 挑出七路码、提交 CPU→CLA 消息并软件触发 Task1。CLA 从消息读取数据，继续独立执行校准、滤波、电流 PI、电压 PID 和 HRPWM 写入，不再直接读取 ADC_RESULT。CPU 不执行闭环算法。

这条路径增加了 DMA 延迟和一次 CPU 中断/消息交接，不能据此宣称 CPU 占用更低或采样延迟更小；实板仍需验证从 ADC 完成到 CLA 控制完成的时间是否满足 20.02 µs 周期。

## SysConfig 配置

`c2000.syscfg` 新增 `DMA_ADC_CAPTURE`，使用 DMA CH1：

- 触发为 ADCC1，来源保持 ADCC SOC3（底盘电流）转换完成；同组 A/B 的控制结果此时已转换完成。
- 源地址 ADCA_RESULT0（0xB00）；每个 burst 取连续 4 个 16 位结果，共 3 个 burst。源 burst 步长 1、transfer 步长 29，让源按 A0..3、B0..3、C0..3 跳转；目标两种步长均为 1。
- 单次触发搬完整个 12-word 结果快照。额外 A2/A3/B2/B3/C2 槽不参与控制，其中软件温度 SOC 没有新增触发。
- `ONESHOT_ENABLE`、`CONTINUOUS_DISABLE`、`DMA_INT_AT_END`，启用 DMA 溢出错误中断。整帧结束自动停止通道；CPU 提交消息后重新 RUN，从 shadow 地址/计数开始下一帧。这样提交期间 GS0 不会被下一帧覆盖。行为见 [TI F28004x TRM 的 DMA 模式和地址指针章节](https://www.ti.com/lit/ug/sprui33c/sprui33c.pdf)。
- CLA Task1 改为软件触发，避免在 DMA 搬完之前仍被 ADCC1 直接触发；Task8 初始化保留。

DMA 几何、触发、模式、初始停止状态全部由 SysConfig 生成。应用只调用生成的初始化及运行/停止、清标志接口，没有手工覆盖静态配置。`device/driverlib.h` 仅补充 TI `dma.h` 声明。

## 内存与通道映射

| 数据 | 位置 | 写入方/读取方 |
| --- | --- | --- |
| `AdcDma_Buffer[12]` | GS0，0xC000 | DMA 写，CPU 读 |
| `AdcDma_Frame` | CPU→CLA 消息 RAM，0x1500 | CPU 写，CLA 读 |
| `Cla_AdcData` | CLA→CPU 消息 RAM，0x1480 | CLA 写，CPU 读 |
| `Electronic_Data`、控制参数与状态 | LS2 | CLA 更新采样/控制结果，CPU 调试/监督 |

| 消息 raw 下标 | 控制量 | DMA 缓冲下标 |
| --- | --- | --- |
| 0 | I_A | 0：A0 |
| 1 | I_B | 8：C0 |
| 2 | I_IN | 1：A1 |
| 3 | I_CAP | 9：C1 |
| 4 | I_OUT | 11：C3 |
| 5 | VBUS | 4：B0 |
| 6 | VCAP | 5：B1 |

原有 `SuperCap_Control.Electronic_Data.*_Raw` 和 `Raw[7]` 命名、排列保持不变，停波时采样与遥测继续更新。

## 完整帧与故障处理

CPU 仅在 DMA 已停、transfer 已完成且没有 ADC/DMA 溢出时提交。提交前检查 CLA 不忙，以及上一消息序号已被 CLA 消费；未完成时不覆盖消息，锁存采集故障并停波。CPU 最后发布新序号，再触发 CLA。CLA 拒绝无效/重复帧；处理结束后才写回相同序号确认消费。序号为 16 位，自然回卷。

| Watch 变量 | 正常表现 |
| --- | --- |
| `AdcDma_Frames` | 每次成功提交递增，约 1.31 秒回卷 |
| `AdcDma_Frame.sequence` | 随提交递增 |
| `Cla_AdcData.sequence` | CLA 完成后跟上提交序号；瞬时相差一帧正常 |
| `AdcDma_Frame.valid` | 首次有效提交后为 0xD4A1 |
| `AdcDma_Fault` | 0 |
| `CLA_Ready / CLA_Fault` | true / 0 |

`AdcDma_Fault`：1 为 DMA 溢出；2 为 ADC 中断溢出；3 为 CLA 忙或上一帧未消费；4 为 DMA 尚未完成却收到中断。这些错误使 `CLA_Fault=4`、`CLA_Ready=false`、`Control_LastFault=5`，停止采集并拉低两桥。必须重新初始化/启动固件后再检查，不能用使能标志绕过。CLA 遇到无效或重复消息时也会立即强制 OST，记录 `Control_LastFault=5`。原 1 ms 后台停采检查保留。

## 验证范围

主机模型直接执行生成的 DMA 初始化，按真实源/目标步长搬运模拟 ADC 结果，再运行同一份 CPU 提交和 CLA 算法。覆盖三个 ADC 块边界、七路映射、整帧提交屏障、ADC 更新后仍使用已提交快照、重复 RUN 重载、序号回卷、重复帧拒绝、CLA 忙/未消费、ADC/DMA 溢出与提前中断。原 PI/PID、HRPWM/SFO、启停、音频、VOFA 回归继续执行。

编译/链接检查确认 DMA 缓冲位于 GS0、两个方向的消息位于对应 RAM、DMA ISR 在 CPU RAM、闭环函数仍在 CLA 程序区。测试模型不模拟真实总线仲裁和指令耗时；本次未烧录，真实吞吐量、延迟与控制稳定性待实板验证。
