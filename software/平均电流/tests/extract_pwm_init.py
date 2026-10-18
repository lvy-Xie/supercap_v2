"""供主机测试执行真实 SysConfig 生成的功率桥、ADC 触发及蜂鸣器初始化。"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
board = (root / 'CPU1_FLASH/syscfg/board.c').read_text()
header = (root / 'CPU1_FLASH/syscfg/board.h').read_text()
functions = []
for name in ('PWM_BRIDGE_A', 'PWM_BRIDGE_B', 'PWM_ADC_TRIGGER', 'PWM_BUZZER',
             'TIMER_BACKGROUND_1KHZ', 'DMA_ADC_CAPTURE', 'DMA'):
    match = re.search(r'void ' + name + r'_init\(\)\s*\{[^}]*\}', board)
    assert match, name
    functions.append(match.group())
defines = re.findall(r'^#define (?:PWM_BRIDGE_[AB]|PWM_BUZZER)_TBPRD\s+\d+\s*$', header, re.M)
assert len(defines) == 3
Path(sys.argv[1]).write_text('\n'.join(defines + functions) + '\n')
