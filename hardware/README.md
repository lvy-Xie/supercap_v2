# 硬件资料 · Supercap V2.1

超级电容控制器 V2.1 的完整硬件设计归档：原理图、PCB 工程源文件、生产用 Gerber、物料清单与 3D 结构模型。

- **版本**：V2.1
- **导出日期**：2026-10-25
- **EDA 工具**：嘉立创 EDA 专业版（工程文件 `.epro2`）
- **板层**：4 层（顶层 / 底层 + 2 内层）

> 本目录只存放对外发布的硬件资料。设计源文件的编辑请基于 `pcb/` 下的 `.epro2` 工程，不要直接改动 Gerber。

---

## 目录结构

```text
hardware/
├── README.md                                  # 本说明
├── schematic/                                 # 原理图
│   ├── SCH_V2.1_2026-10-25.pdf                # 原理图 PDF（可直接查看）
│   └── supercap_v2_schematic_pdf.zip          # 原理图 PDF 原始打包（含同名 PDF）
├── pcb/                                       # PCB 设计源文件与生产文件
│   ├── supercap_V2.1_2026-10-25.epro2         # 嘉立创 EDA 专业版工程（原理图 + PCB 源文件）
│   └── Gerber_super_v2.zip                    # Gerber + 钻孔 + 飞针测试文件，可直接下单
├── bom/                                       # 物料清单
│   └── BOM_supercap_V2.1.xlsx                 # 73 项物料，立创商城编号，可直接配单
└── mechanical/                                # 结构 / 装配
    └── supercap_v2.step.gz                    # 3D 模型（STEP AP214，gzip 压缩，解压后 43 MB）
```

---

## 文件说明

| 文件 | 大小 | 内容 | 打开方式 |
|---|---|---|---|
| `schematic/SCH_V2.1_2026-10-25.pdf` | 727 KB | V2.1 原理图，单页 PDF | 任意 PDF 阅读器 / 浏览器 |
| `schematic/supercap_v2_schematic_pdf.zip` | 727 KB | 原理图 PDF 的原始打包，内容与上者一致 | 解压后查看 |
| `pcb/supercap_V2.1_2026-10-25.epro2` | 804 KB | 嘉立创 EDA 专业版工程（含原理图、PCB、层叠设置、预览图） | 嘉立创 EDA 专业版客户端「打开工程」 |
| `pcb/Gerber_super_v2.zip` | 446 KB | 4 层 Gerber、钻孔、飞针测试 JSON、板厂加工说明 | 直接上传板厂；本地可用 KiCad Gerber Viewer / 嘉立创 EDA 预览 |
| `bom/BOM_supercap_V2.1.xlsx` | 12 KB | 物料清单，73 项，含位号 / 封装 / 型号 / 立创编号 | Excel / Numbers / WPS |
| `mechanical/supercap_v2.step.gz` | 8.1 MB | 整板 3D 模型（STEP AP214，gzip 压缩，解压后 43 MB） | 解压后导入 SolidWorks / Fusion 360 / FreeCAD / Creo |

### Gerber 包内清单（17 个文件）

| 类别 | 文件 |
|---|---|
| 顶层 / 底层 | `Gerber_TopLayer.GTL`、`Gerber_BottomLayer.GBL` |
| 内层 | `Gerber_InnerLayer1.G1`、`Gerber_InnerLayer2.G2` |
| 丝印 | `Gerber_TopSilkscreenLayer.GTO`、`Gerber_BottomSilkscreenLayer.GBO` |
| 阻焊 | `Gerber_TopSolderMaskLayer.GTS`、`Gerber_BottomSolderMaskLayer.GBS` |
| 钢网 | `Gerber_TopPasteMaskLayer.GTP`、`Gerber_BottomPasteMaskLayer.GBP` |
| 外形 / 文档 | `Gerber_BoardOutlineLayer.GKO`、`Gerber_DocumentLayer.GDL`、`Gerber_DrillDrawingLayer.GDD` |
| 钻孔 | `Drill_PTH_Through.DRL`、`Drill_PTH_Through_Via.DRL` |
| 其它 | `FlyingProbeTesting.json`（飞针测试）、板厂加工说明 `PCB工艺参数+阻抗.txt` |

### BOM 字段

`No.` / `Quantity` / `Comment` / `Designator` / `Footprint` / `Value` / `Manufacturer Part` / `Manufacturer` / `Supplier Part` / `Supplier`

- 共 **73 项**物料，供应商统一为 **LCSC（立创商城）**，`Supplier Part` 即立创编号（如 `C255316`），可直接粘贴到立创商城「BOM 配单」。
- 表中已含 `Designator` 位号与 `Footprint` 封装，贴片厂可直接使用。

---

## 使用流程

### 1）打样 / 小批量生产

1. 取 `pcb/Gerber_super_v2.zip`，**不要解压**，直接上传嘉立创（或其它板厂）下单页；
2. 板厂参数按包内 `PCB工艺参数+阻抗.txt` 填写（4 层、阻抗控制）；
3. `bom/BOM_supercap_V2.1.xlsx` 上传立创商城「BOM 配单」自动匹配料号；
4. 贴片时提供 Gerber 包的钢网层（`GTP` / `GBP`）与 BOM 位号表。

### 2）改板 / 二次开发

1. 用**嘉立创 EDA 专业版**打开 `pcb/supercap_V2.1_2026-10-25.epro2`；
2. 修改后重新导出 Gerber 与 BOM，文件名沿用 `V<版本>_<日期>` 规则；
3. 同步更新本 README 的版本信息与校验和。

### 3）结构与装配

STEP 模型以 gzip 压缩存放，使用前先解压（`-k` 表示保留压缩包）：

```bash
cd hardware/mechanical
gunzip -k supercap_v2.step.gz          # 得到 supercap_v2.step（43 MB）

# FreeCAD 命令行查看（可选）
freecad supercap_v2.step
```

将 STEP 导入整机装配体，用于检查与电池仓、主控板的干涉。用完可删掉解压出的 `.step`，仓库里只保留 `.gz`。

---

## 文件校验

```bash
cd hardware
shasum -a 256 \
  schematic/SCH_V2.1_2026-10-25.pdf \
  bom/BOM_supercap_V2.1.xlsx \
  pcb/Gerber_super_v2.zip \
  pcb/supercap_V2.1_2026-10-25.epro2 \
  mechanical/supercap_v2.step.gz
```

| 文件 | SHA256 |
|---|---|
| `schematic/SCH_V2.1_2026-10-25.pdf` | `1cfa3ebae9289271e56a4fc1b5db46071d5bf363e4d693535527dfe20e60032c` |
| `bom/BOM_supercap_V2.1.xlsx` | `2c49a71b801e409183de2f31508b2cbab81dd95129cc1ef90373bcc702360d90` |
| `pcb/Gerber_super_v2.zip` | `fd9965628a0060dc2f68e25ff5d0008d1471a4f8d80eb836fa4a496b9f42045a` |
| `pcb/supercap_V2.1_2026-10-25.epro2` | `2f8b62ca4aea00f736f1678aed451a6196ea6fa898bb3aba25307cd74c0e8898` |
| `mechanical/supercap_v2.step.gz` | `30fcbdb2d0b236c2a20f068a2202ce73bd641f808a893b5a262dcd030706e648` |

---

## 注意事项

1. **STEP 已 gzip 压缩存储**（原 43 MB → 8.1 MB），仓库体积因此可控。使用前解压：`gunzip -k hardware/mechanical/supercap_v2.step.gz`，用完可删掉解压出的 `.step`；重新压缩用 `gzip -9 -k supercap_v2.step`。校验和对应的是 `.gz` 文件。
2. **`.epro2` 本身是压缩包格式**（内部为 `project2.json` + `.epru` 文档 + 预览图），不要用文本编辑器修改，也不要解压后单独提交内部文件。
3. **Gerber 与 BOM 版本必须对应**：本目录内所有文件均为 V2.1 / 2026-10-25 同一次导出，混用不同版本会导致贴片错件。
4. 原理图 `schematic/` 下同时保留了 PDF 与原始 zip，内容一致，任选其一使用。

---

## 更新记录

| 日期 | 版本 | 内容 |
|---|---|---|
| 2026-10-25 | V2.1 | 首次归档硬件资料：原理图 PDF、嘉立创 EDA 工程、Gerber、BOM（73 项）、STEP 3D 模型 |

---

## 后续补充

- 关键电路参数与设计说明（功率级拓扑、采样与保护阈值）。
- 实测波形、效率曲线与热测试结果。
- 与 `software/` 中平均电流 / 峰值电流固件的引脚与信号对应表。
