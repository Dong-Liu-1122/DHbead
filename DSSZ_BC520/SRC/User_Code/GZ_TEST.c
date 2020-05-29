/*
 * Yajiao_Control.c
 *
 *  Created on: 2019年01月08日
 *      Author: liudong
 */

#include "F2806x_Device.h"     //DSP28030外设定义头文件
#include "Stepper-Settings.h"
#include "GzTest.h"
#include "HC595.h"

GZTEST_HANDLE GZ = GZTEST_HANDLE_DEFAULT;

void CAN_TX_DLC2(Uint16 TX_Data);

void GZ_TEST_SM(void)
{
    switch(GZ.State)
    {
    case 0:
       /* if(DIP2 == 0)
            GZ.State = 1;*/
        break;
    case 1:
        GZ.Cmd = 1;//闭环1电机正转一圈 ，闭环2电机正转一圈
        GZ.State = 2;
        break;
    case 2:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            GZ.State = 3;
        }
        break;
    case 3:
        GZ.Cmd = 2;//闭环1电机反转一圈，闭环2电机反转一圈
        GZ.State = 4;
        break;
    case 4:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            if(GZ.loop == 0)
                GZ.State = 5;
            else
            {
                if(MC.Err_Value == 0)
                    CAN_TX_DLC2(0xAA00);
                else
                    CAN_TX_DLC2(0xAA01);
                GZ.State = 0;
            }
        }
        break;
    case 5:
        GZ.Cmd = 3;//开环1/2/3电机正转一圈
        GZ.State = 6;
        break;
    case 6:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            GZ.State = 7;
        }
        break;
    case 7:
        GZ.Cmd = 4;//开环1/2/3电机反转一圈
        GZ.State = 8;
        break;
    case 8:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            if(GZ.loop == 0)
                GZ.State = 9;
            else
            {
                if(MB.Err_Value == 0)
                    CAN_TX_DLC2(0xAA00);
                else
                    CAN_TX_DLC2(0xAA01);
                GZ.State = 0;
            }
        }
        break;
    case 9:
        GZ.Cmd = 5;//开环1电机正转一圈
        GZ.State = 10;
        break;
    case 10:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            GZ.State = 11;
        }
        break;
    case 11:
        GZ.Cmd = 6;//开环1电机反转一圈
        GZ.State = 12;
        break;
    case 12:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            if(GZ.loop == 0)
                GZ.State = 13;
            else
            {
                GZ.State = 0;
            }
        }
        break;
    case 13:
        GZ.Cmd = 7;//开环2电机正转一圈
        GZ.State = 14;
        break;
    case 14:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            GZ.State = 15;
        }
        break;
    case 15:
        GZ.Cmd = 8;//开环2电机反转一圈
        GZ.State = 16;
        break;
    case 16:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            if(GZ.loop == 0)
                GZ.State = 17;
            else
            {
                GZ.State = 0;
            }
        }
        break;
    case 17:
        GZ.Cmd = 9;//开环3电机正转一圈
        GZ.State = 18;
        break;
    case 18:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            GZ.State = 19;
        }
        break;
    case 19:
        GZ.Cmd = 10;//开环3电机反转一圈
        GZ.State = 20;
        break;
    case 20:
        GZ.Delay++;
        if(GZ.Delay >= 10000)
        {
            GZ.Delay = 0;
            M3_DIS = 1;
            GZ.State = 0;
        }
        break;
    }
}
void GZ_TEST_Handle(void)
{
    switch(GZ.Cmd)
    {
    case 0:
        break;
    case 1:
        GZ.Cmd = 0;
        MC.Motor_pos_set = 4000;
        curve1.RunTime  = 1000;
        curve1.RiseTime = 300;
        curve1.DownTime = 30;
        curve1.GenFlg = 2;
        CurveInit(&curve1);//曲线发生器初始化
        MC.Pulse_set_Ena = 1;
        break;
    case 2:
        GZ.Cmd = 0;
        MC.Motor_pos_set = -4000;
        curve1.RunTime  = 1000;
        curve1.RiseTime = 300;
        curve1.DownTime = 300;
        curve1.GenFlg = 2;
        CurveInit(&curve1);//曲线发生器初始化
        MC.Pulse_set_Ena = 1;
        break;
    case 3:
        GZ.Cmd = 0;
        MB.Motor_pos_set = -4000;
        curve2.RunTime  = 1000;
        curve2.RiseTime = 300;
        curve2.DownTime = 300;
        curve2.GenFlg = 2;
        CurveInit(&curve2);//曲线发生器初始化
        MB.Pulse_set_Ena = 1;
        break;
    case 4:
        GZ.Cmd = 0;
        MB.Motor_pos_set = 4000;
        curve2.RunTime  = 1000;
        curve2.RiseTime = 300;
        curve2.DownTime = 300;
        curve2.GenFlg = 2;
        CurveInit(&curve2);//曲线发生器初始化
        MB.Pulse_set_Ena = 1;
        break;
    case 5:
        GZ.Cmd = 0;

        M1_CCW = 1;
        motorC.EnableFlag = 1;
        motorC.step_spmax=40000;
        motorC.step_accel_up=15000000;
        motorC.step_accel_down=15000000;
        motorC.speedenbale = 0;
        motorC.Degree = 6400;
        motorC.PulseEnable = 1;
        break;
    case 6:
        GZ.Cmd = 0;

        M1_CW = 1;
        motorC.EnableFlag = 1;
        motorC.step_spmax=40000;
        motorC.step_accel_up=15000000;
        motorC.step_accel_down=15000000;
        motorC.speedenbale = 0;
        motorC.Degree = 6400;
        motorC.PulseEnable = 1;
        break;
    case 7:
        GZ.Cmd = 0;

        M2_CCW = 1;
        motorD.EnableFlag = 1;
        motorD.step_spmax=40000;
        motorD.step_accel_up=15000000;
        motorD.step_accel_down=15000000;
        motorD.speedenbale = 0;
        motorD.Degree = 6400;
        motorD.PulseEnable = 1;
        break;
    case 8:
        GZ.Cmd = 0;

        M2_CW = 1;
        motorD.EnableFlag = 1;
        motorD.step_spmax=40000;
        motorD.step_accel_up=15000000;
        motorD.step_accel_down=15000000;
        motorD.speedenbale = 0;
        motorD.Degree = 6400;
        motorD.PulseEnable = 1;
        break;
    case 9:
        GZ.Cmd = 0;

        M3_CCW = 1;
        motorE.EnableFlag = 1;
        motorE.step_spmax=20000;
        motorE.step_accel_up=3000000;
        motorE.step_accel_down=3000000;
        motorE.speedenbale = 0;
        motorE.Degree = 3200;
        motorE.PulseEnable = 1;
        break;
    case 10:
        GZ.Cmd = 0;

        M3_CW = 1;
        motorE.EnableFlag = 1;
        motorE.step_spmax=20000;
        motorE.step_accel_up=3000000;
        motorE.step_accel_down=3000000;
        motorE.speedenbale = 0;
        motorE.Degree = 3200;
        motorE.PulseEnable = 1;
        break;

    }
    if(GZ.Enable == 1)
    {
        //if((KeyHandle.UpTrg == 1)||(KeyHandle.DownTrg == 1)||(KeyHandle.ModeTrg == 1)||(KeyHandle.AirTrg == 1)||(KeyHandle.OkTrg == 1))
        if(KeyHandle.Cont != 0)
        {
            KeyHandle.UpTrg = 0;
            KeyHandle.DownTrg = 0;
            KeyHandle.ModeTrg = 0;
            KeyHandle.AirTrg = 0;
            KeyHandle.OkTrg = 0;
            KeyHandle.DisplayData = 0x0000;//全灭
            KeyHandle.DisplayEnable = 1;
            GZ.Delay = 0;
            GZ.Flag = 1;
        }
        else if(GZ.Flag == 1)
        {
            GZ.Delay++;
            if(GZ.Delay >= 2000)
            {
                KeyHandle.DisplayData = 0x077F;//全亮
                KeyHandle.DisplayEnable = 1;
                GZ.Delay = 0;
                GZ.Flag = 0;
            }
        }
    }
}
//-------工装测试CAN通讯处理(程序框架)-----------------//
#pragma CODE_SECTION(GZ_TEST_CAN_Handle, "ramfuncs");
void GZ_TEST_CAN_Handle(void)
{
    if((CAN.DLC==2)&&(CAN.ID==0x666))
    {
        //CAN通讯
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x00))
        {
            GZ.Enable = 1;
            M3_DIS = 1;
            KeyHandle.DisplayData = 0x077F;//全灭
            KeyHandle.DisplayEnable = 1;
#if(HardwareNumber == 0x32)
            CAN_TX_DLC2(0xAC00|SoftWareVersion);
#elif(HardwareNumber == 0x33)
            CAN_TX_DLC2(0xAD00|SoftWareVersion);
#endif
            Embroider.PearlNeedle = 1;
            Embroider.AB_Pulse = 0;

            MC.Pos_Kp_Work = 30;
            MC.Spd_Kp_Work = _IQ(2);
            MC.Spd_Ki_Work = _IQ(0.02);
            MC.Cur_Kp_Work = _IQ(3);
            MC.Cur_Ki_Work = _IQ(0.03);
            MC.Gain_Switch_Ena = 1;

            MB.Pos_Kp_Work = 30;
            MB.Spd_Kp_Work = _IQ(2);
            MB.Spd_Ki_Work = _IQ(0.02);
            MB.Cur_Kp_Work = _IQ(3);
            MB.Cur_Ki_Work = _IQ(0.03);
            MB.Gain_Switch_Ena = 1;

            HC595Regs.bit.M1_TQ0 = 1;
            HC595Regs.bit.M1_TQ1 = 1;

            HC595Regs.bit.M2_TQ0 = 1;
            HC595Regs.bit.M2_TQ1 = 1;

            HC595SendData(HC595Regs.all);
        }
        //CZ2012红绿灯信号
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x01))
        {
            CAN_TX_DLC2(0xAA00|(Embroider.LightState-1));
           /* if(Embroider.LightState == 0x03)
            {
                CAN_TX_DLC2(0xAA00|Embroider.LightState);
            }
            if(Embroider.LightState == 0x02)
            {
                CAN_TX_DLC2(0xAA00|Embroider.LightState);
            }*/
        }
        //CZ204
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x02))
        {
            if(INPUT_HC597Regs.bit.OPTO5 == 0x01)
            {
                CAN_TX_DLC2(0xAA01);
            }
            if(INPUT_HC597Regs.bit.OPTO5 == 0x00)
            {
                CAN_TX_DLC2(0xAA00);
            }
        }
        //CZ205
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x03))
        {
            if(INPUT_HC597Regs.bit.OPTO4 == 0x01)
            {
                CAN_TX_DLC2(0xAA01);
            }
            if(INPUT_HC597Regs.bit.OPTO4 == 0x00)
            {
                CAN_TX_DLC2(0xAA00);
            }
        }
        //CZ206
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x04))
        {
            if(INPUT_HC597Regs.bit.OPTO3 == 0x01)
            {
                CAN_TX_DLC2(0xAA01);
            }
            if(INPUT_HC597Regs.bit.OPTO3 == 0x00)
            {
                CAN_TX_DLC2(0xAA00);
            }
        }
        //CZ207
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x05))
        {
            if(INPUT_HC597Regs.bit.OPTO2 == 0x01)
            {
                CAN_TX_DLC2(0xAA01);
            }
            if(INPUT_HC597Regs.bit.OPTO2 == 0x00)
            {
                CAN_TX_DLC2(0xAA00);
            }
        }
        //CZ208
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x06))
        {
            if(INPUT_HC597Regs.bit.OPTO1 == 0x01)
            {
                CAN_TX_DLC2(0xAA01);
            }
            if(INPUT_HC597Regs.bit.OPTO1 == 0x00)
            {
                CAN_TX_DLC2(0xAA00);
            }
        }
        //AB脉冲信号
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x07))
        {
            Embroider.AB_Pulse = Embroider.AB_Pulse>>2;
            CAN_TX_DLC2(Embroider.AB_Pulse);
            Embroider.AB_Pulse = 0;
        }
        //气阀CZ2021 2022
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x08))
        {
            Air1_Valve_Clc = 1;
            Air2_Valve_Clc = 1;
        }
        //气阀CZ2021 2022
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x09))
        {
            Air1_Valve_Set = 1;
            Air2_Valve_Set = 1;
        }
        //闭环电机1
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0A))
        {
            GZ.State = 1;
            GZ.loop = 1;
        }
        //闭环电机2
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0B))
        {
            GZ.State = 5;
            GZ.loop = 1;
        }
        //开环电机123
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0C))
        {
            GZ.loop = 1;
            GZ.State = 9;
        }
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0E))
        {
            GZ.loop = 1;
            GZ.State = 13;
        }
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0F))
        {
            GZ.loop = 1;
            GZ.State = 17;
        }
        //DIP开关
        if((CAN.Rec[0]==0xEF)&&(CAN.Rec[1]==0x0D))
        {
            if((DIP1 == 0)&&(DIP2 == 1))
            {
                CAN_TX_DLC2(0xAA02);
            }
            if((DIP1 == 1)&&(DIP2 == 0))
            {
                CAN_TX_DLC2(0xAA01);
            }
        }
    }
}
