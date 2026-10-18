"""离线测试：不修改固件，不访问硬件。"""
import csv
import importlib.util
import json
import re
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "tools/calibration/fit_calibration.py"
spec = importlib.util.spec_from_file_location("calibration_fit", SCRIPT)
fit = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = fit
spec.loader.exec_module(fit)


class CalibrationTests(unittest.TestCase):
    def test_latest_current_parameters_are_used_by_firmware(self):
        folder = ROOT / "tools/calibration"
        report = json.loads((folder / "results/calibration_data03/report.json").read_text())
        data = fit.read_points(folder / "data/calibration_data03_measured.csv")
        config = (ROOT / "Codes/includes/config.h").read_text()
        self.assertEqual(fit.fit_channel("input_current", data["input_current"]),
                         report["channels"]["input_current"])
        cap = report["channels"]["cap_current"]
        self.assertEqual(len(data["cap_current"]), 1)
        self.assertEqual(data["cap_current"][0].x, 1966)
        self.assertEqual(data["cap_current"][0].y, 1)
        self.assertEqual(cap["slope_float32"], cap["previous_slope_float32"])
        self.assertEqual(cap["independent_check_points"], 0)
        self.assertEqual(fit.predict(cap["slope_float32"], cap["intercept_float32"], 1966), 1)
        # 单点偏置校正不能绕过常规脚本的多点拟合要求。
        with self.assertRaises(ValueError):
            fit.fit_channel("cap_current", data["cap_current"])
        for name, result in report["channels"].items():
            macro = "SUPERCAP_CALIBRATION_" + fit.CHANNELS[name][0]
            match = re.search(r"^#define\s+" + macro + r"\s+\{\s*([^,]+),\s*([^}]+)\}",
                              config, re.M)
            self.assertIsNotNone(match, macro)
            firmware = [fit.f32(float(v.strip().rstrip("Ff"))) for v in match.groups()]
            self.assertEqual(firmware, [result["slope_float32"], result["intercept_float32"]])

    def test_archived_measurements_reproduce_firmware(self):
        folder = ROOT / "tools/calibration"
        with (folder / "data/calibration_data02_original.csv").open(newline="") as source:
            original = list(csv.DictReader(source))
        with (folder / "data/calibration_data02_measured.csv").open(newline="") as source:
            measured = list(csv.DictReader(source))
        self.assertEqual(len(original), 49)
        self.assertEqual(len(measured), 27)
        self.assertEqual(measured, [r for r in original if r["adc_raw"] and r["reference"]])
        data = fit.read_points(folder / "data/calibration_data02_measured.csv")
        self.assertEqual(set(data), {"input_current", "chassis", "input_voltage", "cap_voltage"})
        config = (ROOT / "Codes/includes/config.h").read_text()
        report = json.loads((folder / "results/calibration_data02/report.json").read_text())
        self.assertFalse(report["synthetic_demo"])
        self.assertEqual(set(report["missing_channels"]),
                         {"bridge_a", "bridge_b", "cap_current"})
        for name, points in data.items():
            result = fit.fit_channel(name, points)
            archived_name = "referee" if name == "input_current" else name
            archived = dict(report["channels"][archived_name])
            archived["channel"] = name
            archived["macro"] = archived["macro"].replace("CALIBRATION_REFEREE", "CALIBRATION_INPUT_CURRENT")
            self.assertEqual(result, archived)
            self.assertEqual(result["fit"]["points"], 5)
            self.assertEqual(result["check"]["points"], 1 if name == "cap_voltage" else 2)
            macro = "SUPERCAP_CALIBRATION_" + fit.CHANNELS[name][0]
            match = re.search(r"^#define\s+" + macro + r"\s+\{\s*([^,]+),\s*([^}]+)\}",
                              config, re.M)
            self.assertIsNotNone(match, macro)
            firmware = [fit.f32(float(v.strip().rstrip("Ff"))) for v in match.groups()]
            self.assertEqual(firmware, [result["slope_float32"], result["intercept_float32"]])

    def test_known_positive_and_negative_lines(self):
        for name, (_, _, unit, sign) in fit.CHANNELS.items():
            k, b = (sign*0.02, -sign*40) if unit == "A" else (0.008, -0.1)
            points = [fit.Point(str(i), "fit", x, k*x+b)
                      for i, x in enumerate([1800, 1900, 2000, 2100, 2200])]
            result = fit.fit_channel(name, points)
            self.assertAlmostEqual(result["slope_float32"], k, places=8)
            self.assertAlmostEqual(result["intercept_float32"], b, places=6)
            self.assertLess(result["fit"]["max_abs_error"], 0.00001)
            self.assertIsNone(result["check"])
            self.assertNotIn("VALID_MARKER", result["macro"])
            ET.fromstring(fit.svg_plot(points, result))

    def test_check_points_never_change_fit(self):
        points = [fit.Point(str(x), "fit", x, 0.008*x) for x in (500, 1000, 1500)]
        before = fit.fit_channel("input_voltage", points)
        points.append(fit.Point("independent", "check", 1100, 10.0))
        after = fit.fit_channel("input_voltage", points)
        self.assertEqual(before["macro"], after["macro"])
        self.assertAlmostEqual(after["check"]["max_abs_error"], 1.2, places=5)
        self.assertIsNone(after["check"]["r_squared"])

    def test_bad_fit_rejected(self):
        cases = [
            [fit.Point(str(i), "fit", 2048, float(i)) for i in range(3)],
            [fit.Point(str(i), "fit", 2000+i*50, 0) for i in range(3)],
            [fit.Point(str(i), "fit", 2000+i*50, -i) for i in range(3)],
            [fit.Point(str(i), "fit", 2000+i, float(i)) for i in range(3)],
            [fit.Point(str(i), "fit", 2000+i*50, float(i)) for i in range(2)],
        ]
        for points in cases:
            with self.subTest(points=points), self.assertRaises(ValueError):
                fit.fit_channel("bridge_a", points)

    def write_csv(self, path, rows):
        with path.open("w", encoding="utf-8-sig", newline="") as target:
            writer = csv.writer(target)
            writer.writerow(fit.COLUMNS)
            writer.writerows(rows)

    def test_average_repeats_and_preserve_roles(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "samples.csv"
            self.write_csv(path, [
                ["input_voltage", "p1", 999, 8.0, "V", "fit"],
                ["input_voltage", "p1", 1001, 8.0, "V", "fit"],
                ["input_voltage", "p2", 1500, 12.0, "V", "check"]])
            points = fit.read_points(path)["input_voltage"]
            self.assertEqual(points[0].samples, 2)
            self.assertEqual(points[0].x, 1000)
            self.assertEqual(points[1].role, "check")

    def test_input_current_legacy_alias_and_template(self):
        with tempfile.TemporaryDirectory() as folder:
            source = Path(folder) / "input_current.csv"
            self.write_csv(source, [
                ["referee", "p1", 2058, 1.027, "A", "fit"],
                ["input_current", "p1", 2060, 1.027, "A", "fit"]])
            data = fit.read_points(source)
            self.assertEqual(set(data), {"input_current"})
            self.assertEqual(data["input_current"][0].x, 2059)
            self.assertEqual(data["input_current"][0].samples, 2)
            template = Path(folder) / "template.csv"
            fit.write_template(template)
            self.assertIn("input_current,", template.read_text())
            self.assertNotIn("referee,", template.read_text())

    def test_bad_csv_rejected(self):
        bad_rows = [
            ["unknown", "p1", 1000, 8, "V", "fit"],
            ["input_voltage", "p1", "NaN", 8, "V", "fit"],
            ["input_voltage", "p1", 1000, "inf", "V", "fit"],
            ["input_voltage", "p1", 4095, 8, "V", "fit"],
            ["input_voltage", "p1", 0, 8, "V", "fit"],
            ["input_voltage", "p1", 1000, 8000, "mV", "fit"],
            ["input_voltage", "p1", "", "", "V", "fit"],
            ["input_voltage", "p1", 1000, 8, "V", "wrong_role"],
        ]
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "samples.csv"
            for row in bad_rows:
                with self.subTest(row=row), self.assertRaises(ValueError):
                    self.write_csv(path, [row])
                    fit.read_points(path)
            with self.assertRaises(ValueError):
                self.write_csv(path, [["input_voltage", "p1", 1000, 8, "V", role]
                                      for role in ("fit", "check")])
                fit.read_points(path)

    def test_cli_end_to_end_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as folder:
            template = Path(folder) / "measurements.csv"
            output = Path(folder) / "results"
            run = lambda *args: subprocess.run([sys.executable, str(SCRIPT), *map(str, args)],
                                               capture_output=True, text=True)
            self.assertEqual(run("--template", template).returncode, 0)
            original = template.read_bytes()
            self.assertEqual(run("--template", template).returncode, 2)
            self.assertEqual(template.read_bytes(), original)
            self.assertEqual(run(template, "--output", output).returncode, 2)  # Blank isn't zero.
            self.assertFalse(output.exists())
            rows = [["input_voltage", f"p{i}", x, 0.008*x, "V", "fit"]
                    for i, x in enumerate([500, 1000, 1500, 2000, 2500])]
            rows.append(["input_voltage", "verify", 1250, 10.0, "V", "check"])
            self.write_csv(template, rows)
            result = run(template, "--output", output)
            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads((output / "report.json").read_text())
            self.assertFalse(report["synthetic_demo"])
            self.assertEqual(len(report["missing_channels"]), 6)
            self.assertIn("SUPERCAP_CALIBRATION_INPUT_VOLTAGE", (output / "coefficients.txt").read_text())
            self.assertEqual(run(template, "--output", output).returncode, 2)
            demo = Path(folder) / "demo"
            result = run("--demo", "--output", demo)
            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads((demo / "report.json").read_text())
            self.assertTrue(report["synthetic_demo"])
            self.assertEqual(len(report["channels"]), 7)
            self.assertEqual(len(list(demo.glob("*.svg"))), 7)


if __name__ == "__main__":
    unittest.main()
