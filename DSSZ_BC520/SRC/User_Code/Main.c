/* ==============================================================================
System Name:    Stepper
File Name:      Steper.C
Description:    Close loop control of Stepper Motor
Version    Date        Author      Description
  1.0    2019.01.02    LiuDong        use
=================================================================================
(1)
(2)
������ʽ����������׸񲽽����F42-H40��
����裺0ŷ                          ���У�0mH
 �ת�أ�0.46Nm     �������1.3A
 ת��������260gcm2    ��������� 0.2Kg
DC24V����--������������ع��ģ�1.2W;  �ȴ����ģ�2.1W
�����Ʋ����ֵ(��Ƶ������PWM): imax=Ud/(8*La*fpwm)=24/(8*3*40)=0.025A
============================================================================*/

//1����ͷ�ļ�
#include "F2806x_Device.h"  //DSP2806x���趨��ͷ�ļ�
#include "Stepper-Settings.h" //��������ͷ�ļ�
#include "SPTA.h"
#include "GzTest.h"

//2������������
void DeviceInit();//�����ʼ������
void MemCopy();
void LED_Flash(void);//Led����
void Embroider_Handle(void);
void Pearl_Change_Handle(void);
void Pearl_Change_Time_Check(void);
void Pearl_Push_Handle(void);
void ALL_Header_Push_Pearl_Handle(void);
void ReadHeaderLight(void);//��ȡ��ͷ��״̬
extern void HandleHeaderAddress(void);
extern void Task_Handle_B(void);
void KeyRead(void);  //�жϰ���״̬���ӳ���
void KEY_Long_Handle(void);
void CubeDispaly_Handle(void);
Uint16 MS_SpeedCalCnt = 0;//ת�ټ������
extern SPEED_MEAS_QEP speed_MS;
extern CURVEGEN curve1;
extern Uint16 Con_Init_End;
extern _iq BM_Open_Id;
extern Uint16 Y_Mor_Ena;
extern Uint16 B_Mor_Ena;
//Uint16 T_Sw=0;
//Uint16 T_Start=0;
Uint16 T_Pos_Set=1;
Uint16 Emb_Index=0;
Uint16 Head_En=0;
Uint16 LightState=OFF;
Uint16 LightState_Last=GREEN;
//Uint16 Motor_Ctl_En=0;
//3��������������
extern TuiZhu_PARA TZ;
extern HuanSe_PARA HS;
extern Uint16 ATrg;
extern Uint16 BTrg;
extern Uint16 ATrgCnt;
//extern Uint16 BTrgCnt;

Uint16 OpenLoopDelay = 10000;

//#define OriginWaitTime 700
#define OriginWaitTime 300

extern Uint16 Change_Pearl_Color(void);
extern Uint16 Push_Pearl(Uint16 Dir);
extern Uint16 OPTO_Check(Uint16 Number);
void PrePearlHandle(void);
void AirValveHandle(void);
void Par_Key_Read(void);
void Par_Key_Write(void);

void PowerOnOff_Handle(void);
void Pearl_Continuity_Check(void);

void GZ_TEST_SM(void);
void GZ_TEST_Handle(void);
void GZ_TEST_CAN_Handle(void);
void Origa_ID_Check(void);

Uint16 RecordFlag=0;
Uint16 RecordDelay=0;
Uint32 RecordQEP=0;
int32  RecordDelta = 0;
Uint16 RecordCnt = 0;

Uint32 RecordQEP_old=0;

extern Uint16 Pearl_Position_State;
extern Uint16 BCorigin;
extern Uint16 ClearPulseFlag;//��������������־

Uint16 OTP3_DOWN=0;
Uint16 OTP3_RISE=0;
Uint16 OTP3_HIS=0;
Uint32 MotorE_HIS = 0;
Uint16 PARA=16;

Uint16 FILEFORMAT = 20;

#define FindOriginType Edge
#define Filter 1
#define Edge 2

void main(void)
{
//step1: ϵͳʱ�Ӽ�GPIO����
    //DeviceInit();
	InitSysCtrl();

	// Step 2. Initialize GPIO:
	// This example function is found in the F2806x_Gpio.c file and/
	// illustrates how to set the GPIO to it's default state.
	  InitGpio();  // Skipped for this example

	// Step 3. Clear all interrupts and initialize PIE vector table:
	// Disable CPU interrupts
	   DINT;

	// Initialize the PIE control registers to their default state.
	// The default state is all PIE interrupts disabled and flags
	// are cleared.
	// This function is found in the F2806x_PieCtrl.c file.
	   InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	   IER = 0x0000;
	   IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in F2806x_DefaultIsr.c.
	// This function is found in F2806x_PieVect.c.
	InitPieVectTable();
    PWM1_OFF_MACRO() //��ֹPWM���,��λDrv8412
	PWM2_OFF_MACRO()

//step2: ����Ҫ�����Flash����RAM������
    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
    MemCopy(&IQmathLoadStart, &IQmathLoadEnd, &IQmathRunStart);
    InitFlash();     //Flash��ʼ��

#if(Origa_Ena)
    Origa_ID_Check();
#endif

    //��ȡDIP1 DIP2
    INPUT_HC597Regs.all = READ_HC597();

//step3: DSP������������ʼ����ͨѶ�������ʼ����CAN,SCI,SPI��
    Motor_Ctrl_Init(); //������������ʼ��(ADC,ePWM,eQEP,eCAP),���Ʋ�����ʼ��

    //CPU��ʱ��0---(δʹ��)
    //CPU��ʱ��1---(���ڲ���10kHZ��ʱ)
    //CPU��ʱ��2---(���ڲ��Գ�������ʱ��)
    CpuTimer1Regs.PRD.all = mSec0_1; // 10kHz

//step4������74HC595����
    LED_HC595SendData(0x0040);//Ĭ��״̬��ȷ�ϲ�������ʾ��-��

    HC595Regs.bit.M1_ATE = 1;
    HC595Regs.bit.M1_TOFF = 0;
    HC595Regs.bit.M1_TQ0 = 1;
    HC595Regs.bit.M1_TQ1 = 0;
    HC595Regs.bit.M2_ATE = 1;
    HC595Regs.bit.M2_DY0 = 0;
    HC595Regs.bit.M2_DY1 = 0;
    HC595Regs.bit.M2_TOFF = 0;
    HC595Regs.bit.M2_TQ0 = 1;
    HC595Regs.bit.M2_TQ1 = 0;
    HC595Regs.bit.M3_ATE = 1;
    HC595Regs.bit.M3_DY0 = 0;
    HC595Regs.bit.M3_DY1 = 0;
    HC595Regs.bit.M3_TOFF = 0;
    HC595Regs.bit.M3_TQ0 = 1;
    HC595Regs.bit.M3_TQ1 = 1;
    HC595SendData(HC595Regs.all);
    M3_EN = 1;
#if 1
    LoopMode_Read();
    if((HS.loopmode_Flag == 0xB1)&&(TZ.loopmode_Flag == 0xB1))
    {
        HS.loopmode = 1;
        TZ.loopmode = 1;
        LoopMode_Write(0xB0);
    }
    else if((HS.loopmode_Flag == 0xB0)&&(TZ.loopmode_Flag == 0xB0))
    {
        HS.loopmode = 0;
        TZ.loopmode = 0;
    }
    else
    {
        LoopMode_Write(0xB0);
    }
#endif
//step5: �ж�����

    //ȫ���ж�ʹ��
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

//step5: ���������ѭ��
   for(;;)  //infinite loop--���� for(;;)�� while��1��Ч�ʸ�
   {
       DecryptHandle();

#if(FindOriginType == Edge)
       if((INPUT_HC597Regs.bit.OPTO3 == 0)&&(OTP3_HIS != INPUT_HC597Regs.bit.OPTO3))
       {
           OTP3_DOWN = 1;
           OTP3_RISE = 0;
           //LED_RED_BLINK; //�����Ƶ���˸
           MotorE_HIS = motorE.step_count;
       }
       else if((INPUT_HC597Regs.bit.OPTO3 == 1)&&(OTP3_HIS != INPUT_HC597Regs.bit.OPTO3))
       {
           OTP3_DOWN = 0;
           OTP3_RISE = 1;
           //LED_RED_BLINK; //�����Ƶ���˸
           MotorE_HIS = motorE.step_count;
       }
       OTP3_HIS = INPUT_HC597Regs.bit.OPTO3;
#endif

      if(CpuTimer1Regs.TCR.bit.TIF == 1)//10kHz��������
      {
          CpuTimer1Regs.TCR.bit.TIF = 1; // clear flag

          //-------Step0: ����ϵ�����---------------//
          PowerOnOff_Handle();

          //-------Step1: ���ϼ��---------------//
           Motor_Fault_Check();

          //------Step2: �������״̬ת������------//
           Motor_Ctrl_FSM();

          //------Step3: ��ͷ��ַ����------//
           HandleHeaderAddress();

          //------Step4: 597�����źŴ���------//
           INPUT_HC597Regs.all = READ_HC597();

          //------Step5: �������ʾ�������ƴ���------//
           CubeDispaly_Handle();

           //------Step6: �ֶ��������鼰����------//
           KEY_Hand_Handle();

           //------Step7: ��������ҵ����------//
           PrePearlHandle();

           //------Step8: ������������------------//
           if(Embroider.LightState != 1)//�ػ�ͷ������������������ 2019.09.10
           {AirValveHandle();}

           //------Step9: CAN������------------//
           CAN_Handle();

           //------Step10: Led����-------------//
           Embroider.LightState = (GpioDataRegs.GPADAT.bit.GPIO5 << 1)+ GpioDataRegs.GPADAT.bit.GPIO20 + 1;
           LED_Flash();

           //------Step11: ����ʱ���ж�-------------//
           Pearl_Change_Time_Check();

           //------Step12: �����������-------------//
           Pearl_Continuity_Check();
           //------Step13: bc520��װ����-------------//
           GZ_TEST_SM();
           GZ_TEST_Handle();
           //------Step14: ȫͷ���鹦��-------------//
           ALL_Header_Push_Pearl_Handle();
      }
   }//end of for(;;)
} //END MAIN CODE


//--------�������״̬��------------------//
#pragma CODE_SECTION(Motor_Ctrl_FSM, "ramfuncs");
void Motor_Ctrl_FSM(void)
{
    static Uint16 Delay_CNt=0;

    if(MC.S_ERR==1)
    {
        MC.State = ST_Err_Y;  /*��ת������ͣ��*/
    }
    /*if((MC.S_ERR==2)||(MB.S_ERR==1))
    {
        MC.State = ST_Err_B;  ��ת������ͣ��
    }*/

    switch(MC.State)
    {
    case(ST_Idle):  //��ʼ�����ȴ�״̬
               if(MC.RunMode == Run_Closeloop)//�ȴ�ʹ�ܱ�־
               {
                   PWM1_ON_MACRO()  //ʹ��PWM���
                   PWM2_ON_MACRO()  //ʹ��PWM���
                   DELAY_US(10);

                   MC.EnableFlag = 1;
                   MB.EnableFlag = 1;

                   EQep1Regs.QPOSCNT = 0;
                   EQep2Regs.QPOSCNT = 0;

                   MC.State = ST_RDY;//��ת����Run����������״̬
               }
               else if(MC.RunMode == Run_Test)
               {
                   PWM1_ON_MACRO()  //ʹ��PWM���
		           PWM2_ON_MACRO()  //ʹ��PWM���
		           DELAY_US(10);

                   MC.EnableFlag = 1;
                   MB.EnableFlag = 1;

                   MC.State = ST_Run;//��ת����Run����������״̬
               }
               else
               {
                   PWM1_OFF_MACRO()
		           PWM2_OFF_MACRO()
               }
    break;
    ////////////////////////////////////////////////////////////////
    case(ST_RDY):
                    MC.State = wait;//��ת����Run����������״̬
    break;
    //-------------------------------------------------------------------
    case (wait):
		        if(Con_Init_End==1)
		        {
		            if(Delay_CNt++>10000)//1s
		            {
		                MC.State = ST_Run;//��ת����Run����������״̬
		            }
		        }
    break;

    //---------------------------------------------------//
    case(ST_Run):  //��������״̬
    if(MC.Stop_bit==1)
    {
        MC.State = ST_Err_Y;  //����ͣ��
        //(*DSP_Soft_Reset)();
    }
    if(MB.Stop_bit==1)
    {
        //MC.State = ST_Err_B;  //����ͣ��
        //(*DSP_Soft_Reset)();
    }
    if(MC.RunMode == Run_Closeloop)
    {
        //if(Embroider.PearlNeedle != 0)//��ɢ����λ��ִ�л�ɫ������ҵ�� 2019.03.17
        if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
        {
            if(GZ.Enable != 1)//�ǹ�װ����ģʽ
                Embroider_Handle();
        }
    }
    else if(MC.RunMode == Run_Test)
    {
    }
    break;
    //---------------------------------------------------//
    case(ST_Err_Y):  //����ͣ��״̬
       if(HS.ErrorEnable == 0)
       {
           HS.ErrorEnable = 1;
           if(MC.Err_Value == 4)
           {
               KeyHandle.DisplayData = 0x007F;//��ɫ������ʾ��8��
               KeyHandle.DisplayEnable = 1;
           }
           else
           {
               KeyHandle.DisplayData = 0x0079;//��ɫ������ʾ��E��
               KeyHandle.DisplayEnable = 1;
           }
           if(Embroider.LightState != 1)
           {
               //if(Embroider.PearlNeedle != 0)
               if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
               {
                   CAN_TX_DLC2(0x0D98);
               }
           }
       }
       else
       {
           if(Embroider.StartStop == 1)
           {
               if(Embroider.LightState != 1)
               {
                   //if(Embroider.PearlNeedle != 0)
                   if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
                   {
                       if(HS.ErrCnt < 2)//��2�κ��ٷ�0D98
                       {
                           CAN_TX_DLC2(0x0D98);
                           HS.ErrCnt++;
                       }
                   }
               }
           }
       }
       PWM1_OFF_MACRO()  //����ͣ������������MOSFET���ر�Drv8412
    break;
    case(ST_Err_B):  //����ͣ��״̬
	   /*if(TZ.ErrorEnable == 0)
	   {
	       TZ.ErrorEnable = 1;
	       KeyHandle.DisplayData = 0x0071;//���汨����ʾ��F��
	       KeyHandle.DisplayEnable = 1;
	       if(Embroider.LightState != 1)
	       {
	           //CAN_TX_TZ();//�����ط��ͱ�����Ϣ
	           //ɢ�鱨��ͣ��(�����ش���)
	           CAN_TX_DLC2(0x0D98);

	       }
	   }
	   else
	   {
	       if(Embroider.StartStop == 1)
	       {
	           if(Embroider.LightState != 1)
	           {
	               if(TZ.ErrCnt < 2)//��2�κ��ٷ�0D98
	               {
	                   //CAN_TX_TZ();//�����ط��ͱ�����Ϣ
	                   CAN_TX_DLC2(0x0D98);
	                   TZ.ErrCnt++;
	               }
	           }
	       }
	   }*/
       PWM2_OFF_MACRO()  //����ͣ������������MOSFET���ر�Drv8412
    break;
    //----------------------------------------------------//
    default:
        break;
    }//end of switch()
}

//------------------------------------------------------//
//--LED���ƣ����ݿ���ģʽ���ı�LED��˸Ƶ�ʣ�����ʱLED����
#pragma CODE_SECTION(LED_Flash, "ramfuncs");
void LED_Flash(void)
{
     static Uint16 Led_Ticker=0;
     static Uint16 Led_Time=0;
     static Uint16 Key_Ticker = 0;

     if(MC.S_ERR==1)//����Led��˸Ƶ��
     {
    	 Led_Time = 5000;
     }//��������ʱ���������̵�
     else
     {
    	 Led_Time = 10000;
     }

     Led_Ticker++;
     if(Led_Ticker>Led_Time)
     {
    	 Led_Ticker = 0;
    	 LED_RED_BLINK; //�����Ƶ���˸
     }

     Key_Ticker++;
     if(Key_Ticker > 200)
     {
         Key_Ticker = 0;
         KeyRead();
         KEY_Long_Handle();
     }

     if(MC.S_ERR==2)
     {
        LED_FLT_OUT;//�й���,�������ϵ�
     }
}

//---------------------------------------------------------------------------------

//----------------------------------------------------------------------
void Embroider_Handle(void)
{
    if(TZ.HandTZFlag == 1)
    {
        TZ.HandTZDelay++;
        if(TZ.HandTZDelay >= 100000)//10s�Ժ��л�ΪС������ԭ��Ϊ0.1s,2019.07.20
        //if(TZ.HandTZDelay >= 1000)//0.1s,2020.02.10
        {
            TZ.HandTZDelay = 0;
            TZ.HandTZFlag = 0;
            TZ.MotorEnableFlag = 1;
        }
    }
    if(TZ.MotorEnableFlag == 1)
    {
        //M3_DIS = 1;//����
        HC595Regs.bit.M3_TQ0 = 1;
        HC595Regs.bit.M3_TQ1 = 1;
        HC595SendData(HC595Regs.all);
        TZ.MotorEnableFlag = 0;
    }
    //Embroider.PearlNeedle = 2;
    //if(Embroider.PearlNeedle != 0)//��ɢ����λ��ִ�л�ɫ������ҵ�� 2019.03.17
    {
        switch(Embroider.WorkState)
        {
        case 0x01://�ػ�ͷ
            if(Embroider.LightFlag == 1)
            {
                Embroider.LightFlag = 0;
                if(Embroider.AirValvePos == 0)
                {
                    Air1_Valve_Set = 1;
                    Air2_Valve_Set = 1;
                    Embroider.AirValvePos = 1;
                    CAN_TX_DLC2(0xF710);
                }
            }
            if((BCorigin == 1)&&(ClearPulseFlag == 0))
            {
                motorE.speedenbale = 0;
                motorE.step_state = 4;
                motorE.step_count = 0;
                TZ.Position_state = Push_Pearl(3);
                ClearPulseFlag = 1;
            }
            break;
            //--------------------------------------------------
        case 0x02://�̵�����
            //��������йػ�ͷ�ٿ���ͷ��װ�õ����⣨��ʤ���� 2019.08.26��
            if(Embroider.LightFlag == 0)
            {
                CAN_TX_DLC2(0xF710);
            }
            Embroider.LightFlag = 1;
            if(Embroider.NoEmbFlag == 1)
            {
                if(Embroider.AirValveState == 0)
                {
                    Air1_Valve_Clc = 1;
                    Air2_Valve_Clc = 1;
                    Embroider.AirValvePos = 0;
                }
                Embroider.NoEmbFlag = 0;
            }
            Pearl_Change_Handle();
            Pearl_Push_Handle();
            break;
            //--------------------------------------------------
        case 0x03://�������
            Pearl_Change_Handle();//��ɫʱ����ƣ����µ�һ�����Ӳ��ỻɫ 2019.02.25
            Pearl_Push_Handle();//��ɫʱ����ƣ����µ�һ������©�� 2019.02.26
            break;
            //--------------------------------------------------
        case 0x5://�ػ�ͷ
            if(Embroider.LightFlag == 1)
            {
                Embroider.LightFlag = 0;
                if(Embroider.AirValvePos == 0)
                {
                    Air1_Valve_Set = 1;
                    Air2_Valve_Set = 1;
                    Embroider.AirValvePos = 1;
                    CAN_TX_DLC2(0xF710);
                }
            }
            break;
            //--------------------------------------------------
        case 0x6://�ǲ���ͷ���̵Ʋ��壩
            Embroider.NoEmbFlag = 1;
            Pearl_Push_Handle();
            break;
            //--------------------------------------------------
        case 0x7://����ͷ����Ʋ��壩
            if(Embroider.NoEmbFlag == 1)
            {
                if(Embroider.AirValveState == 0)
                {
                    Air1_Valve_Clc = 1;
                    Air2_Valve_Clc = 1;
                    Embroider.AirValvePos = 0;
                }
                Embroider.NoEmbFlag = 0;
            }
            Embroider.LightFlag = 1;
            Pearl_Change_Handle();
            Pearl_Push_Handle();
            break;
        default:
            break;
            //--------------------------------------------------
        }
    }
	//Embroider.LightState = (GpioDataRegs.GPADAT.bit.GPIO5 << 1)+ GpioDataRegs.GPADAT.bit.GPIO20 + 1;

	//���Բ������Ƿ�����
	if(Embroider.LightState != Embroider.LightStateOld)
	{
	    switch(Embroider.LightState)
	    {
	    case 1:
	        KeyHandle.DisplayData = 0x0000;//�ػ�ͷ����ʾ
	        KeyHandle.DisplayEnable = 1;
	        break;
	    case 2:
	        KeyHandle.DisplayData = 0x0040;//�̵���ʾ��-��
	        KeyHandle.DisplayEnable = 1;
	        break;
	    case 3:
	        KeyHandle.DisplayData = 0x0076;//�����ʾ��H��
	        KeyHandle.DisplayEnable = 1;
	        break;
	    }
	    Embroider.LightStateOld = Embroider.LightState;
	}

	//if((TZ.Position_state == 3)&&(TZ.state == 2))
	{
	    Embroider.WorkState = (Embroider.ControlState << 2) + Embroider.LightState;
	}
	if(TZ.state == 0)
	{
	    if(MC.Zero_Find_Flag == 1)
	    {
	        TZ.InitDelay++;
	        if(TZ.InitDelay >= 5000)
	        {
	            TZ.InitDelay = 0;
	            TZ.state = 1;
	            //Pearl_Position_State = 1;
	            //TZ.Position_state = 1;
	            //��¼��ɫ���λ��
	            HS.InitPosData = EQep1Regs.QPOSCNT;
	            //TZ.cmd = 1;
	            TZ.cmd = 4;//��ǰ��һ����������ֹ��ͬλ���ϵ�������쳣 2019.07.22
	        }
	    }
	}
	if(TZ.state == 1)
	{
	    TZ.InitDelay++;
	    if(TZ.InitDelay >= 5000)
	    {
	        TZ.InitDelay = 0;
	        TZ.cmd = 2;
	        TZ.state = 2;
	    }
	}
}
void Pearl_Change_Handle(void)
{
    //����CAN����->��ɫ����
    if(HS.cmd == 1)
    {
        HS.cmd = 0;
        if(HS.Color_state == 0)
        {}
        else if(HS.Color_state == 1)
        {
            HS.Color_state = Change_Pearl_Color();
        }
    }
    else if(HS.cmd == 2)
    {
        HS.cmd = 0;
        if(HS.Color_state == 0)
        {
            HS.Color_state = Change_Pearl_Color();
        }
        else if(HS.Color_state == 1)
        {}
    }
}
//����һ��������ʱ�ȴ�60ms�󲦲�ǰ��
//�����������˵�λ��ȴ�5ms�󲦲�ǰ��
#define BackPlan 1

void Pearl_Push_Handle(void)
{
    //------------����ҵ������---------------
    if(TZ.cmd == 0)
    {
        if(TZ.OriginStopFlag == 1)
        {
#if(FindOriginType == Filter)
            if(BCorigin == 1)
            {
                TZ.ErrFlag = 0;
                TZ.ERRTZDelay = 0;
                motorE.speedenbale = 0;
                motorE.step_state = 4;
                if(KeyHandle.OkTrgCnt == 2)//�ֶ�
                {
                    TZ.OriginStopFlag = 0;
                }
                else//�Զ�
                {
                    TZ.OriginStopCnt++;
                    if(TZ.OriginStopCnt >= OriginWaitTime)
                    {
                        TZ.OriginStopCnt = 0;
                        TZ.cmd = 3;
                        //TZ.HandTZFlag = 1;
                        TZ.OriginStopFlag = 0;
                    }
                }
            }
#elif(FindOriginType == Edge)
            if((OTP3_DOWN == 1)&&(OTP3_RISE ==0))
            {
                if((motorE.step_count - MotorE_HIS) > PARA)
                {
                    TZ.ErrFlag = 0;
                    TZ.ERRTZDelay = 0;
                    motorE.speedenbale = 0;
                    motorE.step_state = 4;
                    MotorE_HIS = 0;
                    if(KeyHandle.OkTrgCnt == 2)//�ֶ�
                    {
                        motorE.step_count=0;
                        TZ.OriginStopFlag = 0;
                    }
                    else//�Զ�
                    {
                        TZ.OriginStopCnt++;
                        if(TZ.OriginStopCnt >= OriginWaitTime)
                        {
                            TZ.OriginStopCnt = 0;
                            motorE.step_count=0;
                            TZ.cmd = 3;
                            //TZ.HandTZFlag = 1;
                            TZ.OriginStopFlag = 0;
                        }
                    }
                }
            }
            //��ʤ��ͷ�Ļ������Ĳ�����������������������˻��˲��������趨ֵ���������ܣ�2019.11.05
            if(motorE.TempFlag == 1)
            {
                M3_DIS=1;//������������
                motorE.step_state = 4;
                motorE.step_count = 0;
                KeyHandle.DisplayData = 0x0071;
                KeyHandle.DisplayEnable = 1;
                CAN_TX_DLC2(0x0D98);
                motorE.TempFlag = 0;
            }
#endif
        }
    }
    else if(TZ.cmd == 1)
    {
        if((TZ.state==2)&&((BCorigin == 1)||(TZ.ErrFlag > 2)))//����ʱ���ֻ���ԭ�������һ��û�лص�ԭ��
        {
            M3_DIS=1;//������������
            motorE.step_state = 4;
            motorE.step_count = 0;
            KeyHandle.DisplayData = 0x0071;
            KeyHandle.DisplayEnable = 1;
            CAN_TX_DLC2(0x0D98);
        }
        else
        {TZ.Position_state = Push_Pearl(TZ.cmd);}
        TZ.ErrFlag++;
        TZ.cmd = 0;
    }
    else if(TZ.cmd == 2)
    {
        if(TZ.ErrFlag <= 2){
            TZ.Position_state = Push_Pearl(TZ.cmd);
            TZ.OriginStopFlag = 1;
            TZ.ERRTZDelay = 0;
        }
        TZ.cmd = 0;
    }
    else if(TZ.cmd == 3)
    {
        TZ.Position_state = Push_Pearl(TZ.cmd);
        TZ.cmd = 0;
        TZ.HandTZFlag = 1;
    }
    else if(TZ.cmd == 4)
    {
        TZ.Position_state = Push_Pearl(TZ.cmd);
        TZ.cmd = 0;
    }
}

void ALL_Header_Push_Pearl_Handle(void)
{
    //------------ȫͷ����ҵ��---------------
    if((TZ.All_START == 0)&&(TZ.All_START_State == 0))
    {
        //�ȴ�ȫͷ����
    }
    else
    {
        switch(TZ.All_START_State)
        {
        case 0:
            TZ.All_START_State = 1;
            break;
        case 1:
            TZ.cmd = 1;
            TZ.All_START_State = 2;
            break;
        case 2:
            TZ.All_START_Delay++;
            if(TZ.All_START_Delay >= 1000)
            {
                TZ.All_START_Delay = 0;
                TZ.All_START_State = 3;
            }
            break;
        case 3:
            TZ.cmd = 2;
            TZ.All_START_State = 4;
            break;
        case 4:
            TZ.All_START_Delay++;
            if(TZ.All_START_Delay >= 3000)
            {
                TZ.All_START_Delay = 0;
                TZ.All_START_State = 0;
            }
            break;
        }
    }
}
#define ChangePosPoint 600
Uint16 ChangeColorDelay = 0;//���濪ʼ�������ʱ20ms�ٻ��飬��ֹ����
void Pearl_Change_Time_Check(void)
{
    /*TZ.QEPTargetValue = TZ.QEPInitValue - EQep2Regs.QPOSCNT;
    if(TZ.QEPTargetValue < -2000)
        TZ.QEPTargetValue = TZ.QEPTargetValue + 4000;*/

    if(TZ.Position_state == 3)
    {
        if(Embroider.StartStop == 1)//��̬���飨Ҫ�ο����鲦������λ�ã�
        {
            ChangeColorDelay++;
            if(ChangeColorDelay >= 100)//���濪ʼ�������ʱ20ms�ٻ��飬��ֹ����
            {
                ChangeColorDelay = 0;
                //TZ.Position_state = 0;
                if(HS.cmdFlag == 1)
                {
                    HS.cmdFlag = 0;
                    HS.cmd = 1;
                }
                if(HS.cmdFlag == 2)
                {
                    HS.cmdFlag = 0;
                    HS.cmd = 2;
                }
            }
        }
        else//��̬����
        {
            //TZ.Position_state = 0;
            if(HS.cmdFlag == 1)
            {
                HS.cmdFlag = 0;
                HS.cmd = 1;
            }
            if(HS.cmdFlag == 2)
            {
                HS.cmdFlag = 0;
                HS.cmd = 2;
            }
        }
    }
}
void PowerOnOff_Handle(void)
{
    if(HS.loopmode == 0)
    {
        OpenLoopDelay--;
        if(OpenLoopDelay <= 1)
        {
            PWM1_OFF_MACRO()
            PWM2_OFF_MACRO()
            LoopMode_Write(0xB1);
            DINT;
            (*DSP_Flash_Update)(); /*��ת��DSP Bootloader*/
        }
    }
    else
    {
        if(HS.ErrorStartFlag == 1)
        {
            if(HS.Delay == 1)
            {
                HS.Delay = 0;
                HS.Color_state_old = HS.Color_state;
                RestartFlag_Write(0x00);
                if(HS.Color_state == 1)
                    KeyHandle.UpTrg = 1;
                if(Embroider.AirValvePos == 0)
                    CAN_TX_DLC2(0xF710);//ֱ�ӽ������Ƚ�Σ�գ���Ϊ֪ͨ����
                    TZ.Step1 = -((TZ.Step1H<<8)+TZ.Step1L);
                    TZ.Step2 = -(TZ.Step2);
                    TZ.Step3 = -(TZ.Step1+TZ.Step2);
                    if((TZ.DeltaTimeH&0x80)==0x80)
                    {
                        TZ.DeltaTime = -(((TZ.DeltaTimeH&0x7F)<<8) + TZ.DeltaTimeL);
                    }
                    else
                    {
                        TZ.DeltaTime = (TZ.DeltaTimeH<<8) + TZ.DeltaTimeL;
                    }
            }
            else if(HS.Delay == 0)
            {}
            else
            {HS.Delay--;}
        }
    }
}
void CubeDispaly_Handle(void)
{
    //--------------��ɫ�ص�ԭ���������������ʾ��.��-------------------2019.06.26
    if((EQep1Regs.QPOSCNT != HS.QEP_HisData)&&((KeyHandle.DisplayData == 0x0040)||(KeyHandle.DisplayData == 0x00C0)))
    {
        //if(HS.InitPosData != EQep1Regs.QPOSCNT)
        if(((HS.InitPosData >= EQep1Regs.QPOSCNT)&&(HS.InitPosData - EQep1Regs.QPOSCNT > 3))||
                ((HS.InitPosData <= EQep1Regs.QPOSCNT)&&(EQep1Regs.QPOSCNT - HS.InitPosData > 3)))
        {
            KeyHandle.DisplayData = 0x0040;
            KeyHandle.DisplayEnable = 1;
        }
        else
        {
            KeyHandle.DisplayData = 0x00C0;
            KeyHandle.DisplayEnable = 1;
        }
    }
    HS.QEP_HisData = EQep1Regs.QPOSCNT;
    //--------------��ɫ�ص�ԭ���������������ʾ��.��-------------------2019.06.26

    if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
    {
        if(KeyHandle.DisplayEnable == 1)
        {
            KeyHandle.DisplayEnable = 0;
            if(HS.InitPosData == EQep1Regs.QPOSCNT)
            {
                if(GZ.Enable != 1)
                    KeyHandle.DisplayData = KeyHandle.DisplayData|0x0080;
            }
            LED_HC595SendData(KeyHandle.DisplayData);
        }
    }
    else
    {
        LED_HC595SendData(0x0000);
    }
    if(Embroider.StartStopFlag == 1)//��������������־λ
    {
        Embroider.StartStopFlag = 0;
        if(MC.State == ST_Run)//��������ʱ�����ѽ����������ָܻ���Ĭ��״̬
        {
            KeyHandle.DisplayData = 0x0040;//"-"�ָ���ʾĬ��״̬
            KeyHandle.DisplayEnable = 1;
        }
    }
}
//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
