/*
 * I2C_Handle_Update.c
 *
 *  Created on: 2015��12��22��
 *      Author: liudong
 */

//======================================================================================
//         COPYRIGHT(C) 2009 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//  Project Number: pmsm_sensor_controller
//  File Name : I2C_handle.c
//  Description: ���I2C��д����
//  Version    Date     Author     Description
//  1.0    2015.01.24   wangdayu        use
//=========================================================================

//step1:ͷ�ļ�����
#include "F2806x_Device.h"
#include "F2806x_I2c_defines.h"
#include "Stepper-Settings.h"
//#define CPU_CLOCK_SPEED      9.000L   // 10.000L for a 100MHz CPU clock speed
#define ADC_usDELAY 10000L
//#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_CLOCK_SPEED) - 9.0L) / 5.0L)
//extern void DSP28x_usDelay(unsigned long Count);
void Motor_Par_Reset(void);   //д���ʼ������
void Motor_Par_PowerOn_Read(void);
//step2:��������
Uint16  I2CA_ReadData(Uint16 RomAddress,Uint16 *RamAddr, Uint16 number);
Uint16  I2CA_WriteData(Uint16 RomAddress,Uint16 *Wdata, Uint16 number);
Uint16  I2C_xrdy();
Uint16  I2C_rrdy();

//I2C��д�ñ���
Uint16 Wri_eprom_addr;
Uint16 I2CRD_PAR[8]={0}; //I2C��������
Uint16 I2CWD_PAR[2]={0}; //I2Cд������
//��ߴ������ʣ�400kbps

#define Par_num  18  //PA��������
//���������Ʋ���PA��IIC--128*16bit��--���ɴ�128����

//��ʼ�����Ʋ���
Uint16 Def_Par[Par_num]={
                    0x05,\
                    //IIC��ַ=0x00 F1:ԭ�㵽Ԥ��λ�þ������
                    0x05,\
                    //IIC��ַ=0x01 F2:Ԥ��λ�õ���׾������
                    0x05,\
                    //IIC��ַ=0x02 F3:A�������ٶȣ�2019.02.26
                    0x05,\
                    //IIC��ַ=0x03 F4(F1+F2):�����г�΢������
                    0x05,\
                    //Ԥ������->IIC��ַ=0x04 F5 B�������ٶȣ�2019.02.26
                    0xB0,\
                    //����������ģʽ��־λ
                    0xB0,\
                    //����������ģʽ��־λ
                    0x00,\
                    //��ͷ���ַ��Ϣ
                    0x00,\
                    //������ɫ��Ϣ
                    0x00,\
                    //����������־��Ϣ
                    0x01,\
                    //����״̬��Ϣ
                    0x00,\
                    //ɢ����λ��Ϣ
                    0x00,\
                    //ԭ�㵽Ԥ��λ�û�ֵ �߰�λ
                    0x52,\
                    //ԭ�㵽Ԥ��λ�û�ֵ �Ͱ�λ
                    0x19,\
                    //Ԥ��λ�õ�����ֵ
                    0x8C,\
                    //����������
                    0x00,\
                    //�������ʱ��΢�� �߰�λ
                    0x00};
                    //�������ʱ��΢�� �Ͱ�λ


//----���ò�ѯ��ʽIIC��д
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

   if (I2caRegs.I2CSTR.bit.BB == 1)//����æ��־
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
    t = I2caRegs.I2CSTR.bit.XRDY; //����������׼����
    return t;
}

Uint16  I2C_rrdy()
{
    Uint16  t;
    t = I2caRegs.I2CSTR.bit.RRDY;//����������׼����
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
//********Step2: IIC ģ���ʼ��**********************//
// Initialize I2C
   I2caRegs.I2CMDR.bit.IRS =0;      // Reset I2C
   I2caRegs.I2CSAR = 0x0050;        // Slave address - EEPROM control code
   I2caRegs.I2CPSC.all = 8;         // 60MHz/(5+1)=10MHz Prescaler - need 7-12 Mhz on module clk
   I2caRegs.I2CCLKL = 45;           // NOTE: must be non zero
   I2caRegs.I2CCLKH = 45;           // NOTE: must be non zero
   I2caRegs.I2CIER.all = 0x00;      // Disable interrupts
   I2caRegs.I2CMDR.bit.IRS = 1;     // Take I2C out of reset
}



//***********���������ʼ��******************
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
       Motor_Par_Reset(); //д���ʼ���Ʋ���
       I2CWD_PAR[0] = 0xaa;
       I2CWD_PAR[1] = 0xaa;
       Wri_eprom_addr = eprom_addr_Reset;
       I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
     }

    DELAY_US(5000);
    Motor_Par_PowerOn_Read();//��������ϵ��ȡ
}

//////////////////////////////////////////////////////////
#pragma CODE_SECTION(Wri_Rst_Par, "ramfuncs");
void Wri_Rst_Par(void) //д���ʼ����־
{
   I2CWD_PAR[0] = 0xff;
   I2CWD_PAR[1] = 0xff;
   Wri_eprom_addr = eprom_addr_Reset;  //IIC��ַ
   I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
   DELAY_US(5000);
}

#pragma CODE_SECTION(Eeprom_Err_Wri, "ramfuncs");
void Eeprom_Err_Wri(void) //��ʷ������Ϣд��Eeprom
{
    Uint16 Err_num[4];

    //�ݴ浱ǰ��ʷ������Ϣ
    Err_num[0] = MC.Err_Value;
    Err_num[1] = PA.Err_num1;
    Err_num[2] = PA.Err_num2;
    Err_num[3] = PA.Err_num3;

    //����RAM�е���ʷ������Ϣ
    PA.Err_num4 = PA.Err_num3;
    PA.Err_num3 = PA.Err_num2;
    PA.Err_num2 = PA.Err_num1;
    PA.Err_num1 = MC.Err_Value;

    //������Ϣд��EEPROM
    I2CWD_PAR[0] = Err_num[0];//��ǰ������
    I2CWD_PAR[1] = Err_num[1];//��ʷ������1
    Wri_eprom_addr = eprom_addr_Error;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[2];//��ʷ������2
    I2CWD_PAR[1] = Err_num[3];//��ʷ������3
    Wri_eprom_addr = eprom_addr_Error+2;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}

#pragma CODE_SECTION(Eeprom_Err_Clear, "ramfuncs");
void Eeprom_Err_Clear(void)  //������Ϣ����
{
    //��RAM�еĹ�����Ϣ����
    PA.Err_num1=0;
    PA.Err_num2=0;
    PA.Err_num3=0;
    PA.Err_num4=0;

    //��EEPROM�еĹ�����Ϣ����
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

//*********���������λ***********//
#pragma CODE_SECTION(Motor_Par_Reset, "ramfuncs");
void Motor_Par_Reset(void)   //д���ʼ������
{
    register Uint16 i,Eeprom_Addr; //��ַ:0x0~0xFF(0~255)

    //д����������ʼPA����
    for(i=0;i<Par_num;i++)
    {
       Eeprom_Addr=i;
       I2CA_WriteData(Eeprom_Addr,&Def_Par[i],1);
       DELAY_US(5000);
    }

    Eeprom_Err_Clear();  //������Ϣ����

    #if(DSP_Update_Ena)
    I2CWD_PAR[0]= HardwareNumber;//��ַ0xFAд��DSP����Ӳ��ʶ����(MD612--0x06,0x07)
    Wri_eprom_addr = 0xFA;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
    DELAY_US(5000);

    I2CWD_PAR[0]= 0xFF;//��ַ0xFB��д0xFF�� ���������־λ
    Wri_eprom_addr = 0xFB;
    I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
    DELAY_US(5000);
    #endif
}

//*********��������ϵ��ȡ***********//
void Motor_Par_PowerOn_Read(void)
 {
     register Uint16 I2C_state;
//-------------------------------------------------------------------------------
     I2C_state=I2CA_ReadData(0x00,&I2CRD_PAR[0],5);//ÿ�ζ�8����ַ��0x0~0x8��
     while(I2C_state!=I2C_SUCCESS)   //�ȴ�FIFO��ȡEEPROM����
     {;}
     PA.F1 = I2CRD_PAR[0]; //PA01--��ַ0x00
     PA.F2 = I2CRD_PAR[1]; //PA01--��ַ0x00
     PA.F3 = I2CRD_PAR[2]; //PA02--��ַ0x01
     PA.F4 = I2CRD_PAR[3]; //PA03--��ַ0x02
     PA.F5 = I2CRD_PAR[4]; //PA03--��ַ0x02
     DELAY_US(3000);
//-------------------------------------------------------------------------------
     //��ȡ��ʷ������Ϣ
     I2C_state=I2CA_ReadData(eprom_addr_Error,&I2CRD_PAR[0],4);
     while(I2C_state!=I2C_SUCCESS)
     {;}
     PA.Err_num1 = I2CRD_PAR[0];  //--��ʷ����1
     PA.Err_num2 = I2CRD_PAR[1];  //--��ʷ����2
     PA.Err_num3 = I2CRD_PAR[2];  //--��ʷ����3
     PA.Err_num4 = I2CRD_PAR[3];  //--��ʷ����4
     DELAY_US(3000);
//-------------------------------------------------------------------------------
     #if(DSP_Update_Ena)
     //��ȡDSP������������
     I2C_state=I2CA_ReadData(0xFA,&I2CRD_PAR[0],2);//0xFA~0xFB
     while(I2C_state!=I2C_SUCCESS)  //
     {;}
     PA.Hardware_Code = I2CRD_PAR[0];   //IIC��ַ=0xFA--Ӳ��ʶ����
     PA.Update_Flag   = I2CRD_PAR[1];   //IIC��ַ=0xFB--���������־
     DELAY_US(3000);
     #endif
//-------------------------------------------------------------------------------
}
void Par_Key_Read(void)
 {
     register Uint16 I2C_state;
//-------------------------------------------------------------------------------
     I2C_state=I2CA_ReadData(0x00,&I2CRD_PAR[0],5);//ÿ�ζ�8����ַ��0x0~0x8��
     while(I2C_state!=I2C_SUCCESS)   //�ȴ�FIFO��ȡEEPROM����
     {;}
     PA.F1 = I2CRD_PAR[0]; //PA01--��ַ0x00
     PA.F2 = I2CRD_PAR[1]; //PA01--��ַ0x00
     PA.F3 = I2CRD_PAR[2]; //PA02--��ַ0x01
     PA.F4 = I2CRD_PAR[3]; //PA03--��ַ0x02
     PA.F5 = I2CRD_PAR[4];
     DELAY_US(3000);
//-------------------------------------------------------------------------------
}
void LoopMode_Read(void)
{
    register Uint16 I2C_state;
    //-------------------------------------------------------------------------------
    I2C_state=I2CA_ReadData(0x05,&I2CRD_PAR[0],7);//ÿ�ζ�8����ַ��0x0~0x8��
    while(I2C_state!=I2C_SUCCESS)   //�ȴ�FIFO��ȡEEPROM����
    {;}
    HS.loopmode_Flag = I2CRD_PAR[0]; //PA01--��ַ0x05
    TZ.loopmode_Flag = I2CRD_PAR[1]; //PA01--��ַ0x06
    PA.Address = I2CRD_PAR[2]; //PA07--��ַ0x07
    HS.Color_state = I2CRD_PAR[3]; //PA07--��ַ0x07
    HS.ErrorStartFlag = I2CRD_PAR[4];
    Embroider.AirValvePos = I2CRD_PAR[5];
    Embroider.PearlNeedle = I2CRD_PAR[6];
    DELAY_US(3000);
    //-------------------------------------------------------------------------------
    if(HS.ErrorStartFlag == 1)
    {
        //��ȡ����ǰ��Ϣ
        I2C_state=I2CA_ReadData(0x0C,&I2CRD_PAR[0],6);
        while(I2C_state!=I2C_SUCCESS)
        {;}
        TZ.Step1H = I2CRD_PAR[0];  //--��ʷ����1
        TZ.Step1L = I2CRD_PAR[1];  //--��ʷ����2
        TZ.Step2  = I2CRD_PAR[2];  //--��ʷ����3
        TZ.JiaZhu = I2CRD_PAR[3];  //--��ʷ����3
        TZ.DeltaTimeH = I2CRD_PAR[4];  //--��ʷ����4
        TZ.DeltaTimeL = I2CRD_PAR[5];  //--��ʷ����4
        DELAY_US(3000);
    }
}
void Par_Key_Write(void)   //д���ʼ������
{
    Uint16 Err_num[5];

    //�ݴ浱ǰ��ʷ������Ϣ
    Err_num[0] = PA.F1;
    Err_num[1] = PA.F2;
    Err_num[2] = PA.F3;
    Err_num[3] = PA.F4;
    Err_num[4] = PA.F5;

    //������Ϣд��EEPROM
    I2CWD_PAR[0] = Err_num[0];//��ǰ������
    I2CWD_PAR[1] = Err_num[1];//��ʷ������1

    I2CA_WriteData(0x00,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[2];//A�������ٶ�
    I2CWD_PAR[1] = Err_num[3];//��ʷ������3

    I2CA_WriteData(0x02,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = Err_num[4];//B�������ٶȣ�0x04��ַԭ����C�飩

    I2CA_WriteData(0x04,&I2CWD_PAR[0],1);
    DELAY_US(3000);
}
void LoopMode_Write(Uint16 Data)   //д���ʼ������
{
    I2CWD_PAR[0] = Data;//��ǰ������
    I2CWD_PAR[1] = Data;//��ʷ������1

    I2CA_WriteData(0x05,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
/*void ColorState_Write(Uint16 Data) //д�뵱ǰ������ɫ״̬(0x08)��������־λ(0x09)
{
    I2CWD_PAR[0] = Data;//
    I2CWD_PAR[1] = 0x01;//��¼����������־
    I2CA_WriteData(0x08,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
void AirValveState_Write(Uint16 Data1,Uint16 Data2) //д�뵱ǰ����״̬��0x0A����ɢ����λ��Ϣ(0x0B)
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

    I2CWD_PAR[0] = Embroider.AirValvePos;//��ǰ������
    I2CWD_PAR[1] = Embroider.PearlNeedle;
    I2CA_WriteData(0x0A,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = (abs(TZ.Step1)&0xFF00)>>8;//��ǰ������
    I2CWD_PAR[1] = abs(TZ.Step1)&0x00FF;
    I2CA_WriteData(0x0C,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = abs(TZ.Step2);//��ǰ������
    I2CWD_PAR[1] = TZ.JiaZhu;
    I2CA_WriteData(0x0E,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    if(TZ.DeltaTime >= 0)
    {
        I2CWD_PAR[0] = (TZ.DeltaTime&0xFF00)>>8;//��ǰ������
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
void RestartFlag_Write()   //д���ʼ������
{
    I2CWD_PAR[0] = 0x00;//��ǰ������
    I2CWD_PAR[1] = 0x00;
    I2CA_WriteData(0x08,&I2CWD_PAR[0],2);
    DELAY_US(3000);

    I2CWD_PAR[0] = 0x01;//��ǰ������
    I2CWD_PAR[1] = 0x00;
    I2CA_WriteData(0x0A,&I2CWD_PAR[0],2);
    DELAY_US(3000);
}
