/* ==============================================================================
System Name:    Stepper
File Name:      Steper.C
Description:    Close loop control of Stepper Motor
Version    Date        Author      Description
  1.0    2019.01.02    LiuDong        use
=================================================================================
(1)
(2)
两相混合式步进电机（米格步进电机F42-H40）
相电阻：0欧                          相电感：0mH
 额定转矩：0.46Nm     额定电流：1.3A
 转动惯量：260gcm2    电机重量： 0.2Kg
DC24V供电--驱动器自身空载功耗：1.2W;  等待功耗：2.1W
电流纹波最大值(倍频单极性PWM): imax=Ud/(8*La*fpwm)=24/(8*3*40)=0.025A
============================================================================*/

//1——头文件
#include "F2806x_Device.h"  //DSP2806x外设定义头文件
#include "Stepper-Settings.h" //参数设置头文件
#include "SPTA.h"
#include "GzTest.h"

//2——函数声明
void DeviceInit();//外设初始化函数
void MemCopy();
void LED_Flash(void);//Led控制
void Embroider_Handle(void);
void Pearl_Change_Handle(void);
void Pearl_Change_Time_Check(void);
void Pearl_Push_Handle(void);
void ALL_Header_Push_Pearl_Handle(void);
void ReadHeaderLight(void);//读取机头灯状态
extern void HandleHeaderAddress(void);
extern void Task_Handle_B(void);
void KeyRead(void);  //判断按键状态的子程序
void KEY_Long_Handle(void);
void CubeDispaly_Handle(void);
Uint16 MS_SpeedCalCnt = 0;//转速计算计数
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
//3——主函数定义
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
extern Uint16 ClearPulseFlag;//清除拨叉电机脉冲标志

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
//step1: 系统时钟及GPIO设置
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
    PWM1_OFF_MACRO() //禁止PWM输出,复位Drv8412
	PWM2_OFF_MACRO()

//step2: 将重要程序从Flash载入RAM中运行
    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
    MemCopy(&IQmathLoadStart, &IQmathLoadEnd, &IQmathRunStart);
    InitFlash();     //Flash初始化

#if(Origa_Ena)
    Origa_ID_Check();
#endif

    //读取DIP1 DIP2
    INPUT_HC597Regs.all = READ_HC597();

//step3: DSP电机控制外设初始化，通讯类外设初始化（CAN,SCI,SPI）
    Motor_Ctrl_Init(); //电机控制外设初始化(ADC,ePWM,eQEP,eCAP),控制参数初始化

    //CPU定时器0---(未使用)
    //CPU定时器1---(用于产生10kHZ定时)
    //CPU定时器2---(用于测试程序运行时间)
    CpuTimer1Regs.PRD.all = mSec0_1; // 10kHz

//step4：设置74HC595参数
    LED_HC595SendData(0x0040);//默认状态及确认参数后显示“-”

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
//step5: 中断设置

    //全局中断使能
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

//step5: 主程序控制循环
   for(;;)  //infinite loop--采用 for(;;)比 while（1）效率高
   {
       DecryptHandle();

#if(FindOriginType == Edge)
       if((INPUT_HC597Regs.bit.OPTO3 == 0)&&(OTP3_HIS != INPUT_HC597Regs.bit.OPTO3))
       {
           OTP3_DOWN = 1;
           OTP3_RISE = 0;
           //LED_RED_BLINK; //呼吸灯灯闪烁
           MotorE_HIS = motorE.step_count;
       }
       else if((INPUT_HC597Regs.bit.OPTO3 == 1)&&(OTP3_HIS != INPUT_HC597Regs.bit.OPTO3))
       {
           OTP3_DOWN = 0;
           OTP3_RISE = 1;
           //LED_RED_BLINK; //呼吸灯灯闪烁
           MotorE_HIS = motorE.step_count;
       }
       OTP3_HIS = INPUT_HC597Regs.bit.OPTO3;
#endif

      if(CpuTimer1Regs.TCR.bit.TIF == 1)//10kHz控制任务
      {
          CpuTimer1Regs.TCR.bit.TIF = 1; // clear flag

          //-------Step0: 板件上电重启---------------//
          PowerOnOff_Handle();

          //-------Step1: 故障检测---------------//
           Motor_Fault_Check();

          //------Step2: 电机运行状态转换控制------//
           Motor_Ctrl_FSM();

          //------Step3: 机头地址处理------//
           HandleHeaderAddress();

          //------Step4: 597输入信号处理------//
           INPUT_HC597Regs.all = READ_HC597();

          //------Step5: 数码管显示及按键灯处理------//
           CubeDispaly_Handle();

           //------Step6: 手动按键换珠及推珠------//
           KEY_Hand_Handle();

           //------Step7: 开环拌珠业务处理------//
           PrePearlHandle();

           //------Step8: 气阀控制任务------------//
           if(Embroider.LightState != 1)//关机头后气阀按键不起作用 2019.09.10
           {AirValveHandle();}

           //------Step9: CAN任务处理------------//
           CAN_Handle();

           //------Step10: Led控制-------------//
           Embroider.LightState = (GpioDataRegs.GPADAT.bit.GPIO5 << 1)+ GpioDataRegs.GPADAT.bit.GPIO20 + 1;
           LED_Flash();

           //------Step11: 换珠时机判断-------------//
           Pearl_Change_Time_Check();

           //------Step12: 珠子连续检测-------------//
           Pearl_Continuity_Check();
           //------Step13: bc520工装测试-------------//
           GZ_TEST_SM();
           GZ_TEST_Handle();
           //------Step14: 全头换珠功能-------------//
           ALL_Header_Push_Pearl_Handle();
      }
   }//end of for(;;)
} //END MAIN CODE


//--------电机控制状态机------------------//
#pragma CODE_SECTION(Motor_Ctrl_FSM, "ramfuncs");
void Motor_Ctrl_FSM(void)
{
    static Uint16 Delay_CNt=0;

    if(MC.S_ERR==1)
    {
        MC.State = ST_Err_Y;  /*跳转到故障停机*/
    }
    /*if((MC.S_ERR==2)||(MB.S_ERR==1))
    {
        MC.State = ST_Err_B;  跳转到故障停机
    }*/

    switch(MC.State)
    {
    case(ST_Idle):  //初始化及等待状态
               if(MC.RunMode == Run_Closeloop)//等待使能标志
               {
                   PWM1_ON_MACRO()  //使能PWM输出
                   PWM2_ON_MACRO()  //使能PWM输出
                   DELAY_US(10);

                   MC.EnableFlag = 1;
                   MB.EnableFlag = 1;

                   EQep1Regs.QPOSCNT = 0;
                   EQep2Regs.QPOSCNT = 0;

                   MC.State = ST_RDY;//跳转到“Run”正常运行状态
               }
               else if(MC.RunMode == Run_Test)
               {
                   PWM1_ON_MACRO()  //使能PWM输出
		           PWM2_ON_MACRO()  //使能PWM输出
		           DELAY_US(10);

                   MC.EnableFlag = 1;
                   MB.EnableFlag = 1;

                   MC.State = ST_Run;//跳转到“Run”正常运行状态
               }
               else
               {
                   PWM1_OFF_MACRO()
		           PWM2_OFF_MACRO()
               }
    break;
    ////////////////////////////////////////////////////////////////
    case(ST_RDY):
                    MC.State = wait;//跳转到“Run”正常运行状态
    break;
    //-------------------------------------------------------------------
    case (wait):
		        if(Con_Init_End==1)
		        {
		            if(Delay_CNt++>10000)//1s
		            {
		                MC.State = ST_Run;//跳转到“Run”正常运行状态
		            }
		        }
    break;

    //---------------------------------------------------//
    case(ST_Run):  //正常运行状态
    if(MC.Stop_bit==1)
    {
        MC.State = ST_Err_Y;  //正常停机
        //(*DSP_Soft_Reset)();
    }
    if(MB.Stop_bit==1)
    {
        //MC.State = ST_Err_B;  //正常停机
        //(*DSP_Soft_Reset)();
    }
    if(MC.RunMode == Run_Closeloop)
    {
        //if(Embroider.PearlNeedle != 0)//在散珠针位才执行换色与推珠业务 2019.03.17
        if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
        {
            if(GZ.Enable != 1)//非工装调试模式
                Embroider_Handle();
        }
    }
    else if(MC.RunMode == Run_Test)
    {
    }
    break;
    //---------------------------------------------------//
    case(ST_Err_Y):  //故障停机状态
       if(HS.ErrorEnable == 0)
       {
           HS.ErrorEnable = 1;
           if(MC.Err_Value == 4)
           {
               KeyHandle.DisplayData = 0x007F;//换色报错显示‘8’
               KeyHandle.DisplayEnable = 1;
           }
           else
           {
               KeyHandle.DisplayData = 0x0079;//换色报错显示‘E’
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
                       if(HS.ErrCnt < 2)//发2次后不再发0D98
                       {
                           CAN_TX_DLC2(0x0D98);
                           HS.ErrCnt++;
                       }
                   }
               }
           }
       }
       PWM1_OFF_MACRO()  //故障停机，封锁所有MOSFET，关闭Drv8412
    break;
    case(ST_Err_B):  //故障停机状态
	   /*if(TZ.ErrorEnable == 0)
	   {
	       TZ.ErrorEnable = 1;
	       KeyHandle.DisplayData = 0x0071;//拨叉报错显示‘F’
	       KeyHandle.DisplayEnable = 1;
	       if(Embroider.LightState != 1)
	       {
	           //CAN_TX_TZ();//向主控发送报错信息
	           //散珠报错停车(不严重错误)
	           CAN_TX_DLC2(0x0D98);

	       }
	   }
	   else
	   {
	       if(Embroider.StartStop == 1)
	       {
	           if(Embroider.LightState != 1)
	           {
	               if(TZ.ErrCnt < 2)//发2次后不再发0D98
	               {
	                   //CAN_TX_TZ();//向主控发送报错信息
	                   CAN_TX_DLC2(0x0D98);
	                   TZ.ErrCnt++;
	               }
	           }
	       }
	   }*/
       PWM2_OFF_MACRO()  //故障停机，封锁所有MOSFET，关闭Drv8412
    break;
    //----------------------------------------------------//
    default:
        break;
    }//end of switch()
}

//------------------------------------------------------//
//--LED控制，根据控制模式，改变LED闪烁频率，故障时LED常亮
#pragma CODE_SECTION(LED_Flash, "ramfuncs");
void LED_Flash(void)
{
     static Uint16 Led_Ticker=0;
     static Uint16 Led_Time=0;
     static Uint16 Key_Ticker = 0;

     if(MC.S_ERR==1)//控制Led闪烁频率
     {
    	 Led_Time = 5000;
     }//正常工作时，慢速闪绿灯
     else
     {
    	 Led_Time = 10000;
     }

     Led_Ticker++;
     if(Led_Ticker>Led_Time)
     {
    	 Led_Ticker = 0;
    	 LED_RED_BLINK; //呼吸灯灯闪烁
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
        LED_FLT_OUT;//有故障,点亮故障灯
     }
}

//---------------------------------------------------------------------------------

//----------------------------------------------------------------------
void Embroider_Handle(void)
{
    if(TZ.HandTZFlag == 1)
    {
        TZ.HandTZDelay++;
        if(TZ.HandTZDelay >= 100000)//10s以后切换为小电流，原来为0.1s,2019.07.20
        //if(TZ.HandTZDelay >= 1000)//0.1s,2020.02.10
        {
            TZ.HandTZDelay = 0;
            TZ.HandTZFlag = 0;
            TZ.MotorEnableFlag = 1;
        }
    }
    if(TZ.MotorEnableFlag == 1)
    {
        //M3_DIS = 1;//松轴
        HC595Regs.bit.M3_TQ0 = 1;
        HC595Regs.bit.M3_TQ1 = 1;
        HC595SendData(HC595Regs.all);
        TZ.MotorEnableFlag = 0;
    }
    //Embroider.PearlNeedle = 2;
    //if(Embroider.PearlNeedle != 0)//在散珠针位才执行换色与推珠业务 2019.03.17
    {
        switch(Embroider.WorkState)
        {
        case 0x01://关机头
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
        case 0x02://绿灯正绣
            //解决绣作中关机头再开机头下装置的问题（信胜反馈 2019.08.26）
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
        case 0x03://红灯正绣
            Pearl_Change_Handle();//换色时亮红灯，导致第一颗珠子不会换色 2019.02.25
            Pearl_Push_Handle();//换色时亮红灯，导致第一颗珠子漏掉 2019.02.26
            break;
            //--------------------------------------------------
        case 0x5://关机头
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
        case 0x6://非补绣头（绿灯补绣）
            Embroider.NoEmbFlag = 1;
            Pearl_Push_Handle();
            break;
            //--------------------------------------------------
        case 0x7://补绣头（红灯补绣）
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

	//测试补绣线是否连接
	if(Embroider.LightState != Embroider.LightStateOld)
	{
	    switch(Embroider.LightState)
	    {
	    case 1:
	        KeyHandle.DisplayData = 0x0000;//关机头无显示
	        KeyHandle.DisplayEnable = 1;
	        break;
	    case 2:
	        KeyHandle.DisplayData = 0x0040;//绿灯显示‘-’
	        KeyHandle.DisplayEnable = 1;
	        break;
	    case 3:
	        KeyHandle.DisplayData = 0x0076;//红灯显示‘H’
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
	            //记录换色电机位置
	            HS.InitPosData = EQep1Regs.QPOSCNT;
	            //TZ.cmd = 1;
	            TZ.cmd = 4;//向前走一个大步数，防止不同位置上电引起的异常 2019.07.22
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
    //绣作CAN命令->换色部分
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
//方案一：回退延时等待60ms后拨叉前进
//方案二：回退到位后等待5ms后拨叉前进
#define BackPlan 1

void Pearl_Push_Handle(void)
{
    //------------推珠业务处理函数---------------
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
                if(KeyHandle.OkTrgCnt == 2)//手动
                {
                    TZ.OriginStopFlag = 0;
                }
                else//自动
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
                    if(KeyHandle.OkTrgCnt == 2)//手动
                    {
                        motorE.step_count=0;
                        TZ.OriginStopFlag = 0;
                    }
                    else//自动
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
            //信胜汕头改机反馈的拨叉慢慢往后退情况，增加了回退步数超过设定值两倍报错功能，2019.11.05
            if(motorE.TempFlag == 1)
            {
                M3_DIS=1;//开环拨叉松轴
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
        if((TZ.state==2)&&((BCorigin == 1)||(TZ.ErrFlag > 2)))//推珠时发现还在原点或者上一次没有回到原点
        {
            M3_DIS=1;//开环拨叉松轴
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
    //------------全头换珠业务---------------
    if((TZ.All_START == 0)&&(TZ.All_START_State == 0))
    {
        //等待全头换珠
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
Uint16 ChangeColorDelay = 0;//拨叉开始推珠后延时20ms再换珠，防止卡顿
void Pearl_Change_Time_Check(void)
{
    /*TZ.QEPTargetValue = TZ.QEPInitValue - EQep2Regs.QPOSCNT;
    if(TZ.QEPTargetValue < -2000)
        TZ.QEPTargetValue = TZ.QEPTargetValue + 4000;*/

    if(TZ.Position_state == 3)
    {
        if(Embroider.StartStop == 1)//动态换珠（要参考推珠拨叉所在位置）
        {
            ChangeColorDelay++;
            if(ChangeColorDelay >= 100)//拨叉开始推珠后延时20ms再换珠，防止卡顿
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
        else//静态换珠
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
            (*DSP_Flash_Update)(); /*跳转到DSP Bootloader*/
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
                    CAN_TX_DLC2(0xF710);//直接降气阀比较危险，改为通知主控
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
    //--------------换色回到原点后会在数码管上显示“.”-------------------2019.06.26
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
    //--------------换色回到原点后会在数码管上显示“.”-------------------2019.06.26

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
    if(Embroider.StartStopFlag == 1)//拉杆启动触发标志位
    {
        Embroider.StartStopFlag = 0;
        if(MC.State == ST_Run)//拉杆启动时故障已解除，则数码管恢复到默认状态
        {
            KeyHandle.DisplayData = 0x0040;//"-"恢复显示默认状态
            KeyHandle.DisplayEnable = 1;
        }
    }
}
//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
