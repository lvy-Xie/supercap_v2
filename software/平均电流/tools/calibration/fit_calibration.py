#!/usr/bin/env python3
"""离线 ADC 校准：y = k*x + b。要求 Python 3.9 或更新版本，仅使用标准库。

不修改固件、不连接硬件，也不批准功率运行。
CSV 列为 channel,point,adc_raw,reference,unit,role；role 取 fit（拟合）或 check（验证）。
同一测量点的重复数据先求平均，再进行各点等权拟合。
"""
import argparse
import csv
from dataclasses import dataclass
from html import escape
import json
import math
from pathlib import Path
import statistics
import struct
import sys


# 与当前固件的宏、方向符号和合理性范围保持一致。
# 通道名对应：（宏后缀、CCS 原始值成员名、单位、预期斜率符号）。
CHANNELS = {
    "bridge_a": ("BRIDGE_A", "Raw[0]", "A", 1),
    "bridge_b": ("BRIDGE_B", "Raw[1]", "A", -1),
    "input_current": ("INPUT_CURRENT", "Raw[2]", "A", 1),
    "cap_current": ("CAP_CURRENT", "Raw[3]", "A", -1),
    "chassis": ("CHASSIS", "Raw[4]", "A", 1),
    "input_voltage": ("INPUT_VOLTAGE", "Raw[5]", "V", 1),
    "cap_voltage": ("CAP_VOLTAGE", "Raw[6]", "V", 1),
}
COLUMNS = ["channel", "point", "adc_raw", "reference", "unit", "role"]
# 历史归档保留旧名；读取时统一成明确的输入电流名称。
CHANNEL_ALIASES = {"referee": "input_current"}


@dataclass
class Point:
    name: str
    role: str
    x: float
    y: float
    samples: int = 1
    adc_std: float = 0.0


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def predict(k, b, x):
    # 近似模拟 CLA FPU32 分开的乘法和加法，不模拟硬件或执行时序。
    return f32(f32(f32(x) * k) + b)


def read_points(path):
    groups = {}
    with path.open(encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source, strict=True)
        if not reader.fieldnames or len(reader.fieldnames) != len(set(reader.fieldnames)):
            raise ValueError("CSV 表头为空或重复")
        if set(reader.fieldnames) != set(COLUMNS):
            raise ValueError("CSV 表头必须为：" + ",".join(COLUMNS))
        for row in reader:
            line = reader.line_num
            if None in row or any(v is None for v in row.values()):
                raise ValueError(f"第 {line} 行列数不正确")
            row = {k: v.strip() for k, v in row.items()}
            if not any(row.values()):
                continue
            name = CHANNEL_ALIASES.get(row["channel"], row["channel"])
            point, role = row["point"], row["role"]
            if name not in CHANNELS or not point or role not in ("fit", "check"):
                raise ValueError(f"第 {line} 行：未知通道、空 point 或 role 不是 fit/check")
            if row["unit"] != CHANNELS[name][2]:
                raise ValueError(f"第 {line} 行：{name} 必须使用 {CHANNELS[name][2]}，不要使用 mA/mV")
            try:
                x, y = float(row["adc_raw"]), float(row["reference"])
            except ValueError as error:
                raise ValueError(f"第 {line} 行：adc_raw/reference 必须填实测数字，不能留空") from error
            if not math.isfinite(x) or not math.isfinite(y):
                raise ValueError(f"第 {line} 行：不允许 NaN/Inf")
            if not 16.0 < x < 4079.0:
                raise ValueError(f"第 {line} 行：ADC 已越界或接近电源轨 (要求 16 < ADC < 4079)，请重测")
            # 仅作宽范围合理性检查，不代表建议的台架运行范围。
            if abs(y) > 1000.0 or (row["unit"] == "V" and y < 0.0):
                raise ValueError(f"第 {line} 行：参考物理量异常，请检查单位/接线")
            key = (name, point)
            group = groups.setdefault(key, {"role": role, "x": [], "y": []})
            if group["role"] != role:
                raise ValueError(f"{name}/{point} 同时用于 fit 和 check，请使用不同 point")
            group["x"].append(x)
            group["y"].append(y)
    result = {}
    for (name, point), group in groups.items():
        xs, ys = group["x"], group["y"]
        result.setdefault(name, []).append(Point(
            point, group["role"], statistics.fmean(xs), statistics.fmean(ys),
            len(xs), statistics.stdev(xs) if len(xs) > 1 else 0.0))
    if not result:
        raise ValueError("没有测量数据")
    return result


def metrics(points, k, b):
    if not points:
        return None
    errors = [predict(k, b, p.x) - p.y for p in points]
    mean_y = statistics.fmean(p.y for p in points)
    sst = math.fsum((p.y - mean_y)**2 for p in points)
    sse = math.fsum(e*e for e in errors)
    return {"points": len(points), "rmse": math.sqrt(sse / len(points)),
            "max_abs_error": max(abs(e) for e in errors),
            "mean_error": statistics.fmean(errors),
            "r_squared": 1.0 - sse / sst if sst > 0.0 else None}


def fit_channel(name, points):
    train = [p for p in points if p.role == "fit"]
    check = [p for p in points if p.role == "check"]
    if len(train) < 3 or len({p.x for p in train}) < 3 or len({p.y for p in train}) < 3:
        raise ValueError(f"{name}：至少需要 3 个不同 ADC 和参考值的 fit 稳态点（建议 5～9 个）")
    mx = statistics.fmean(p.x for p in train)
    my = statistics.fmean(p.y for p in train)
    sxx = math.fsum((p.x - mx)**2 for p in train)
    k64 = math.fsum((p.x - mx)*(p.y - my) for p in train) / sxx
    b64 = my - k64*mx
    k, b = f32(k64), f32(b64)
    suffix, raw, unit, sign = CHANNELS[name]
    low, high, max_b = (0.005, 0.050, 150.0) if unit == "A" else (0.002, 0.020, 10.0)
    if k*sign <= 0.0:
        raise ValueError(f"{name}：斜率符号与固件约定相反，检查实测电流正方向，不能直接反号绕过检查")
    if not f32(low) <= abs(k) <= f32(high) or abs(b) > max_b:
        raise ValueError(f"{name}：系数超出固件合理性范围，检查 ADC/单位/仪表；k={k}, b={b}")
    warnings = []
    if len(train) < 5:
        warnings.append("拟合点少于5个，建议补充覆盖范围的稳态点。")
    if not check:
        warnings.append("没有独立验证点，不能判定校准已通过。")
    if any(p.samples == 1 for p in points):
        warnings.append("存在单行数据：若不是已平均ADC，请采集多个稳态样本；单行均值无法估计噪声。")
    if unit == "A":
        if not any(p.y == 0.0 for p in train):
            warnings.append("缺少实测零电流点。")
        if not (min(p.y for p in train) < 0.0 < max(p.y for p in train)):
            warnings.append("未覆盖正负电流，反向校准尚未验证。")
    x_min, x_max = min(p.x for p in train), max(p.x for p in train)
    if any(not x_min <= p.x <= x_max for p in check):
        warnings.append("部分验证点在拟合ADC范围外，属于外推。")
    if any(p.x in {q.x for q in train} or p.y in {q.y for q in train} for p in check):
        warnings.append("验证点与拟合点的ADC或参考值重合，请确认是独立重测而非复制数据。")
    macro = f"#define SUPERCAP_CALIBRATION_{suffix} {{ {k:.9e}F, {b:.9e}F }}"
    return {"channel": name, "unit": unit, "raw_field": raw,
            "slope_float32": k, "intercept_float32": b,
            "slope_fit_double": k64, "intercept_fit_double": b64,
            "zero_value_adc": -b/k, "fit_adc_range": [x_min, x_max],
            "fit_reference_range": [min(p.y for p in train), max(p.y for p in train)],
            "fit": metrics(train, k, b), "check": metrics(check, k, b),
            "warnings": warnings, "macro": macro}


def svg_plot(points, result):
    """无第三方依赖的矢量图，包含按真实数值缩放的坐标轴和残差。"""
    k, b = result["slope_float32"], result["intercept_float32"]
    xs = [p.x for p in points]
    ys = [p.y for p in points] + [predict(k, b, x) for x in xs]
    errors = [predict(k, b, p.x) - p.y for p in points]
    def bounds(values):
        lo, hi = min(values), max(values)
        pad = (hi-lo)*0.08 if hi > lo else max(abs(lo)*0.08, 0.001)
        return lo-pad, hi+pad
    xmin, xmax = bounds(xs)
    ymin, ymax = bounds(ys)
    emin, emax = bounds(errors + [0.0])
    sx = lambda x: 90 + (x-xmin)/(xmax-xmin)*740
    sy = lambda y: 350 - (y-ymin)/(ymax-ymin)*235
    se = lambda e: 600 - (e-emin)/(emax-emin)*150
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="900" height="670" viewBox="0 0 900 670">',
             '<rect width="900" height="670" fill="white"/>',
             '<g font-family="Arial,sans-serif" font-size="13" fill="#263244">']
    def text(x, y, value, anchor="start", size=13):
        parts.append(f'<text x="{x}" y="{y}" text-anchor="{anchor}" font-size="{size}">{escape(str(value))}</text>')
    def line(x1, y1, x2, y2, color="#dce1e8"):
        parts.append(f'<line x1="{x1:.3f}" y1="{y1:.3f}" x2="{x2:.3f}" y2="{y2:.3f}" stroke="{color}"/>')
    text(40, 30, result["channel"] + " calibration", size=22)
    text(40, 55, f"y = {k:.9g} * ADC + ({b:.9g}) [{result['unit']}]")
    text(40, 78, "Blue: fit points    Orange: independent check points    Line: float32 coefficients")
    for i in range(6):
        x = xmin + i*(xmax-xmin)/5
        y = ymin + i*(ymax-ymin)/5
        e = emin + i*(emax-emin)/5
        for top, bottom in ((115, 350), (450, 600)):
            line(sx(x), top, sx(x), bottom)
        text(sx(x), 375, f"{x:.5g}", "middle")
        text(sx(x), 625, f"{x:.5g}", "middle")
        line(90, sy(y), 830, sy(y)); text(80, sy(y)+4, f"{y:.5g}", "end")
        line(90, se(e), 830, se(e)); text(80, se(e)+4, f"{e:.3g}", "end")
    text(90, 106, f"Reference [{result['unit']}]")
    text(460, 400, "ADC mean [counts]", "middle")
    text(90, 433, f"Residual = predicted - reference [{result['unit']}]")
    text(460, 650, "ADC mean [counts]", "middle")
    line(90, se(0), 830, se(0), "#263244")
    lo, hi = min(xs), max(xs)
    line(sx(lo), sy(predict(k, b, lo)), sx(hi), sy(predict(k, b, hi)), "#2563eb")
    for p, e in zip(points, errors):
        color = "#2563eb" if p.role == "fit" else "#d97706"
        label = escape(f"{p.name}: ADC={p.x:.6g}, reference={p.y:.6g}, n={p.samples}, error={e:.6g}")
        for y in (sy(p.y), se(e)):
            parts.append(f'<circle cx="{sx(p.x):.3f}" cy="{y:.3f}" r="4" fill="{color}"><title>{label}</title></circle>')
    parts.append('</g></svg>')
    return "\n".join(parts)


def write_template(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("x", encoding="utf-8", newline="") as target:
        writer = csv.writer(target)
        writer.writerow(COLUMNS)
        for name, (_, _, unit, _) in CHANNELS.items():
            for i in range(1, 8):
                writer.writerow([name, f"p{i}", "", "", unit, "fit" if i <= 5 else "check"])


def demo_points():
    # 仅为合成测试数据，不是真实板卡测量结果。
    result = {}
    for name, (_, _, unit, sign) in CHANNELS.items():
        refs = [-2, -1, 0, 1, 2, -0.5, 0.5] if unit == "A" else [5, 10, 15, 20, 25, 12, 22]
        k, b = (sign*0.0202, -sign*0.0202*2046) if unit == "A" else (0.0081, -0.03)
        result[name] = [Point(f"p{i+1}", "fit" if i < 5 else "check",
                              (y-b)/k + (i % 3 - 1)*0.2, y, 1024, 0.8)
                        for i, y in enumerate(refs)]
    return result


def write_results(data, results, output, source, demo=False):
    # 拒绝覆盖已有测量或报告；请选用新目录。
    output.mkdir(parents=True, exist_ok=False)
    report = {"source": source, "synthetic_demo": demo,
              "model": "physical = k * ADC_raw + b",
              "method": "unweighted OLS of point means; exported coefficients rounded to float32",
              "missing_channels": sorted(set(CHANNELS) - set(data)),
              "channels": results}
    (output / "report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False)+"\n", encoding="utf-8")
    banner = "DEMO ONLY - DO NOT USE ON HARDWARE" if demo else "CANDIDATE COEFFICIENTS - verify independently before use"
    macros = ["/* 校准候选结果需独立验证，不要直接包含本文件。 */",
              "/* 将对应宏手动填回 Codes/includes/config.h。 */",
              "/* 本工具不会修改功率级启停状态。 */"]
    body = [f"<h1>{escape(banner)}</h1>",
            f"<p>Source: {escape(source)}</p>",
            "<p>Model: physical = k * ADC + b. Residual = predicted - reference. Each point has equal weight after averaging.</p>",
            "<p>拟合结果不是校准批准。请核对测量范围、方向、独立验证误差和仪表精度；不要仅凭 R² 解锁功率级。</p>"]
    if report["missing_channels"]:
        body.append("<p>Missing channels: " + escape(", ".join(report["missing_channels"])) + "</p>")
    for name, r in results.items():
        macros.append(r["macro"])
        (output / f"{name}.svg").write_text(svg_plot(data[name], r), encoding="utf-8")
        body.extend([f"<h2>{name}</h2>", f"<pre>{escape(r['macro'])}</pre>",
                     f"<p>Fit ADC range: {r['fit_adc_range']}; zero-value ADC: {r['zero_value_adc']:.6g}</p>",
                     "<table><tr><th>Dataset</th><th>Points</th><th>RMSE</th><th>Max |error|</th><th>R²</th></tr>"])
        for role in ("fit", "check"):
            m = r[role]
            if m:
                r2 = "n/a" if m['r_squared'] is None else f"{m['r_squared']:.9g}"
                body.append(f"<tr><td>{role}</td><td>{m['points']}</td><td>{m['rmse']:.6g} {r['unit']}</td><td>{m['max_abs_error']:.6g} {r['unit']}</td><td>{r2}</td></tr>")
        body.append("</table>")
        body.extend(f"<p class='warning'>{escape(w)}</p>" for w in r["warnings"])
        body.append(f'<img src="{name}.svg" alt="{name} fit and residuals" width="900">')
    (output / "coefficients.txt").write_text("\n".join(macros)+"\n", encoding="utf-8")
    with (output / "points.csv").open("w", encoding="utf-8", newline="") as target:
        writer = csv.writer(target)
        writer.writerow(["channel", "point", "role", "unit", "samples", "adc_mean", "adc_std", "reference_mean", "predicted", "error"])
        for name, points in data.items():
            r = results[name]
            for p in points:
                y = predict(r["slope_float32"], r["intercept_float32"], p.x)
                writer.writerow([name, p.name, p.role, r["unit"], p.samples, p.x,
                                 p.adc_std if p.samples > 1 else "", p.y, y, y-p.y])
    html = ('<!doctype html><html lang="zh-CN"><meta charset="utf-8"><title>Supercap calibration</title>'
            '<style>body{font:16px system-ui;max-width:980px;margin:32px auto;padding:0 20px;color:#263244}'
            'table{border-collapse:collapse}td,th{padding:8px 14px;border-bottom:1px solid #ddd;text-align:right}'
            'pre{white-space:pre-wrap}img{max-width:100%;height:auto}.warning{color:#92400e}</style><body>'
            + "\n".join(body) + '</body></html>')
    (output / "report.html").write_text(html, encoding="utf-8")


def main(argv=None):
    parser = argparse.ArgumentParser(description="七路ADC线性校准拟合；离线工具，不修改固件。")
    parser.add_argument("input", nargs="?", type=Path, help="实测CSV文件")
    parser.add_argument("--template", type=Path, help="生成空白CSV采集模板后退出（不覆盖已有文件）")
    parser.add_argument("--demo", action="store_true", help="用明确标记的模拟数据演示，禁止用于硬件")
    parser.add_argument("--output", type=Path, help="结果目录，必须不存在（避免覆盖历史校准）")
    args = parser.parse_args(argv)
    if sum([args.input is not None, args.template is not None, args.demo]) != 1:
        parser.error("请选择一种模式：输入CSV、--template 或 --demo")
    try:
        if args.template:
            if args.output:
                parser.error("--template 不使用 --output")
            write_template(args.template)
            print(f"空白模板：{args.template}\n填写实测数据；未采集的通道请整组删除，不能留空参与拟合。")
            return 0
        if args.output is None:
            parser.error("拟合或演示时必须指定 --output 新目录")
        data = demo_points() if args.demo else read_points(args.input)
        results = {name: fit_channel(name, data[name]) for name in CHANNELS if name in data}
        source = "SYNTHETIC DEMO - NOT BOARD MEASUREMENTS" if args.demo else str(args.input.resolve())
        write_results(data, results, args.output, source, args.demo)
        if args.demo:
            print("DEMO ONLY：以下是模拟数据的演示系数，禁止用于硬件。")
        missing = sorted(set(CHANNELS) - set(data))
        if missing:
            print("尚未提供这些通道：" + ", ".join(missing))
        for name, r in results.items():
            print(r["macro"])
            print(f"  fit max error={r['fit']['max_abs_error']:.6g} {r['unit']}; check={r['check']}")
            for warning in r["warnings"]:
                print("  注意：" + warning)
        print(f"报告：{args.output / 'report.html'}\n未修改固件、UID、校准有效标记或功率级开关。")
        return 0
    except (ValueError, OSError, csv.Error, OverflowError) as error:
        print(f"错误：{error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
