/*
 * Hand_Key.c
 * Created on: 2019年1月7日
 * Author:liudong
 */

#include "F2806x_Device.h"     //DSP28030外设定义头文件
#include "Stepper-Settings.h"

extern KEY_HC597_REG KEY_HC597Regs;
extern Uint16 KEY_HC597(void);

void Machine_State_Write();

KEY_HANDLE KeyHandle = KEY_HANDLE_DEFAULT;
void KeyRead(void)  //判断按键状态的子程序
{
    static unsigned char KeyCont1 = 0;//去抖
    static unsigned char KeyCont2 = 0;//去抖
    static unsigned char KeyCont3 = 0;//去抖

    KEY_HC597Regs.all = KEY_HC597();

    Uint16 KeyBoard = (KEY_HC597Regs.bit.Mode << 4) + (KEY_HC597Regs.bit.Dir << 3) + (KEY_HC597Regs.bit.Up << 2) + (KEY_HC597Regs.bit.OK << 1) + KEY_HC597Regs.bit.Down;
    Uint16 ReadData = KeyBoard ^ 0x001F;

    KeyCont3++;
    if(KeyCont3 > 2) KeyCont3 = 1;
    if(KeyCont3 == 1) KeyCont1 = ReadData;
    if(KeyCont3 == 2) KeyCont2 = ReadData;
    if(KeyCont1 == KeyCont2 )
    {
        KeyHandle.Trg = ReadData&(ReadData^KeyHandle.Cont);
        KeyHandle.Cont = ReadData;
        KeyHandle.DownTrg = KeyHandle.Trg & 0x0001;
        KeyHandle.OkTrg = (KeyHandle.Trg & 0x0002) >> 1;
        KeyHandle.UpTrg = (KeyHandle.Trg & 0x0004) >> 2;
        KeyHandle.AirTrg = (KeyHandle.Trg & 0x0008) >> 3;
        KeyHandle.ModeTrg = (KeyHandle.Trg & 0x0010) >> 4;
    }
}
void KEY_Hand_Handle(void)
{
    if(KeyHandle.Mode == 0)
    {
        //if((MC.S_ERR==1)||(MB.S_ERR==1)||(MC.S_ERR==2))
        if(MC.S_ERR==1)//防止其他报错时，拨叉无反应
        {
            //if(KeyHandle.OkTrg == 1)
            if(KeyHandle.UpTrg == 1)
            {
                //KeyHandle.OkTrg = 0;
                KeyHandle.UpTrg = 0;
                //将重启前相关状态存入EEPROM
                //ColorState_Write(HS.Color_state);
                //AirValveState_Write(Embroider.AirValvePos,Embroider.PearlNeedle);
                Machine_State_Write();
                PWM1_OFF_MACRO()
                PWM2_OFF_MACRO()
                DINT;
                (*DSP_Flash_Update)(); /*跳转到DSP Bootloader*/
            }
        }
        else
        {
            if(KeyHandle.OkTrg == 1)
            {
                KeyHandle.OkTrg = 0;
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
                KeyHandle.OkTrgCnt++;
                TZ.ErrFlag = 0;
                switch(KeyHandle.OkTrgCnt)
                {
                case 1:
                    TZ.cmd = 1;
                    break;
                case 2:
                    TZ.cmd = 2;
                    break;
                case 3:
                    TZ.cmd = 3;
                    //TZ.HandTZFlag = 1;
                    break;
                }
                if(KeyHandle.OkTrgCnt >= 3)
                    KeyHandle.OkTrgCnt = 0;
                //TZ.Position_state = Push_Pearl(1);
            }
        }
        if(KeyHandle.ModeTrg == 1)
        {
            KeyHandle.ModeTrg = 0;
            KeyHandle.Mode = 1;
            Par_Key_Read();
        }
        if(KeyHandle.UpTrg == 1)
        {
            if(MC.Servo_OFF == 1)
            {
                MC.Servo_OFF = 0;
                HS.ONPosData = EQep1Regs.QPOSCNT;
                HS.DeltaOFFON = HS.OFFPosData - HS.ONPosData;
                if(HS.DeltaOFFON > 2000)
                    HS.DeltaOFFON = 4000 - HS.DeltaOFFON;
                else if(HS.DeltaOFFON < -2000)
                    HS.DeltaOFFON = 4000 + HS.DeltaOFFON;
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
            }
            KeyHandle.UpTrg = 0;
            HS.Color_state = Change_Pearl_Color();
            HS.DeltaOFFON = 0;
        }
        KeyHandle.Para = 1;
        KeyHandle.LEDEnable = 1;
    }
    else if(KeyHandle.Mode == 1)//调参模式
    {
        switch(KeyHandle.Para)
        {
        case 1:
            if(KeyHandle.LEDEnable == 1)
            {
                KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F1]);
                KeyHandle.DisplayEnable = 1;
                KeyHandle.LEDEnable = 0;
            }
            if(KeyHandle.ModeTrg == 1)
            {
                KeyHandle.ModeTrg = 0;
                KeyHandle.Para = 2;
                KeyHandle.LEDEnable = 1;
            }
            if(KeyHandle.OkTrg == 1)
            {
                KeyHandle.OkTrg = 0;
                KeyHandle.Mode = 0;
                KeyHandle.Para = 1;
                Par_Key_Write();
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
            }
            break;
        case 2:
            if(KeyHandle.LEDEnable == 1)
            {
                KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F2]);
                KeyHandle.DisplayEnable = 1;
                KeyHandle.LEDEnable = 0;
            }
            if(KeyHandle.ModeTrg == 1)
            {
                KeyHandle.ModeTrg = 0;
                KeyHandle.Para = 3;
                KeyHandle.LEDEnable = 1;
            }
            if(KeyHandle.OkTrg == 1)
            {
                KeyHandle.OkTrg = 0;
                KeyHandle.Mode = 0;
                KeyHandle.Para = 1;
                Par_Key_Write();
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
            }
            break;
        case 3:
            if(Embroider.LightState != Embroider.LightStateOld)
            {
                KeyHandle.LEDEnable = 1;
            }
            if(KeyHandle.LEDEnable == 1)
            {
                if(Embroider.LightState == 2)
                {
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F3]);
                }
                else if(Embroider.LightState == 3)
                {
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F5]);
                }
                KeyHandle.DisplayEnable = 1;
                KeyHandle.LEDEnable = 0;
            }
            if(KeyHandle.ModeTrg == 1)
            {
                KeyHandle.ModeTrg = 0;
                KeyHandle.Para = 4;
                KeyHandle.LEDEnable = 1;
            }
            if(KeyHandle.OkTrg == 1)
            {
                KeyHandle.OkTrg = 0;
                KeyHandle.Mode = 0;
                KeyHandle.Para = 1;
                Par_Key_Write();
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
            }
            Embroider.LightStateOld = Embroider.LightState;
            break;
        case 4:
            if(KeyHandle.LEDEnable == 1)
            {
                KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F4]);
                KeyHandle.DisplayEnable = 1;
                KeyHandle.LEDEnable = 0;
            }

            if(KeyHandle.OkTrg == 1)
            {
                KeyHandle.OkTrg = 0;
                KeyHandle.OkTrgCnt++;
                TZ.ErrFlag = 0;
                switch(KeyHandle.OkTrgCnt)
                {
                case 1:
                    TZ.cmd = 1;
                    break;
                case 2:
                    TZ.cmd = 2;
                    break;
                case 3:
                    TZ.cmd = 3;
                    //TZ.HandTZFlag = 1;
                    break;
                }
                if(KeyHandle.OkTrgCnt >= 3)
                    KeyHandle.OkTrgCnt = 0;
                //TZ.Position_state = Push_Pearl(1);
            }
            if(KeyHandle.ModeTrg == 1)
            {
                KeyHandle.ModeTrg = 0;
                KeyHandle.Mode = 0;
                KeyHandle.Para = 1;
                Par_Key_Write();
                KeyHandle.DisplayData = 0x0040;
                KeyHandle.DisplayEnable = 1;
            }
            break;
        }
        if(KeyHandle.UpTrg == 1)
        {
            KeyHandle.UpTrg = 0;
            switch(KeyHandle.Para)
            {
            case 1:
                if(PA.F1 < 9)
                {
                    PA.F1++;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F1]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            case 2:
                if(PA.F2 < 9)
                {
                    PA.F2++;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F2]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            case 3:
               /* if(PA.F3 < 9)
                {
                    PA.F3++;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F3]);
                    KeyHandle.DisplayEnable = 1;
                }*/
                if(Embroider.LightState == 2)
                {
                    if(PA.F3 < 9)
                    {
                        PA.F3++;
                        KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F3]);
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                else if(Embroider.LightState == 3)
                {
                    if(PA.F5 < 9)
                    {
                        PA.F5++;
                        KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F5]);
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                break;
            case 4:
                if(PA.F4 < 9)
                {
                    PA.F4++;
                    if(HS.Color_state == 0)
                        HS.Color_state = Change_Pearl_Color();
                    else
                        MC.Motor_pos_set = -40;
                    MC.Pulse_set_Ena = 1;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F4]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            }
        }
        if(KeyHandle.DownTrg == 1)
        {
            KeyHandle.DownTrg = 0;
            switch(KeyHandle.Para)
            {
            case 1:
                if(PA.F1 > 0)
                {
                    PA.F1--;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F1]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            case 2:
                if(PA.F2 > 0)
                {
                    PA.F2--;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F2]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            case 3:
                /*if(PA.F3 > 0)
                {
                    PA.F3--;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F3]);
                    KeyHandle.DisplayEnable = 1;
                }*/
                if(Embroider.LightState == 2)
                {
                    if(PA.F3 > 0)
                    {
                        PA.F3--;
                        KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F3]);
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                else if(Embroider.LightState == 3)
                {
                    if(PA.F5 > 0)
                    {
                        PA.F5--;
                        KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F5]);
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                break;
            case 4:
                if(PA.F4 > 0)
                {
                    PA.F4--;
                    if(HS.Color_state == 0)
                        HS.Color_state = Change_Pearl_Color();
                    else
                        MC.Motor_pos_set = 40;
                    MC.Pulse_set_Ena = 1;
                    KeyHandle.DisplayData = (KeyHandle.LED_DATA[KeyHandle.Para - 1]<<8)|(KeyHandle.NixieTube_DATA[PA.F4]);
                    KeyHandle.DisplayEnable = 1;
                }
                break;
            }
        }
    }
    //在拨叉回到原点后再刷新参数值
    //if(TZ.Position_state == 0)
    if(TZ.Position_state == 2)
    {
        PA.F1_Value = PA.F1;
        PA.F2_Value = PA.F2;
    }
}
void KEY_Long_Handle(void)
{
    static Uint16 Delay = 0;
    if(KeyHandle.Cont == 0x0002)//长按OK
    {
        Delay++;
        if(Delay >= 120)
        {
            Delay = 200;
            //MC.S_ERR=1;
            //MB.S_ERR=1;
            M3_DIS=1;//开环拨叉松轴
            KeyHandle.DisplayData = 0x0071;
            KeyHandle.DisplayEnable = 1;
        }
    }
    else
    {
        Delay= 0;
    }

    static Uint16 Delay2 = 0;
    if(KeyHandle.Cont == 0x0004)//长按上箭头
    {
        Delay2++;
        if(Delay2 >= 120)
        {
            Delay2 = 200;
            //MC.S_ERR=1;
            HS.OFFPosData = EQep1Regs.QPOSCNT;
            KeyHandle.DisplayData = 0x0079;
            KeyHandle.DisplayEnable = 1;
            MC.Servo_OFF = 1;
        }
    }
    else
    {
        Delay2= 0;
    }
}
