/*
 * HC595.c
 * Created on: 2019年01月03日
 * Author: LiuDong
 */

#include "F2806x_Device.h"    //DSP28030外设定义头文件
#include "Stepper-Settings.h"

HC595_REG HC595Regs = {0};
KEY_HC597_REG KEY_HC597Regs = {0};
INPUT_HC597_REG INPUT_HC597Regs = {0};

void delay(unsigned int i)
{
	unsigned int j;
	for(i;i>0;i--)
	{
		for(j=100;j>0;j--);
	}
}
void delay1(unsigned int k)//1us延时
{
    unsigned int l;
    for(k;k>0;k--)
    {
        for(l=2;l>0;l--);
    }
}
//电机控制需要TMS320F28031的GPIO引脚进行控制,更新数据需要10ms
void HC595SendData(Uint16 SendVal)
{
    unsigned char i;
	for(i=0;i<16;i++)
	{
	    if((SendVal<<i)&0x8000)
	        DataHigh = 1;
	    else
	        DataLow = 1;
	    SclkLow = 1;
	    delay1(1);//1us
	    SclkHigh = 1;
	}
	RclkLow = 1;
	delay1(1);//1us
	RclkHigh = 1;
}
void LED_HC595SendData(Uint16 SendVal)
{
    unsigned char i;
    for(i=0;i<16;i++)
    {
        if((SendVal<<i)&0x8000)
        {
            LED_OUT_DATA_H = 1;
        }
        else
        {
            LED_OUT_DATA_L = 1;
        }
        KEY_SHCP_L = 1;
        delay1(1);//1us
        KEY_SHCP_H = 1;
    }
    LED_OUT_CLK_L = 1;
    delay1(1);//1us
    LED_OUT_CLK_H = 1;
}
Uint16 READ_HC597(void)
{
    Uint16 Dat1=0;
    Uint16 i = 0;

    HC597_PL_L = 1;  //
    HC597_STCP_L = 1;

    delay1(1);//1us
    delay1(1);//1us

    HC597_PL_H  = 1;  //读取开关数据
    HC597_STCP_H = 1; //RCK置上升沿(数据加载到输入锁存器)

    for(i=0;i<16;i++)
    {
        Dat1 <<= 1 ;
        if(HC597_DATA)
            Dat1 |= 0x01;
        HC597_SHCP_L = 1; //数据可变
        delay1(1);//1us
        HC597_SHCP_H = 1; //数据不变
        delay1(1);//1us
    }
    return Dat1;
}

Uint16 KEY_HC597(void)
{
    Uint16 Dat1=0;
    Uint16 i = 0;

    KEY_PL_L = 1;
    KEY_STCP_L = 1;

    delay1(1);//1us
    delay1(1);//1us

    KEY_PL_H = 1;
    KEY_STCP_H = 1;

    for(i=0;i<8;i++)
    {
        Dat1 <<= 1 ;
        if(KEY_DATA)
            Dat1 |= 0x01;
        KEY_SHCP_L = 1;
        delay1(1);//1us
        KEY_SHCP_H = 1;
        delay1(1);//1us
    }
    return Dat1;
}







