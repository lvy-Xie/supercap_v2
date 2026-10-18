# 功率环

`Control_Loop()` 根据 `Control_Mode` 选择两个独立入口：0 为原 `Test_Charge_Loop()` 恒流充电，1 为新增 `Power_Control_Loop()`。默认仍为 0，上电停波。两者共用电流 PI、电压 PID 和 PWM 更新函数；功率模式不读取 `set_current`。

功率模式的数据流为：`set_power − VBUS×I_IN → Referee_Power_Loop → target_DCDC_Power / VCAP → target_cap_current → I_CAP 电流 PI → 电压限幅 → PWM`。`target_DCDC_Power` 沿用参考命名，是 PID 输出的功率控制指令，并非实测 DCDC 功率。不做效率换算，实际损耗引起的输入功率偏差由外层积分修正。电流目标为正时充电，为负时放电；`I_DCDC` 仅用于观测。

功率 PID 初值沿用 Reborn：`Kp=0.5、Ki=0.01、Kd=1.0`。控制调用频率仍约 49.95 kHz，增益需要在 V2 实板整定。

动态限幅沿用参考工程的电流、电压范围，去掉效率系数：正向功率上限为 `min(15×VBUS, 充电电流限×VCAP)`，负向为 `−min(15×VBUS, 放电电流限×VCAP)`。电容低于 5 V 时充电限为 4 A，其余为 15 A；放电限在 5 V 以下为 0.1 A，5～12 V 线性增加到 12 A，12 V 以上为 12 A。`VCAP=0` 时电流目标为 0，避免除零。这些是软件目标限幅。

在 CCS Watch 中操作：

1. 写 `SuperCap_Control.Supercap_Enable_Flag=0`，等 `PWM_Running=0`，并让采样继续运行以复位积分。
2. 写 `Control_Mode=1`、`set_power`（单位 W，默认 30）、`set_voltage`（电容电压上限，≤28 V）。模式及增益均在停波时修改。
3. 写 `SuperCap_Control.Supercap_Enable_Flag=1`，仍经过原有采样准备后启动。
4. 观察 `power_feedback`、`target_DCDC_Power`、`target_cap_current`、`current_feedback`（始终为 `I_CAP`）及 `Current_VoltageLimited`。
5. 切回恒流充电时，停波并写 `Control_Mode=0`，目标改由 `set_current` 提供。

停波时功率 PID 清零、电流积分跟踪 `VCAP/VBUS`；电压限幅沿用原双环选小逻辑。参数或反馈无效时 CLA 强制 OST，再由 CPU 确认停波。VOFA 四路通道保持原配置，功率环变量通过 Watch 观察。

主机回归验证有损耗的简化双环对象在充放电间切换时仍能跟踪输入功率，以及限幅、复位、异常参数和实际 CLA 采样路径。该模型不代表真实功率级稳定性，尚未烧录验证。
