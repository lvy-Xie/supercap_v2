# V2 七路校准

物理量 = k × ADC + b。固件使用的七路系数保存在 [`Codes/includes/config.h`](../../Codes/includes/config.h)。

## 输入电流命名与校准状态

输入电流统一命名为 `input_current`，对应 `Electronic_Data.I_IN/I_IN_Raw`、`Raw[2]` 和 `SUPERCAP_CALIBRATION_INPUT_CURRENT`。SysConfig 的 SOC 名称为 `I_IN`，物理通道仍为 ADCA6，没有切换采样引脚。

当前可编辑采集表和 data01 实测表已改为新名称；data02 原始快照及历史报告保留旧名 `referee`。脚本兼容旧名，读取后统一为 `input_current`，新模板和新报告只输出新名。输入电流改名本身不改变数值；本轮电容电流偏置更新见下节。

输入电流与底盘电流使用历史实测拟合，但数据缺少零电流和负向电流点，且用户反馈当前电流不准，尚不能确认当前实板电流准确性。输入电流当前公式的零点约为 ADC=2007.61；若实际零电流时 `I_IN_Raw=2048`，将显示约 0.827 A，需要补充当前板的零点及稳定负载测量后重新拟合。桥 A、桥 B 电流仍使用理论系数。`I_CAP` 已做下面的单点偏置校正；已验证准确的电压系数保持不变。

## 本次电流参数：calibration_data03

输入电流使用当前 CSV 的 5 个拟合点和 2 个验证点重新计算，结果与原系数一致。电容侧仅有本次对话提供的 `I_CAP_Raw=1966`、仪表 `1 A`，因此保留斜率，只修正偏置：

```c
I_IN  =  0.02047242038f * I_IN_Raw  - 41.10068512f;
I_CAP = -0.02014652081f * I_CAP_Raw + 40.60805893f;
```

`I_CAP_Raw=1966` 经 float32 运算得到 `1.000000 A`。这是一组已知电流点的偏置校正，不是多点拟合；尚无实测零点或独立验证点，其他电流范围的准确性不能据此确认。

数据与依据保存在 [`data/calibration_data03_measured.csv`](data/calibration_data03_measured.csv)、[`原始表快照`](data/calibration_data03_original.csv)、[`报告`](results/calibration_data03/report.md) 和 [`参数 JSON`](results/calibration_data03/report.json)。原始表快照中的电容电流栏仍为空，补入的单点明确来自本次对话，没有制造额外样本。常规拟合脚本仍要求至少 3 个不同 fit 点，不能直接拿这份含单点的电流表做常规多点拟合。

## 电压及历史多点拟合：calibration_data02

本轮使用你在 `data/calibration_data01_original.csv` 中追加的数据。该文件继续作为可编辑采集表；每轮另存快照与结果，历史报告不覆盖。

- [`fit_calibration.py`](fit_calibration.py)：离线校准脚本，仅依赖 Python 3.9 及以上版本的标准库。
- [`data/calibration_data02_original.csv`](data/calibration_data02_original.csv)：本轮原始采集表的逐字节快照，共 49 行，保留未填写项。
- [`data/calibration_data02_measured.csv`](data/calibration_data02_measured.csv)：已填写的 27 行，保留点名、ADC、仪表值、单位及 fit/check 角色，可直接用于拟合。
- [`results/calibration_data02/report.html`](results/calibration_data02/report.html)：四路拟合及验证误差报告，附曲线和残差图。
- [`results/calibration_data02/report.json`](results/calibration_data02/report.json)、[`coefficients.txt`](results/calibration_data02/coefficients.txt) 和 [`points.csv`](results/calibration_data02/points.csv)：机器可读报告、系数宏、逐点预测与残差。

data02 的四路 float32 系数仍与当前固件一致；电容电压系数已由理论值更新为实测拟合值，其余三路系数不变：

| 通道 | 固件物理量 | k | b | 拟合/验证点数 | 验证最大绝对误差 |
| --- | --- | --- | --- | --- | --- |
| `input_current` 输入电流 | `I_IN` | 2.047242038e-02 | -4.110068512e+01 | 5 / 2 | 0.014133 A |
| `chassis` 底盘电流 | `I_OUT` | 2.034800500e-02 | -4.084916306e+01 | 5 / 2 | 0.026890 A |
| `input_voltage` 母线电压 | `VBUS` | 8.047959767e-03 | -3.887796402e-02 | 5 / 2 | 0.036950 V |
| `cap_voltage` 电容电压 | `VCAP` | 8.221379481e-03 | 5.106547382e-03 | 5 / 1 | 0.009006 V |

电容电压拟合范围为 5.09～24.50 V，5 个拟合点的最大绝对误差约 0.012362 V；27.12 V 验证点的预测值约 27.110994 V，误差约 -0.009006 V。该验证点位于拟合范围外，属于外推，目前只有这一个验证点。

data02 当时没有桥 A、桥 B、电容电流 `I_CAP` 的实测点；桥 A/B 继续保留理论系数，`I_CAP` 的新单点校正单独记录在 data03。原电流数据没有零点与负向测量；报告保留这些覆盖范围限制。

在工程根目录复现（结果目录必须尚不存在）：

```sh
python3 tools/calibration/fit_calibration.py tools/calibration/data/calibration_data02_measured.csv --output tmp/calibration_data02_recheck
python3 tests/test_calibration_fit.py
```

测试核对本轮原始快照与提取数据、已归档报告以及固件 float32 系数。含未填写项的原始表不能直接作为脚本输入；脚本不会将空白值当作 0。

## 历史校准：calibration_data01

[`data/calibration_data01_measured.csv`](data/calibration_data01_measured.csv) 和 [`results/calibration_data01/report.html`](results/calibration_data01/report.html) 保留首次归档的 21 个实测/验证点及三路拟合结果。最初采集表仍在 `tmp/calibration_data01.csv`；`data/calibration_data01_original.csv` 已由你继续填写，不再作为不可变的历史快照。

## 后续采集

在 CCS 展开 `SuperCap_Control.Electronic_Data`，可直接观察下列字段。`*_Raw` 是最新采样的 12 位 ADC 原始码（0～4095），每次 ADCC1 触发的 CLA Task 1 完成后更新，停波时也持续更新，未经系数换算或低通滤波，可直接填入 CSV 的 `adc_raw` 列。

| CSV 通道 | 原始码字段 | 物理量字段 | 兼容数组 |
| --- | --- | --- | --- |
| `bridge_a` | `I_A_Raw` | `I_A` | `Raw[0]` |
| `bridge_b` | `I_B_Raw` | `I_B` | `Raw[1]` |
| `input_current` | `I_IN_Raw` | `I_IN` | `Raw[2]` |
| `cap_current` | `I_CAP_Raw` | `I_CAP` | `Raw[3]` |
| `chassis` | `I_OUT_Raw` | `I_OUT` | `Raw[4]` |
| `input_voltage` | `VBUS_Raw` | `VBUS` | `Raw[5]` |
| `cap_voltage` | `VCAP_Raw` | `VCAP` | `Raw[6]` |

例如观察 `SuperCap_Control.Electronic_Data.VBUS_Raw`。`Raw[0..6]` 保留原顺序，与具名字段记录同一采样值，已有脚本和归档报告仍适用。

保持 `SuperCap_Control.Supercap_Enable_Flag = 0`，隔离栅极驱动后采集原始码和仪表值。停止开关不代表电容已放电。

```sh
python3 tools/calibration/fit_calibration.py --template tmp/calibration_board01.csv
python3 tools/calibration/fit_calibration.py tmp/calibration_board01.csv --output tmp/calibration_board01_result
python3 tests/test_calibration_fit.py
```

模板通道名、单位和拟合规则保持不变。每路至少三个拟合点，另留独立验证点；双向电流需要零点与正负方向验证。`coefficients.txt` 中的宏应手动替换配置文件的对应宏。

原 UID 和有效标记均为零，没有可沿用的板卡身份绑定。当前固件未启用 CMPSS 过压关断，ADC 校准也不会自动启用硬件保护。
