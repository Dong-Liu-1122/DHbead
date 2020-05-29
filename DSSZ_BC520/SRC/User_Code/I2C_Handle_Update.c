/*
 * I2C_Handle_Update.c
 *
 *  Created on: 2015年12月22日
 *      Author: liudong
 */

//======================================================================================
//         COPYRIGHT(C) 2009 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//  Project Number: pmsm_sensor_controller
//  File Name : I2C_handle.c
//  Description: 完成I2C读写控制
//  Version    Date     Author     Description
//  1.0    2015.01.24   wangdayu        use
//=========================================================================

//step1:头文件定义
#include "F2806x_Device.h"
#include "F2806x_I2c_defines.h"
#include "Stepper-Settings.h"
//#define CPU_CLOCK_SPEED      9.000L   // 10.000L for a 100MHz CPU clock speed
#define ADC_usDELAY 10000L
//#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_CLOCK_SPEED) - 9.0L) / 5.0L)
//extern void DSP28x_usDelay(unsigned long Count);
void Motor_Par_Reset(void);   //写入初始化参数
void Motor_Par_PowerOn_Read(void);
//step2:变量声明
Uint16  I2CA_ReadData(Uint16 RomAddress,Uint16 *RamAddr, Uint16 number);
Uint16  I2CA_WriteData(Uint16 RomAddress,Uint16 *Wdata, Uint16 number);
Uint16  I2C_xrdy();
Uint16  I2C_rrdy();

//I2C读写用变量
Uint16 Wri_eprom_addr;
Uint16 I2CRD_PAR[8]={0}; //I2C读出数组
Uint16 I2CWD_PAR[2]={0}; //I2C写入数组
//最高传输速率：400kbps

#define Par_num  18  //PA参数个数
//驱动器控制参数PA（IIC--128*16bit）--最多可存128参数

//初始化控制参数
Uint16 Def_Par[Par_num]={
                    0x05,\
                    //IIC地址=0x00 F1:原点到预备位置距离参数
                    0x05,\
                    //IIC地址=0x01 F2:预备位置到针孔距离参数
                    0x05,\
                    //IIC地址=0x02 F3:A珠下珠速度，2019.02.26
                    0x05,\
                    //IIC地址=0x03 F4(F1+F2):换珠行程微调参数
                    0x05,\
                    //预留参数->IIC地址=0x04 F5 B珠下珠速度，2019.02.26
                    0xB0,\
                    //换珠电机控制模式标志位
                    0xB0,\
                    //拨叉电机控制模式标志位
                    0x00,\
                    //机头板地址信息
                    0x00,\
                    //珠子颜色信息
                    0x00,\
                    //故障重启标志信息
                    0x01,\
                    //气阀状态信息
                    0x00,\
                    //散珠针位信息
                    0x00,\
                    //原点到预备位置基值 高八位
                    0x52,\
                    //原点到预备位置基值 低八位
                    0x19,\
                    //预备位置到针尖基值
                    0x8C,\
                    //夹子灵敏度
                    0x00,\
                    //拨叉回退时机微调 高八位
                    0x00};
                    //拨叉回退时机微调 低八位


//----采用查询方式IIC读写
Uint16 I2CA_WriteData( Uint16  RomAddress , Uint16 *Wdata, Uint16  number)
{
   Uint16 i;
   if (I2caRegs.I2CSTR.bit.BB == 1)
   {
      return I2C_BUS_BUSY_ERROR;
   }
   while(!I2C_xrdy());
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = number + 1;
   I2caRegs.I2CDXR = RomAddress;
   I2caRegs.I2CMDR.all = 0x2E20; //0x6E20;
   for (i=0; i<number; i++)
   {
      while(!I2C_xrdy());
      I2caRegs.I2CDXR = *Wdata;
      Wdata++;
      if (I2caRegs.I2CSTR.bit.NACK == 1)
          return    I2C_BUS_BUSY_ERROR;
   }
   return I2C_SUCCESS;
}

Uint16 I2CA_ReadData(Uint16 RomAddress, Uint16 *RamAddr , Uint16 number)
{
   Uint16  i,Temp;

   if (I2caRegs.I2CSTR.bit.BB == 1)//总线忙标志
   {
       return I2C_BUS_BUSY_ERROR;
   }
   while(!I2C_xrdy()); //
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = 1;
   I2caRegs.I2CDXR = RomAddress;
   I2caRegs.I2CMDR.all = 0x2620;//0x6620;
   if (I2caRegs.I2CSTR.bit.NACK == 1)
        return  I2C_BUS_BUSY_ERROR;

   DELAY_US(250);
   while(!I2C_xrdy());
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = number;
   I2caRegs.I2CMDR.all = 0x2C20;//0x6C20
   if (I2caRegs.I2CSTR.bit.NACK == 1)
        return  I2C_BUS_BUSY_ERROR;

   for(i=0;i<number;i++)
   {
      while(!I2C_rrdy());
      Temp = I2caRegs.I2CDRR;
      if (I2caRegs.I2CSTR.bit.NACK == 1)
          return    I2C_BUS_BUSY_ERROR;
      *RamAddr = Temp;
      RamAddr++;
   }
   return I2C_SUCCESS;
}

Uint16  I2C_xrdy()
{
    Uint16  t;
    t = I2caRegs.I2CSTR.bit.XRDY; //发送数据已准备好
    return t;
}

Uint16  I2C_rrdy()
{
    Uint16  t;
    t = I2caRegs.I2CSTR.bit.RRDY;//接收数据已准备好
    return t;
}
/*
void Write_Par(Uint16 WriteData)
{
   Wri_eprom_addr = 0xfa;
   I2CA_WriteData(Wri_eprom_addr,&WriteData,1);
   DELAY_US(5000);
}

Uint16 Read_Par()
{
    Uint16 I2C_state;
    Uint16 ReadData;
    I2C_state=I2CA_ReadData(0xfa,&ReadData,1);
    while(I2C_state!=I2C_SUCCESS)
	{;}
    return ReadData;
}
*/
Uint16 Read_Par(Uint16 Read_eprom_addr)
{
    Uint16 I2C_state=0;
    Uint16 ReadData=0;
    I2C_state=I2CA_ReadData(Read_eprom_addr,&ReadData,1);
    while(I2C_state!=I2C_SUCCESS)
	{;}
    return ReadData;
}
void Write_Par(Uint16 Wri_eprom_addr,Uint16 WriteData)
{
   //Wri_eprom_addr = 0xfb;0D
   I2CA_WriteData(Wri_eprom_addr,&WriteData,1);
   DELAY_US(5000);
}


void InitI2C_NOINT(void)
{
//********Step2: IIC 模块初始化**********************//
// Initialize I2C
   I2caRegs.I2CMDR.bit.IRS =0;      // Reset I2C
   I2caRegs.I2CSAR = 0x0050;        // Slave address - EEPROM control code
   I2caRegs.I2CPSC.all = 8;         // 60MHz/(5+1)=10MHz Prescaler - need 7-12 Mhz on module clk
   I2caRegs.I2CCLKL = 45;           // NOTE: must be non zero
   I2caRegs.I2CCLKH = 45;           // NOTE: must be non zero
   I2caRegs.I2CIER.all = 0x00;      // Disable interrupts
   I2caRegs.I2CMDR.bit.IRS = 1;     // Take I2C out of reset
}



//***********电机参数初始化******************
#pragma CODE_SECTION(Motor_Par_Init, "ramfuncs");
void Motor_Par_Init(void)
{
    register Uint16 I2C_state;
    I2C_state=I2CA_ReadData(eprom_addr_Reset,&I2CRD_PAR[0],2);
    while(I2C_state!=I2C_SUCCESS)
    {;}
    if((I2CRD_PAR[0]==0xff) && (I2CRD_PAR[1]==0xff))
    {
       DELAY_US(5000);
       Motor_Par_Reset(); //写入初始控制参数
       I2CWD_PAR[0] = 0xaa;
       I2CWD_PAR[1] = 0xaa;
       Wri_eprom_addr = eprom_addr_Reset;
       I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
     }

    DELAY_US(5000);
    Motor_Par_PowerOn_Read();//电机参数上电读取
}

//////////////////////////////////////////////////////////
#pragma CODE_SECTION(Wri_Rst_Par, "ramfuncs");
void Wri_Rst_Par(void) //写入初始化标志
{
   I2CWD_PAR[0] = 0xff;
   I2CWD_PAR[1] = 0xff;
   Wri_eprom_addr = eprom_addr_Reset;  //IIC地址
   I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
   DELAY_US(5000);
}

#pragma CODE_SECTION(Eeprom_Err_Wri, "ramfuncs");
void Eeprom_Err_Wri(void) //历史故障信息写入Eeprom
{
    Uint16 Err_num[4];

    //暂存当前历史故障信息
    Err_num[0] = MC.Err_Value;
    Err_num[1] = PA.Err_num1;
    Err_num[2] = PA.Err_num2;
    Err_num[3] = PA.Err_num3;

    //更新RAM中的历史故障信息
    PA.Err_num4 = PA.Err_num3;
    PA.Err_num3 = PA.Err_num2;
    PA.Err_num2 = PA.Err_num1;
    PA.Err_num1 = MC.Err_Value;

    //故障信息写入EEPROM
    I2CWD_PAR[0] = Err_num[0];//当前故障码
    I2CWD_PAR[1] = Err_num[1];//历史故障码1
    Wri_eprom_addr = eprom_addr_Error;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[2];//历史故障码2
    I2CWD_PAR[1] = Err_num[3];//历史故障码3
    Wri_eprom_addr = eprom_addr_Error+2;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}

#pragma CODE_SECTION(Eeprom_Err_Clear, "ramfuncs");
void Eeprom_Err_Clear(void)  //故障信息清零
{
    //将RAM中的故障信息清零
    PA.Err_num1=0;
    PA.Err_num2=0;
    PA.Err_num3=0;
    PA.Err_num4=0;

    //将EEPROM中的故障信息清零
    I2CWD_PAR[0] = 0;
    I2CWD_PAR[1] = 0;
    Wri_eprom_addr = eprom_addr_Error;  //
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = 0;
    I2CWD_PAR[1] = 0;
    Wri_eprom_addr = eprom_addr_Error+2;  //
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}

//*********电机参数复位***********//
#pragma CODE_SECTION(Motor_Par_Reset, "ramfuncs");
void Motor_Par_Reset(void)   //写入初始化参数
{
    register Uint16 i,Eeprom_Addr; //地址:0x0~0xFF(0~255)

    //写入驱动器初始PA参数
    for(i=0;i<Par_num;i++)
    {
       Eeprom_Addr=i;
       I2CA_WriteData(Eeprom_Addr,&Def_Par[i],1);
       DELAY_US(5000);
    }

    Eeprom_Err_Clear();  //故障信息清零

    #if(DSP_Update_Ena)
    I2CWD_PAR[0]= HardwareNumber;//地址0xFA写入DSP升级硬件识别码(MD612--0x06,0x07)
    Wri_eprom_addr = 0xFA;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
    DELAY_US(5000);

    I2CWD_PAR[0]= 0xFF;//地址0xFB下写0xFF， 清除升级标志位
    Wri_eprom_addr = 0xFB;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
    DELAY_US(5000);
    #endif
}

//*********电机参数上电读取***********//
void Motor_Par_PowerOn_Read(void)
 {
     register Uint16 I2C_state;
//-------------------------------------------------------------------------------
     I2C_state=I2CA_ReadData(0x00,&I2CRD_PAR[0],5);//每次读8个地址（0x0~0x8）
     while(I2C_state!=I2C_SUCCESS)   //等待FIFO读取EEPROM数据
     {;}
     PA.F1 = I2CRD_PAR[0]; //PA01--地址0x00
     PA.F2 = I2CRD_PAR[1]; //PA01--地址0x00
     PA.F3 = I2CRD_PAR[2]; //PA02--地址0x01
     PA.F4 = I2CRD_PAR[3]; //PA03--地址0x02
     PA.F5 = I2CRD_PAR[4]; //PA03--地址0x02
     DELAY_US(3000);
//-------------------------------------------------------------------------------
     //读取历史故障信息
     I2C_state=I2CA_ReadData(eprom_addr_Error,&I2CRD_PAR[0],4);
     while(I2C_state!=I2C_SUCCESS)
     {;}
     PA.Err_num1 = I2CRD_PAR[0];  //--历史故障1
     PA.Err_num2 = I2CRD_PAR[1];  //--历史故障2
     PA.Err_num3 = I2CRD_PAR[2];  //--历史故障3
     PA.Err_num4 = I2CRD_PAR[3];  //--历史故障4
     DELAY_US(3000);
//-------------------------------------------------------------------------------
     #if(DSP_Update_Ena)
     //读取DSP在线升级数据
     I2C_state=I2CA_ReadData(0xFA,&I2CRD_PAR[0],2);//0xFA~0xFB
     while(I2C_state!=I2C_SUCCESS)  //
     {;}
     PA.Hardware_Code = I2CRD_PAR[0];   //IIC地址=0xFA--硬件识别码
     PA.Update_Flag   = I2CRD_PAR[1];   //IIC地址=0xFB--软件升级标志
     DELAY_US(3000);
     #endif
//-------------------------------------------------------------------------------
}
void Par_Key_Read(void)
 {
     register Uint16 I2C_state;
//-------------------------------------------------------------------------------
     I2C_state=I2CA_ReadData(0x00,&I2CRD_PAR[0],5);//每次读8个地址（0x0~0x8）
     while(I2C_state!=I2C_SUCCESS)   //等待FIFO读取EEPROM数据
     {;}
     PA.F1 = I2CRD_PAR[0]; //PA01--地址0x00
     PA.F2 = I2CRD_PAR[1]; //PA01--地址0x00
     PA.F3 = I2CRD_PAR[2]; //PA02--地址0x01
     PA.F4 = I2CRD_PAR[3]; //PA03--地址0x02
     PA.F5 = I2CRD_PAR[4];
     DELAY_US(3000);
//-------------------------------------------------------------------------------
}
void LoopMode_Read(void)
{
    register Uint16 I2C_state;
    //-------------------------------------------------------------------------------
    I2C_state=I2CA_ReadData(0x05,&I2CRD_PAR[0],7);//每次读8个地址（0x0~0x8）
    while(I2C_state!=I2C_SUCCESS)   //等待FIFO读取EEPROM数据
    {;}
    HS.loopmode_Flag = I2CRD_PAR[0]; //PA01--地址0x05
    TZ.loopmode_Flag = I2CRD_PAR[1]; //PA01--地址0x06
    PA.Address = I2CRD_PAR[2]; //PA07--地址0x07
    HS.Color_state = I2CRD_PAR[3]; //PA07--地址0x07
    HS.ErrorStartFlag = I2CRD_PAR[4];
    Embroider.AirValvePos = I2CRD_PAR[5];
    Embroider.PearlNeedle = I2CRD_PAR[6];
    DELAY_US(3000);
    //-------------------------------------------------------------------------------
    if(HS.ErrorStartFlag == 1)
    {
        //读取重启前信息
        I2C_state=I2CA_ReadData(0x0C,&I2CRD_PAR[0],6);
        while(I2C_state!=I2C_SUCCESS)
        {;}
        TZ.Step1H = I2CRD_PAR[0];  //--历史故障1
        TZ.Step1L = I2CRD_PAR[1];  //--历史故障2
        TZ.Step2  = I2CRD_PAR[2];  //--历史故障3
        TZ.JiaZhu = I2CRD_PAR[3];  //--历史故障3
        TZ.DeltaTimeH = I2CRD_PAR[4];  //--历史故障4
        TZ.DeltaTimeL = I2CRD_PAR[5];  //--历史故障4
        DELAY_US(3000);
    }
}
void Par_Key_Write(void)   //写入初始化参数
{
    Uint16 Err_num[5];

    //暂存当前历史故障信息
    Err_num[0] = PA.F1;
    Err_num[1] = PA.F2;
    Err_num[2] = PA.F3;
    Err_num[3] = PA.F4;
    Err_num[4] = PA.F5;

    //故障信息写入EEPROM
    I2CWD_PAR[0] = Err_num[0];//当前故障码
    I2CWD_PAR[1] = Err_num[1];//历史故障码1

    I2CA_WriteData(0x00,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[2];//A珠下珠速度
    I2CWD_PAR[1] = Err_num[3];//历史故障码3

    I2CA_WriteData(0x02,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[4];//B珠下珠速度（0x04地址原来是C珠）

    I2CA_WriteData(0x04,&I2CWD_PAR[0],1);
    DELAY_US(3000);
}
void LoopMode_Write(Uint16 Data)   //写入初始化参数
{
    I2CWD_PAR[0] = Data;//当前故障码
    I2CWD_PAR[1] = Data;//历史故障码1

    I2CA_WriteData(0x05,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
/*void ColorState_Write(Uint16 Data) //写入当前珠子颜色状态(0x08)和重启标志位(0x09)
{
    I2CWD_PAR[0] = Data;//
    I2CWD_PAR[1] = 0x01;//记录故障重启标志
    I2CA_WriteData(0x08,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
void AirValveState_Write(Uint16 Data1,Uint16 Data2) //写入当前气阀状态（0x0A）和散珠针位信息(0x0B)
{
    I2CWD_PAR[0] = Data1;//
    I2CWD_PAR[1] = Data2;//
    I2CA_WriteData(0x0A,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}*/
void Machine_State_Write()
{
    I2CWD_PAR[0] = HS.Color_state;
    I2CWD_PAR[1] = 0x01;
    I2CA_WriteData(0x08,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Embroider.AirValvePos;//当前故障码
    I2CWD_PAR[1] = Embroider.PearlNeedle;
    I2CA_WriteData(0x0A,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = (abs(TZ.Step1)&0xFF00)>>8;//当前故障码
    I2CWD_PAR[1] = abs(TZ.Step1)&0x00FF;
    I2CA_WriteData(0x0C,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = abs(TZ.Step2);//当前故障码
    I2CWD_PAR[1] = TZ.JiaZhu;
    I2CA_WriteData(0x0E,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    if(TZ.DeltaTime >= 0)
    {
        I2CWD_PAR[0] = (TZ.DeltaTime&0xFF00)>>8;//当前故障码
        I2CWD_PAR[1] = TZ.DeltaTime&0x00FF;
    }
    else
    {
        I2CWD_PAR[0] = ((abs(TZ.DeltaTime)&0xFF00)>>8)|0x80;
        I2CWD_PAR[1] = abs(TZ.DeltaTime)&0x00FF;
    }
    I2CA_WriteData(0x10,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
void RestartFlag_Write()   //写入初始化参数
{
    I2CWD_PAR[0] = 0x00;//当前故障码
    I2CWD_PAR[1] = 0x00;
    I2CA_WriteData(0x08,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = 0x01;//当前故障码
    I2CWD_PAR[1] = 0x00;
    I2CA_WriteData(0x0A,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
