#include "F2806x_Device.h"    //DSP28030外设定义头文件
#include "Stepper-Settings.h"

//Uint16 CapCnt = 0;

#pragma CODE_SECTION(CapISR, "ramfuncs");
interrupt void CapISR(void) //GPIO24--ECAP1 输入脉冲捕获
{
    Embroider.AB_Pulse++;
    TZ.AB_Pulse++;
    if((Embroider.Speed > 95)&&(Embroider.Speed <= 105))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1400+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 85)&&(Embroider.Speed <= 95))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1300+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 75)&&(Embroider.Speed <= 85))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1250+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 65)&&(Embroider.Speed <= 75))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1200+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 55)&&(Embroider.Speed <= 65))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1300+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 45)&&(Embroider.Speed <= 55))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1350+TZ.DeltaTime))//Ax8
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 35)&&(Embroider.Speed <= 45))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1400+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 25)&&(Embroider.Speed <= 35))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1450+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 15)&&(Embroider.Speed <= 25))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1500+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if((Embroider.Speed > 5)&&(Embroider.Speed <= 15))
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1500+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    if(Embroider.Speed < 5)
    {
        if(TZ.Enable == 1)
        {
            if(TZ.AB_Pulse >= (1500+TZ.DeltaTime))
            {
                TZ.cmd = 2;
                TZ.cmdFlag = 0;
                TZ.Enable = 0;
            }
        }
    }
    //-----------------------------------------------------------------------

    ECap1Regs.ECCLR.all = 0x0ff;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;//eCAP1_INT--INT4.1
}

