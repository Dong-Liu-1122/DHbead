
#include "F2806x_Device.h"    //DSP28030外设定义头文件
#include "Stepper-Settings.h"

Uint16 Change_Pearl_Color(void);


#pragma CODE_SECTION(Change_Pearl_Color, "ramfuncs");
Uint16 Change_Pearl_Color(void)
{
    static Uint16 Color_State = 0;
    Color_State++;
    if(Color_State == 1)
    {

        MC.Cur_Kp_Work = _IQ(2);//2
        MC.Cur_Ki_Work =_IQ(0.02);//
        MC.Spd_Kp_Work = _IQ(5.0);//
        MC.Spd_Ki_Work = _IQ(0.02);//
        MC.Pos_Kp_Work = 25;
        MC.Gain_Switch_Ena = 1;

        MC.Motor_pos_set = -HS_Distance - HS.Delta[PA.F4] + HS.DeltaOFFON;
    }
    else if(Color_State == 2)
    {
        MC.Cur_Kp_Work = _IQ(2);//2
        MC.Cur_Ki_Work =_IQ(0.02);//
        MC.Spd_Kp_Work = _IQ(5.0);//
        MC.Spd_Ki_Work = _IQ(0.02);//
        MC.Pos_Kp_Work = 25;
        MC.Gain_Switch_Ena = 1;

        MC.Motor_pos_set = HS_Distance + HS.Delta[PA.F4] + HS.DeltaOFFON;
    }
    //curve1.RunTime  = 100;
    curve1.RunTime  = 120;
    curve1.RiseTime = 30;
    //curve1.DownTime = 40;
    curve1.DownTime = 50;
    curve1.GenFlg = 2;
    CurveInit(&curve1);//曲线发生器初始化
    MC.Pulse_set_Ena = 1;
    if(Color_State >= 2)
        Color_State = 0;
    return Color_State;
}
