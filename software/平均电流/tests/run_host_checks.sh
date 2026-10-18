#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if [ -z "${C2000WARE_ROOT:-}" ]; then
    C2000WARE_ROOT=/Applications/ti/c2000/C2000Ware_26_02_00_00
    if [ ! -d "$C2000WARE_ROOT" ]; then
        C2000WARE_ROOT=/Applications/ti/c2000/C2000Ware_26_01_00_00
    fi
fi
test_build_dir=$(mktemp -d)
trap 'rm -rf "$test_build_dir"' EXIT
python3 tests/extract_pwm_init.py "$test_build_dir/generated_pwm_init.h"
# 使用 TI 原版 DCL；主机没有 C28x 中断指令，只有寄存器接口使用测试替身。
for mode in control current power cla_control diagnostics dma pwm_start hrpwm buzzer cla telemetry background; do
clang -std=c11 -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-cpp \
    -Wno-unused-but-set-variable -fsanitize=undefined,float-cast-overflow \
    -Itests/host -I"$test_build_dir" -I. -I"$C2000WARE_ROOT/libraries/control/DCL/c28/include" \
    "tests/host/${mode}_test.c" -o "$test_build_dir/${mode}_test"
"$test_build_dir/${mode}_test"
done
python3 tests/test_calibration_fit.py
python3 tests/test_startup_voice.py
python3 tests/check_project.py
