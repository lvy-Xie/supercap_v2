# 电流环接入与调试

充电控制参考 Reborn 工程，电流反馈使用本工程校准后的电容电流 `I_CAP`：采用 TI DCL CLA 版 `DCL_runPI_L4`（与原并联 C3 方程一致），电流 PI 直接输出广义占空比。闭环启动即同时运行电流 PI 和电压 PID，两者取较小占空比，实现恒流充电到恒压保持，无电压环开启阈值。`Current_VoltageLimited=1` 表示电压环正在接管，此时电流低于设定可能是正常现象。

控制环仍为 **49950.05 Hz**（20.02 µs），ADCC1 先触发 DMA 搬至 GS0，CPU 提交七路原始码后软件触发 CLA Task1；CLA 完成 ADC 换算、电压滤波、电流 PI、电压 PID、占空比分配及 HRPWM 写入。CPU 完成通知只处理启停监督与遥测采样。VOFA 的约 **4995.01 Hz** 是遥测频率。PWM、死区、SFO、校准和串口波特率保持上一版；外设配置仍由 `c2000.syscfg` 生成。

## 默认行为与 Watch 变量

本页对应默认的 `Control_Mode=0` 充电模式，固定使用电容电流 `I_CAP`；上电默认 `set_current=4.5 A`，`set_voltage=26.5 V`，功率桥保持停波。电流 PI 初值沿用参考工程 `Kp=0.0085、Ki=0.0005`，尚未完成 V2 实板整定。充电模式只支持非负目标，范围 0～15 A；15 A 是软件设定范围，不能视为硬件额定值或过流保护。`set_current=0` 是闭环零电流请求；停止 PWM 应写 `SuperCap_Control.Supercap_Enable_Flag=0`。新增的 `Control_Mode=1` 允许双向电容电流，见 [功率环](power_loop_debug.md)；切换模式须先停波。

| CCS Watch 变量 | 用途 |
| --- | --- |
| `SuperCap_Control.Supercap_Enable_Flag` | 写 1 请求启动，写 0 停波 |
| `SuperCap_Control.PWM_Running` | 实际输出状态，观察用 |
| `set_current` | 电容充电电流 I_CAP 的目标值，单位 A |
| `set_voltage` | 电容目标电压，范围大于 0 且不超过 28 V |
| `current_feedback` | 当前参与电流 PI 的实际反馈，单位 A |
| `CurrentA_loop.Kp / Ki` | 电流 PI 的比例、积分系数；没有 Kd |
| `CurrentA_loop.i10 / i6` | 积分状态、积分允许状态，观察用 |
| `pid_i_out` | 电流 PI 输出的广义占空比，电压限幅之前 |
| `pid_v_out` | 电压 PID 输出，单位为等效输出电压 |
| `Current_VoltageLimited` | 1：电压环正在限制电流 PI 输出 |
| `SuperCap_Control.PWM_Control.outputDuty` | 最终应用的广义占空比，为 dutyA/dutyB，并非某个桥的占空比 |
| `Control_LastFault` | 最近一次 CLA 控制停波原因：0 无；1 参数/输出无效；5 DMA 采集链/原始码帧异常 |
| `AdcDma_Frames / AdcDma_Fault` | DMA 成功提交计数 / 采集故障码，正常故障码为 0 |
| `CLA_Ready / CLA_Fault / HRPWM_Ready / SFO_Status` | CLA、SFO 就绪及故障状态 |

电流 PI 使用 `current_feedback=I_CAP` 与 `set_current` 比较，误差为 `set_current-I_CAP`。`I_DCDC=I_IN-I_OUT` 是输入端流入变换器的净电流，保留为观测量；它与电容侧电流随两端电压和效率变化，通常并不相等。

停波时电流积分跟踪 `VCAP/VBUS` 前馈，清除旧饱和状态；电压环接管时电流积分跟踪实际占空比。唯一控制入口为 `SuperCap_Control.Supercap_Enable_Flag`：1 请求启动、0 停波，上电和故障时为 0；`PWM_Running` 仅观察实际出波状态。建议先停止，再修改增益；不要在 PWM 输出期间暂停 CPU 或单步执行控制中断。

## 积分停滞的只读诊断

`Control_Diag` 每 50000 次控制调用发布一个窗口，正常约 1 秒，窗口之间保持数值供 CCS Watch 观察。它不参与控制决策，也没有新增使能或模式开关。重新烧录后使能并连续运行至少 2 秒，再展开该结构；`sequence` 应每秒左右变化。如果它不变化，先检查采样/CLA 是否持续执行及故障状态。

| 字段 | 含义 |
| --- | --- |
| `pi_samples` | 本窗口实际执行电流 PI 的次数，持续运行通常为 50000 |
| `tracking_samples` | 本窗口执行停波电压比前馈的次数，持续运行应为 0 |
| `voltage_limited_samples` | PI 调用中电压环接管的次数 |
| `integral_blocked_samples` | PI 调用后积分允许标志为 0 的次数 |
| `invalid_samples` | 参数检查失败的次数，正常为 0 |
| `feedback_mean/min/max` | 有效控制调用使用的电流反馈均值、最小值、最大值，单位 A |
| `integral_min/max/last` | 本窗口调用结束时的积分最小值、最大值、末值 |
| `outputDuty`、`vcap` | 窗口结束时的最终广义占空比和电容电压 |
| `peak_i_in`、`peak_i_out` | `feedback_max` 出现的同一帧输入/底盘电流 |
| `peak_i_in_raw`、`peak_i_out_raw` | 上述峰值帧的两路 ADC 原始码 |

`pi_samples + tracking_samples + invalid_samples = 50000`。如果 `tracking_samples` 持续非零，说明 CLA 实际进入过使能/运行状态未就绪的前馈分支，不能仅以某一时刻的 CPU Watch 标志判断。若 PI 持续执行且反馈均值接近目标，积分稳定可能是误差平均抵消，结合峰值判断电流波动。若反馈均值远低于目标、无电压接管及积分冻结，积分却不增长，则继续核对实际加载镜像和共享状态。含异常参数的调用不计入反馈均值；全异常窗口反馈统计为 0，通过 `invalid_samples` 区分。

停止采样后结构保留上一次发布值，必须结合 `sequence` 是否变化使用。窗口最后一次调用可能与 Watch 中的最新瞬时反馈不同；统计也不替代实际电流波形或零点校准。

`feedback_max` 现在是电容电流 `I_CAP` 的峰值；`peak_i_in/out` 及对应原始码仍是该峰值帧的输入/底盘电流快照，用于关联观察，其差值不等于 `feedback_max`。排查电容电流突跳时，应观察 `I_CAP/I_CAP_Raw` 和校准系数，并结合实测波形；Watch 中的最新原始码不保证对应窗口峰值那一帧。

## VOFA 观测值

烧录 `CPU1_FLASH/Supercap_V2.out` 后运行 CPU，VOFA 使用 **1562500 baud、8N1、JustFloat**，波形 **Δt=0.2002 ms**。固定发送以下四路，不切换视图：

| 通道 | 数据 |
| --- | --- |
| CH0 | V_IN（VBUS），V |
| CH1 | I_IN，A |
| CH2 | V_CAP（VCAP），V |
| CH3 | I_CAP，A |

目标电流、PI/PID 状态和最终占空比在 CCS Watch 观察。`Telemetry_QueueOverruns`、`Telemetry_DroppedFrames` 正常应不增长。串口只发送数据，功率桥由唯一使能变量控制。

## 台架调参顺序

1. **停波校验电流。** 使能为 0，确认 `I_IN`、`I_OUT` 和 `I_CAP` 的零点、极性及已知负载电流。固定控制 `I_CAP`，其精度依赖电容电流通道校准。ICAP 当前只有 `raw=1966 → 1 A` 单点偏置校正，斜率仍待多点校准。若极性相反，先修正传感器映射/校准，不能用负 Kp 补救。
2. **准备小电流测试。** 使用外部限流电源和可独立切断的台架。当前固件没有硬件过压/过流关断，电压 PID 不能代替它们。确认电容已有适当电压；在最小广义占空比以下，`VCAP/VBUS` 前馈会被限幅，不以空电容直接验证启动冲击。
3. **设置目标。** 使能为 0。可从 `set_current=0.2 A` 开始，但应高于实际测量噪声和零点误差。`set_voltage` 设为台架允许值；若要观察恒流响应，当前 VCAP 应低于目标电压，并结合 `Current_VoltageLimited` 确认是否受到电压环限制。不能为绕过电压限幅把设定提高到电容额定值以上。
4. **先调比例。** 停波时把 `CurrentA_loop.Ki=0`，Kp 可从较小的 `0.001` 起试；这些是试调起点，不能保证所有工况稳定。将使能置 1，确认实际电流朝目标变化。用小幅目标阶跃，例如 0.2→0.3 A，观察电流反馈；若缓慢可停波后小步提高 Kp，若出现持续振荡、噪声放大或明显超调，停止并降低增益。PI 不使用微分项。
5. **再加入积分。** 停波后可从 `Ki=0.00001` 起逐步提高，消除稳态偏差，反复观察相同小阶跃。出现低频摆动、过冲增加或恢复慢时先减小 Ki。参考值 `0.0085/0.0005` 仅供对照，不要求直接用到它。
6. **核对电压接管。** 电容接近目标时观察 `Current_VoltageLimited`：应逐渐进入恒压，电流下降。此时不能因为电流达不到设定就继续加电流 PI 增益。若电压环振荡，单独停波调整 `Voltage_loop` 的参数，并再次验证恒流到恒压过渡。
7. **验证停止与重启。** 使能为 0 后确认 `PWM_Running=0`；再次启动仍等待原有两轮采样准备。更换输入电压或负载后，重新验证小电流响应。记录参数、两端电压、负载和曲线。

当前 DCL 并联 PI 的计算是 `I[k]=I[k-1]+Ki×误差×i6`，`输出=Kp×误差+I[k]`，然后做输出限幅。Ki 已包含每拍累加的时间尺度，不能再按串口频率换算或额外乘采样周期；若将连续积分系数换算到此实现，应取 `Ki_discrete=Ki_continuous/49950.05`。`Imax/Imin` 不是 `DCL_runPI_L4` 的独立积分钳位参数，勿靠修改它们限制电流。

TI 的数字电源调试资料介绍通过实际环路响应验证补偿器，见 [C2000 数字电源控制与实现](https://www.ti.com/video/4318610130001)。本工程的主机一阶对象测试只能检查软件方向、跟踪和接管逻辑，不能证明真实功率级的稳定裕量。5 kHz 遥测也不能显示 350 kHz 开关纹波或更高频振荡；这些需用示波器验证。

## 已验证范围

主机回归覆盖固定 I_CAP 电流 PI 实际校准反馈路径、双采样启波、停波前馈、简化对象电流阶跃、CC/CV 接管与积分恢复、单一使能启停、无效参数、VOFA 固定四通道及原串口线速模型。PI/PID、PWM 分配与更新函数链接到 CLA 程序 RAM，CPU 保留控制参数和状态供 Watch 观察。共享结构只含固定宽度整数与 float，并检查 CPU/CLA 的布局。CLA 程序使用 LS0～LS1、共享数据使用 LS2，所有权由 SysConfig 生成；CPU RAM 函数移至 LS3～LS4。CCS 与脚本均使用 O2，O0 版本超出当前 CLA 程序区，不可直接改回 O0。新增测试对照 TI 原 DCL PI/PID，并验证不执行 CPU 完成中断也能完成闭环和 HRPWM 更新，以及 CLA 忙时禁止释放输出、CLA 故障先强制 OST 再由 CPU 确认。DMA 缓冲位于 GS0，由 CPU 提交到消息 RAM；详细映射和保护见 [`ADC DMA 链路`](adc_dma.md)。尚未烧录，本机测试不包含电流极性实测、闭环稳定裕量、CLA 控制任务及 CPU 通知 ISR 耗时和实机串口吞吐量。
