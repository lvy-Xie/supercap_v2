"""检查实际生成的 V2 引脚、构建入口和精简后的链接结果。"""
from pathlib import Path
import re
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
board = (root / 'CPU1_FLASH/syscfg/board.h').read_text()
expected = {
    'PWM_BRIDGE_A_EPWMA_GPIO': '0', 'PWM_BRIDGE_A_EPWMB_GPIO': '1',
    'PWM_BRIDGE_B_EPWMA_GPIO': '2', 'PWM_BRIDGE_B_EPWMB_GPIO': '3',
    'ADC_A_CHANNEL_I_BRIDGE_A': 'ADC_CH_ADCIN2',
    'ADC_A_CHANNEL_I_IN': 'ADC_CH_ADCIN6',
    'ADC_B_CHANNEL_V_BAT': 'ADC_CH_ADCIN2',
    'ADC_B_CHANNEL_V_CAP': 'ADC_CH_ADCIN4',
    'ADC_C_CHANNEL_I_BRIDGE_B': 'ADC_CH_ADCIN1',
    'ADC_C_CHANNEL_I_CAP': 'ADC_CH_ADCIN10',
    'ADC_C_CHANNEL_I_CHASSIS': 'ADC_CH_ADCIN3',
    'CMPSS_V_CAP_BASE': 'CMPSS4_BASE',
    # SysConfig 是 PWM 参数的唯一来源；主机执行实际生成的初始化。
    'PWM_BRIDGE_A_TBPRD': '285', 'PWM_BRIDGE_B_TBPRD': '285',
    'PWM_ADC_TRIGGER_TBPRD': '285',
    'PWM_BRIDGE_A_DBRED': '5', 'PWM_BRIDGE_A_DBFED': '5',
    'PWM_BRIDGE_B_DBRED': '5', 'PWM_BRIDGE_B_DBFED': '5',
    'PWM_BUZZER_TBPRD': '1561',
    'SCI_DEBUG_BAUDRATE': '1562500',
    'SCI_DEBUG_SCITX_GPIO': '24', 'SCI_DEBUG_SCIRX_GPIO': '17',
}
for name, value in expected.items():
    assert re.search(r'^#define\s+' + name + r'\s+' + value + r'\s*$', board, re.M), name
project = ET.parse(root / '.cproject')
configs = project.findall(".//storageModule[@moduleId='cdtBuildSystem']/configuration")
assert len(configs) == 1 and configs[0].get('name') == 'CPU1_FLASH'
optimization = project.find(".//option[@superClass='com.ti.ccstudio.buildDefinitions.C2000_25.11.compilerID.OPT_LEVEL']")
assert optimization is not None and optimization.get('value').endswith('.2')
for old in ('application', 'control', 'supercap_debug_main.c', 'device/driverlib', '28004x_generic_ram_lnk.cmd'):
    assert not (root / old).exists(), old
link_map = (root / 'CPU1_FLASH/Supercap_V2.map').read_text()
for unused in ('Supercap_RequestEnable', 'ReceiveCommands', 'Telemetry_CommandsAccepted',
               'Telemetry_CommandsRejected', 'Telemetry_RxErrors', 'Telemetry_LastCommand',
               'Voltage_Control_Loop',
               'Current_FeedbackSource', 'CMPSS_I_BRIDGE_A_init',
               'CMPSS_I_BRIDGE_B_init', 'CMPSS_I_CAP_init', 'CMPSS_V_BAT_init', 'CMPSS_V_CAP_init',
               'CAN_CHASSIS_init', 'PWM_FAN_init'):
    assert unused not in link_map, unused
for function in ('CLA1Task1_ISR', 'Stop_PWM', 'AdcDma_ISR',
                 'Buzzer_PWM_ISR', 'Telemetry_Publish'):
    assert '.TI.ramfunc:' + function in link_map, function
for function in ('Control_Loop', 'Test_Charge_Loop', 'Power_Control_Loop', 'Update_PWM_Duty'):
    assert 'cla_tasks.obj (Cla1Prog:' + function + ')' in link_map, function
    assert '.TI.ramfunc:' + function not in link_map, function
    address = re.search(r'^0\s+([0-9a-fA-F]{8})\s+' + function + r'\s*$', link_map, re.M)
    assert address and 0x8000 <= int(address[1], 16) < 0x9000, function
assert 'SFO_v8_fpu_lib_build_c28_driverlib_eabi.lib' in link_map
for function in ('SFO', 'MEP_ScaleFactor', 'Supercap_ServiceHRPWM'):
    assert function in link_map, function
for function in ('Buzzer_Init', 'Buzzer_PlayStartupTone', 'PWM_BUZZER_init', 'SCI_DEBUG_init',
                 'Telemetry_Init', 'Telemetry_Poll', 'Supercap_BackgroundPoll',
                 'TIMER_BACKGROUND_1KHZ_init', 'ClaADC_Init', 'Cla1Task1', 'Cla1Task8',
                 'CurrentA_loop', 'Control_Init', 'AdcDma_Init', 'DMA_ADC_CAPTURE_init'):
    assert function in link_map, function
print('通过：V2引脚、死区、构建入口、RAM控制函数和无旧模块链接残留')

app = (root / 'Codes/supercap.c').read_text()
for manual_config in ('setTimeBasePeriod', 'setPhaseShift', 'setMEPEdgeSelect',
                      'setDeadBand', 'setADCTrigger', 'setCounterCompareShadowLoadMode',
                      'setActionQualifierAction', 'setInterruptSource'):
    assert manual_config not in app, manual_config
board_c = (root / 'CPU1_FLASH/syscfg/board.c').read_text()
for name, phase in [('PWM_BRIDGE_B', 145), ('PWM_ADC_TRIGGER', 4)]:
    assert f'EPWM_setPhaseShift({name}_BASE, {phase});' in board_c
for name in ('PWM_BRIDGE_A', 'PWM_BRIDGE_B'):
    body = re.search(r'void ' + name + r'_init\(\)\s*\{[^}]*\}', board_c).group()
    assert 'enableTripZoneSignals' not in body
    assert 'HRPWM_enableAutoConversion' in body
    assert 'HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_RISING_EDGE' in body
    assert 'HRPWM_CHANNEL_B, HRPWM_MEP_CTRL_FALLING_EDGE' in body
assert 'ADC_setInterruptSource(ADC_C_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER3)' in board_c
assert 'CLA_setTriggerSource(CLA_TASK_1, CLA_TRIGGER_SOFTWARE)' in board_c
assert 'CLA_setTriggerSource(CLA_TASK_1, CLA_TRIGGER_ADCC1)' not in board_c
assert 'CLA_enableTasks(CLA_ADC_BASE, CLA_TASKFLAG_1 | CLA_TASKFLAG_8' in board_c
assert 'CLA_TASKFLAG_2' not in board_c
assert 'MEMCFG_SECT_LS0, MEMCFG_CLA_MEM_PROGRAM' in board_c
assert 'MEMCFG_SECT_LS1, MEMCFG_CLA_MEM_PROGRAM' in board_c
assert 'MEMCFG_SECT_LS2, MEMCFG_CLA_MEM_DATA' in board_c
assert 'Interrupt_register(INT_CLA1_1, &CLA1Task1_ISR)' in app
assert 'Interrupt_register(INT_ADCC1' not in app
assert 'Control_Loop();' not in app
assert 'HRPWM_setCounterCompareValue' not in app
assert 'CLA_disableTasks(CLA1_BASE, CLA_TASKFLAG_1)' in app
assert 'CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1)' in app
cla_source = (root / 'Codes/cla_tasks.cla').read_text()
assert 'if (!Control_StopRequest) Control_Loop();' in cla_source
assert 'ADC_readResult' not in cla_source and 'ADC_clearInterruptStatus' not in cla_source
assert 'AdcDma_Frame.raw[i]' in cla_source
adc_source = (root / 'Codes/cla_adc.c').read_text()
assert 'CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_1)' in adc_source
assert 'AdcDma_Frame.sequence != Cla_AdcData.sequence' in adc_source
assert 'DMA_getTransferStatusFlag' in adc_source and 'DMA_getOverflowFlag' in adc_source
capture = re.search(r'void DMA_ADC_CAPTURE_init\(\)\s*\{[^}]*\}', board_c).group()
for generated_setting in ('DMA_configBurst(DMA_ADC_CAPTURE_BASE, 4U, 1, 1)',
                           'DMA_configTransfer(DMA_ADC_CAPTURE_BASE, 3U, 29, 1)',
                           'DMA_TRIGGER_ADCC1', 'DMA_CFG_ONESHOT_ENABLE',
                           'DMA_CFG_CONTINUOUS_DISABLE', 'DMA_INT_AT_END',
                           'DMA_enableOverrunInterrupt', 'DMA_stopChannel'):
    assert generated_setting in capture, generated_setting
assert 'DMA_init();' in adc_source and 'DMA_configBurst' not in adc_source
assert 'DCL_runPI' not in (root / 'Codes/control.c').read_text()
timer = re.search(r'void TIMER_BACKGROUND_1KHZ_init\(\)\s*\{[^}]*\}', board_c).group()
assert '99999U' in timer and 'CPUTimer_disableInterrupt' in timer
assert 'CPUTimer_startTimer' in timer
assert 'DEVICE_DELAY_US' not in (root / 'main.c').read_text()
assert 'DEVICE_DELAY_US' not in (root / 'Codes/buzzer.c').read_text()
telemetry = (root / 'Codes/includes/telemetry.h').read_text()
assert re.search(r'TELEMETRY_SAMPLE_DIVIDER\s+10U', telemetry)
# 25 MHz LSPCLK，BRR=1 无分频误差；目标速率约占线速的 80%。
assert 25000000 // (8 * 1562500) - 1 == 1
assert 100000000 / (286 * 7 * 10) * 20 * 10 < 1562500 * 0.81
for symbol, address in [('Cla1ProgRunStart', 0x8000),
                        ('RamfuncsRunStart', 0x9800), ('Cla_AdcData', 0x1480),
                        ('AdcDma_Frame', 0x1500), ('AdcDma_Buffer', 0xC000)]:
    match = re.search(r'^[01]\s+([0-9a-fA-F]{8})\s+' + symbol + r'\s*$', link_map, re.M)
    assert match and int(match[1], 16) == address, symbol
for symbol in ('Cla1ConstRunStart', 'SuperCap_Control', 'Voltage_loop', 'CurrentA_loop',
               'set_current', 'set_voltage', 'Control_StopRequest', 'Control_Diag',
               'Referee_Power_Loop', 'Control_Mode', 'set_power', 'power_feedback',
               'target_DCDC_Power', 'target_cap_current'):
    match = re.search(r'^[01]\s+([0-9a-fA-F]{8})\s+' + symbol + r'\s*$', link_map, re.M)
    assert match and 0x9000 <= int(match[1], 16) < 0x9800, symbol
assert '.cinit.Cla1ToCpuMsgRAM' not in link_map
assert '.cinit.CpuToCla1MsgRAM' not in link_map
print('通过：SysConfig生成HRPWM/相移/ADC触发配置、SFO库链接、应用无手工驱动覆盖')
print('通过：CLA2任务触发、CPU/CLA分区与Flash复制、消息RAM地址、SCIA引脚与波特率')
print('通过：SysConfig DMA跨ADC整帧搬运、GS0缓冲、消息RAM提交与软件触发CLA，应用无静态DMA覆盖')
