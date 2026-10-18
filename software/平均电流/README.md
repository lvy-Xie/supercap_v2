# Supercap V2：约 350 kHz HRPWM 电流/电压控制

当前固定使用电容电流 `I_CAP` 反馈的电流 PI 恒流充电与电压 PID 限幅，目标为 1 A / 16 V；上电保持停波。唯一使能变量为 `SuperCap_Control.Supercap_Enable_Flag`：写 1 请求启动，写 0 停波，故障时自动清零。`PWM_Running` 仅观察实际出波状态。GPIO0/1 为输入侧 A 桥，GPIO2/3 为电容侧 B 桥；两桥均使用 HRPWM 占空比控制、互补低侧及 50 ns 死区，B 桥时基相移 180°。

`c2000.syscfg` 是静态外设参数的来源。100 MHz 时基采用 `TBPRD=285`，实际开关频率 **349.650 kHz**（相对 350 kHz 低约 0.10%）；本版使用整数周期和 HR 占空比。ePWM3 同周期，SOCA/SOCB 七分频，采样与电压环为 **49.950 kHz**。PID 与滤波参数保留现值。


## HRPWM 与 SFO 校准

参考本机 `Supercap_49C/SuperCap_Reborn` 和 TI `hrpwm_ex1_duty_sfo.c`、`hrpwm_ex9_dutyhr_updown_deadband_sfo.c`：

- 两桥的 CMPA/CMPB 写入相同 Q8 比较值，保留占空比小数；整数比较点按例程将 HR 部分设为 1。
- 每个半桥的高侧上升沿与互补低侧下降沿使用相同 MEP 位移，比较值及 HR 部分均在计数器归零加载，启用自动 MEP 换算。
- 启动时在时基未开启、栅极 GPIO 保持低电平期间完成首次 SFO 校准。成功后 `HRPWM_Ready=true`，仍需手动设置 使能标志才能出波。
- 前台持续轮询串口，CPU Timer1 每 1 ms 调度一次 `Supercap_ServiceHRPWM()`，跟踪温度和电压变化。后台 `SFO_INCOMPLETE` 时沿用上次有效标定值。
- 首次校准超过 100000 次调用仍未完成，或任意阶段返回 `SFO_ERROR`，锁存错误并拉低两桥；使能标志不能绕过校准，重新启动固件后再校准。
- CCS 和 `tools/build.sh` 均链接 F28004x 的 `SFO_v8_fpu_lib_build_c28_driverlib_eabi.lib`。

可在 CCS 观察 `SFO_Status`（0=进行中，1=完成，2=错误）、`HRPWM_Ready`、`MEP_ScaleFactor` 和 `SuperCap_Control.PWM_Control.COMPA_A_Q8/COMPA_B_Q8`。Q8 是 HRPWM API 的逻辑输入格式，实际分辨率由 SFO 标定的 MEP 步长决定。

## 电流／功率闭环与电压限幅

`main.c` 是入口，`Codes/supercap.c` 处理 CPU 侧启停监督；`Codes/cla_tasks.cla` 执行校准换算、电压滤波及闭环控制。`Codes/includes/control_cla.h` 中的 `Control_Loop()` 按 `Control_Mode` 选择 `Test_Charge_Loop()` 或独立的 `Power_Control_Loop()`，共用电流 PI、电压 PID 选小控制和 `Update_PWM_Duty()`，全部由 CLA 编译。`Codes/control.c` 仅定义共享调试参数和启动初值。

控制参考 `/Users/lvy/Supercap/Supercap_49C/SuperCap_Reborn`，两种模式的内层反馈均为电容电流 `I_CAP`；`I_DCDC=I_IN-I_OUT` 保留为观测量。默认 `Control_Mode=0`，`set_current` 为电容充电电流目标；停波后改为 `Control_Mode=1`，以 `set_power` 为输入功率目标，功率 PID 反馈为 `VBUS×I_IN`，其输出除以 `VCAP` 得到双向电容电流目标，不做效率前馈。两种模式均用 `set_voltage` 限制电容电压。电压环接管时跟踪电流积分，停波时电流积分跟踪电压比前馈并清零功率 PID。见 [`功率环调试`](docs/power_loop_debug.md) 和 [`电流环调试`](docs/current_loop_debug.md)。

## CCS 调试

- `Control_Mode`：停波后切换，0=恒流充电（默认），1=输入功率闭环。
- `set_power`：输入功率目标，默认 30 W；`Referee_Power_Loop.Kp/Ki/Kd` 初值为 0.5/0.01/1.0，沿用参考工程。
- `power_feedback / target_DCDC_Power / target_cap_current`：实际输入功率／功率 PID 输出／电容电流目标；PID 输出是控制指令，不是实测 DCDC 功率。
- `set_voltage`：电容目标电压，默认 26.5 V，允许大于 0 且不超过 28 V。
- `set_current`：充电模式目标电流，默认 4.5 A，允许 0～15 A；范围不代表硬件额定或保护值。
- `CurrentA_loop.Kp/Ki`：电流 PI，初值 0.0085/0.0005，沿用参考工程，尚需实板整定。
- `Control_Diag`：每约 1 秒发布的只读控制统计，记录 PI/停波前馈次数、电流反馈均值及峰值、积分变化；积分停滞排查见 [`电流环调试`](docs/current_loop_debug.md#积分停滞的只读诊断)。
- `SuperCap_Control.Supercap_Enable_Flag`：写 1 启动，写 0 停止；上电为 0。
- `SuperCap_Control.PWM_Running`：实际运行状态，只读观测。
- `SuperCap_Control.PWM_Control`：广义占空比、两桥占空比、比较值。
- `SuperCap_Control.Electronic_Data`：七路原始 ADC 及换算结果。
- `SuperCap_Control.Electronic_Data.VBUS_Raw/VCAP_Raw/I_IN_Raw/I_OUT_Raw/I_CAP_Raw/I_A_Raw/I_B_Raw`：可按通道名观察最新 ADC 原始码，未经校准或滤波，停波时仍更新；原 `Raw[7]` 数组保留。
- `Voltage_loop.Kp/Ki/Kd`：电压环参数；初值沿用 49C，V2 实板需重新整定，停机修改。

四开关函数沿用 49C 的 Buck／Buck-Boost／Boost 滞回分配。三路 ePWM 的周期直接加载配置由 SysConfig 生成，在开启 TBCLKSYNC 前即写入有效周期，避免 B 桥启动时无法命中比较事件。B 桥初始 `TBCTR=143`，`TBPHS=145` 补偿内部同步的 2 TBCLK 延迟；ePWM3 的 `TBPHS=4` 补偿级联同步延迟，`CMPA=71`、`CMPB=214`，SOCA/SOCB 均七分频。

ADCC1 按约 50 kHz 触发 DMA CH1，将 A/B/C 三组结果搬到 GS0。DMA 完成 ISR 将七路有效码提交到 CPU→CLA 消息 RAM，再软件触发 CLA Task1，完成校准换算和 VBUS/VCAP 低通滤波；CLA Task1 直接执行所选闭环、电压限幅和 HRPWM 写入。CPU 的 CLA 完成通知只处理启停监督和遥测发布。CLA 程序位于 LS0～LS1，共享控制数据位于 LS2，CPU RAM 函数位于 LS3～LS4；SFO 和串口发送在前台执行。启停时重置积分状态，启动前等待一轮采样以装载比较值。

## VOFA 电容侧数据与 CLA

SCIA 已启用：GPIO24 为 TX、GPIO17 为 RX，**1562500 baud、8N1**，VOFA 选择 **JustFloat**，波形 **Δt 设为 0.2002 ms**。固定发送四通道，顺序为 `V_IN`（VBUS）、`I_IN`、`V_CAP`（VCAP）、`I_CAP`，每帧 20 字节。第一组有效采样立即发布首帧，之后每 10 次控制采样发布一帧，目标 **4995.005 Hz**（原约 99.9 Hz 的 50 倍）。发送走前台非阻塞 FIFO，用 15 帧容量的队列缓冲短时繁忙，停波及开机语音期间也持续更新，无需先使能功率桥。观察 `Telemetry_QueueOverruns` 可检查固件是否发送不及；正常运行应不增长，实际接收速率仍需烧录后验证。

功率桥仅通过 CCS Watch 的 `SuperCap_Control.Supercap_Enable_Flag` 控制：写 1 启动、写 0 停止；`PWM_Running` 观察实际状态。上电默认停波，启动仍需 CLA/SFO 就绪并经过原有采样准备，停止后采样和串口照常运行。串口命令及 RX 错误恢复已移除，RX 错误不会复位发送器或打断帧尾。GPIO24 接转换器 RX 并共地，GPIO17 无需接线。接线、通道单位和验证说明见 [`docs/vofa_cla.md`](docs/vofa_cla.md)。

VOFA 固定发送上述四路物理量；控制目标、实际反馈与占空比可在 CCS Watch 观察。

CLA Task1 承担采样换算、电压滤波、电流 PI、电压 PID 和 HRPWM 更新；CPU 承担 DMA 原始码提交、启停监督、SFO、串口和开机音频。停波修改控制增益；使能和运行状态使用上述 Watch 名称。观察 `CLA_Ready`、`CLA_Fault` 和 `Cla_AdcData.sequence` 可检查初始化及运行状态。初始化超时、任务溢出、DMA 采集链故障或后台检查停采均锁存故障并停波。

DMA 通道、跨 ADC 步长及触发配置由 `c2000.syscfg` 生成。F280049 的 DMA 不能直接写 CLA 数据 RAM，CLA 也不能读 GS RAM，因此保留 CPU 的七路原始码提交；控制算法仍全部在 CLA。`AdcDma_Frames` 应持续递增，`AdcDma_Fault=0` 为正常。缓冲映射、故障码和时序说明见 [`ADC DMA 链路`](docs/adc_dma.md)。

## 校准与过压

`Codes/includes/config.h` 保存 V2 的七路线性校准系数。输入电流、底盘电流、母线电压及电容电压已有拟合；桥 A、桥 B 仍为理论值。电容电流已按本次实测 `I_CAP_Raw=1966 → 1 A` 修正偏置，斜率保留原理论值，其他电流范围仍待验证。旧 UID/有效标记均为 0，已删除无实际绑定作用的校准许可状态机。

输入电流的 CSV 通道名已由 `referee` 改为 `input_current`，对应 `I_IN/I_IN_Raw`，固件宏为 `SUPERCAP_CALIBRATION_INPUT_CURRENT`。历史表仍可读取。输入电流和底盘电流的历史数据没有零点与负向测量，当前实板电流准确性待复核；电容侧电流 `I_CAP` 的本次单点偏置校正见 [`calibration_data03`](tools/calibration/results/calibration_data03/report.md)。输入电流重新拟合后系数与原值相同，电压系数不变。

校准脚本、原始采集表、四路实测数据及可复现的拟合报告已整理到 [`tools/calibration`](tools/calibration/README.md)。本轮 `calibration_data02` 共 27 个实测/验证点，电容电压使用新增的 5 个拟合点和 1 个验证点，更新了固件中的 `SUPERCAP_CALIBRATION_CAP_VOLTAGE`；其余三路拟合系数不变。

输出过压保护已关闭：不再初始化或读取 CMPSS4，两桥所有硬件跳闸源及数字比较事件的直接关断动作均禁用。输出超过原约 29 V 门限不会因此停波。上电停波、使能变量手动启停、异常参数及 ADC 中断溢出停机仍保留，目标范围仍为大于 0 且不超过 28 V。LMG1210 内部欠压保护不由本固件控制。

`board.c/h` 由 `c2000.syscfg` 生成。PWM 周期、HRPWM、死区、相移、ADC/DMA/CLA 触发、CLA RAM 所有权、SCI 参数和关闭跳闸源的配置均在 SysConfig 中维护；应用只调用需要的初始化函数，处理 SFO、占空比和启停，不覆盖静态驱动参数。旧外设配置仍可在 SysConfig 中看到，但未启用。不要直接调用 `Board_init()` 替代这里的初始化顺序。

## 构建与测试

```sh
sh tools/build.sh
sh tests/run_host_checks.sh
```

默认使用 CCS 21.0.1、cl2000 25.11.1.LTS；C2000Ware 优先 26.02，本机未安装时回退到 26.01。本次在已安装的 26.01 上构建验证。可通过 `CCS_ROOT`、`C2000WARE_ROOT` 指定安装路径。输出为 `CPU1_FLASH/Supercap_V2.out`。CCS 工程仅保留 CPU1_FLASH，CCS 与脚本统一使用 O2；O0 版本超过当前 CLA 程序 RAM 容量，不可直接改回 O0。

主机测试使用 TI 原版 DCL，覆盖占空比分配、滞回、校准换算、启动取消、关闭过压及手动停波、异常输入及积分饱和恢复。电流环测试对照原 TI DCL PI/PID 方程，验证 CLA 不依赖 CPU 中断即可完成 HRPWM 控制、任务忙禁止启波、故障停波确认，以及实际反馈路径、前馈复位、简化对象阶跃跟踪、CC/CV 接管与积分恢复、固定反馈与单一使能启停。PWM 测试直接提取并执行 SysConfig 生成的初始化，覆盖 HRPWM 小数精度、互补边沿、同步延迟补偿，以及 SFO 首次/后台校准、失败与超时锁存。新增 DMA 测试执行生成的跨 ADC 步长，覆盖整帧屏障、七路映射、序号回卷、重新装载、CLA 未消费与 ADC/DMA 溢出停波。其余测试覆盖同源 CLA 算法、初始化及任务故障互锁、VOFA 固定四通道、小端帧、非阻塞 FIFO、队列顺序及溢出、串口线速模型与 1 ms 后台调度。寄存器、CLA 触发和 SFO 使用测试替身，不能代替实测波形、闭环稳定性、MEP 校准、CLA/控制中断耗时、串口实收和跳闸延迟。

本次未烧录或带电测试。本版没有输出过压或过流硬件关断，台架需外部限流和独立过压切断；启动浪涌、环路稳定性仍需验证。停止 PWM 不代表电容已放电。
