#include "includes/control.h"
#include "includes/config.h"

// 固定宽度、无指针的调试参数与状态，CPU 定义于双方可访问的 LS2 数据 RAM。
#pragma DATA_SECTION(Voltage_loop, "ControlShared")
#pragma DATA_SECTION(Referee_Power_Loop, "ControlShared")
#pragma DATA_SECTION(Control_Mode, "ControlShared")
#pragma DATA_SECTION(set_power, "ControlShared")
#pragma DATA_SECTION(power_feedback, "ControlShared")
#pragma DATA_SECTION(target_DCDC_Power, "ControlShared")
#pragma DATA_SECTION(target_cap_current, "ControlShared")
#pragma DATA_SECTION(CurrentA_loop, "ControlShared")
#pragma DATA_SECTION(set_voltage, "ControlShared")
#pragma DATA_SECTION(set_current, "ControlShared")
#pragma DATA_SECTION(pid_v_out, "ControlShared")
#pragma DATA_SECTION(pid_i_out, "ControlShared")
#pragma DATA_SECTION(current_feedback, "ControlShared")
#pragma DATA_SECTION(Current_VoltageLimited, "ControlShared")
#pragma DATA_SECTION(Control_StopRequest, "ControlShared")
#pragma DATA_SECTION(Control_LastFault, "ControlShared")
#pragma DATA_SECTION(Control_Diag, "ControlShared")
DCL_PID_CLA Voltage_loop;
DCL_PID_CLA Referee_Power_Loop;
DCL_PI_CLA CurrentA_loop;
volatile uint16_t Control_Mode = CONTROL_MODE_CHARGE;
volatile float set_power = 30.0f, power_feedback, target_DCDC_Power, target_cap_current;
volatile float set_voltage = 26.5f, set_current = 4.5f;
volatile float pid_v_out, pid_i_out, current_feedback;
volatile uint16_t Current_VoltageLimited, Control_StopRequest, Control_LastFault;
volatile Control_Diagnostics_s Control_Diag;

// CPU 只在启动时设置参数；所有周期性控制运算在 cla_tasks.cla 中编译执行。
void Control_Init(void)
{
    Control_Diagnostics_s cleared = {0};
    Control_Diag = cleared;
    Referee_Power_Loop.Kp = 0.5f;
    Referee_Power_Loop.Ki = 0.01f;
    Referee_Power_Loop.Kd = 1.0f;
    Referee_Power_Loop.Kr = Referee_Power_Loop.c1 = Referee_Power_Loop.i14 = 1.0f;
    Referee_Power_Loop.c2 = Referee_Power_Loop.d2 = Referee_Power_Loop.d3 = Referee_Power_Loop.i10 = 0.0f;
    Referee_Power_Loop.Umax = 200.0f;
    Referee_Power_Loop.Umin = -200.0f;
    power_feedback = target_DCDC_Power = target_cap_current = 0.0f;
    Voltage_loop.Kp = 0.1f;
    Voltage_loop.Ki = 0.005f;
    Voltage_loop.Kd = 1.5f;
    Voltage_loop.Kr = Voltage_loop.c1 = 1.0f;
    Voltage_loop.c2 = Voltage_loop.d2 = Voltage_loop.d3 = Voltage_loop.i10 = 0.0f;
    Voltage_loop.i14 = 1.0f;
    Voltage_loop.Umax = MAX_CAP_VOLTAGE;
    Voltage_loop.Umin = 0.0f;
    CurrentA_loop.Kp = 0.0085f;
    CurrentA_loop.Ki = 0.0005f;
    CurrentA_loop.Umax = MAX_OUTPUT_DUTY;
    CurrentA_loop.Umin = MIN_OUTPUT_DUTY;
    CurrentA_loop.i10 = CurrentA_loop.i11 = 0.0f;
    CurrentA_loop.i6 = 1.0f;
    CurrentA_loop.Imax = 2.0f;
    CurrentA_loop.Imin = -2.0f;
    pid_i_out = pid_v_out = current_feedback = 0.0f;
    Current_VoltageLimited = Control_StopRequest = Control_LastFault = 0;
}
