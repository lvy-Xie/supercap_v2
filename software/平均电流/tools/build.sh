#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
# 可用环境变量指定另一套已安装的 TI 工具。
if [ -z "${C2000WARE_ROOT:-}" ]; then
    C2000WARE_ROOT=/Applications/ti/c2000/C2000Ware_26_02_00_00
    if [ ! -d "$C2000WARE_ROOT" ]; then
        C2000WARE_ROOT=/Applications/ti/c2000/C2000Ware_26_01_00_00
    fi
fi
CCS_ROOT=${CCS_ROOT:-/Applications/ti/ccs2101/ccs}
compiler="$CCS_ROOT/tools/compiler/ti-cgt-c2000_25.11.1.LTS/bin/cl2000"
mkdir -p CPU1_FLASH/obj
rm -f CPU1_FLASH/Supercap_V2.out CPU1_FLASH/obj/*.obj
"$CCS_ROOT/utils/sysconfig_1.28.1/sysconfig_cli.sh" \
    -s "$C2000WARE_ROOT/.metadata/sdk.json" -d F28004x \
    -p F28004x_64PM -r F28004x_64PM --script c2000.syscfg \
    -o CPU1_FLASH/syscfg --compiler ccs
for source in main.c Codes/control.c Codes/supercap.c Codes/buzzer.c Codes/cla_adc.c Codes/cla_tasks.cla Codes/telemetry.c device/device.c \
    device/f28004x_codestartbranch.asm CPU1_FLASH/syscfg/board.c; do
    "$compiler" -v28 -ml -mt --float_support=fpu32 --tmu_support=tmu0 \
        --vcu_support=vcu0 --cla_support=cla2 --cla_background_task=off --cla_signed_compare_workaround=on -O2 --abi=eabi --define=_FLASH --define=DEBUG \
        --gen_func_subsections=on --display_error_number \
        --include_path=. --include_path=device --include_path=CPU1_FLASH/syscfg \
        --include_path="$C2000WARE_ROOT/driverlib/f28004x/driverlib" \
        --include_path="$C2000WARE_ROOT/libraries/control/DCL/c28/include" \
        --include_path="$C2000WARE_ROOT/libraries/calibration/hrpwm/f28004x/include" \
        --include_path="$CCS_ROOT/tools/compiler/ti-cgt-c2000_25.11.1.LTS/include" \
        --obj_directory=CPU1_FLASH/obj --compile_only "$source"
done
"$compiler" -v28 -ml --float_support=fpu32 --abi=eabi --run_linker \
    --rom_model --entry_point=code_start --stack_size=0x3F8 --heap_size=0x100 \
    --warn_sections --output_file=CPU1_FLASH/Supercap_V2.out \
    --map_file=CPU1_FLASH/Supercap_V2.map CPU1_FLASH/obj/*.obj \
    28004x_generic_flash_lnk.cmd \
    "$C2000WARE_ROOT/driverlib/f28004x/driverlib/ccs/Release/driverlib_eabi.lib" \
    --search_path="$CCS_ROOT/tools/compiler/ti-cgt-c2000_25.11.1.LTS/lib" \
    "$C2000WARE_ROOT/libraries/calibration/hrpwm/f28004x/lib/SFO_v8_fpu_lib_build_c28_driverlib_eabi.lib" \
    --library=rts2800_fpu32_eabi.lib
