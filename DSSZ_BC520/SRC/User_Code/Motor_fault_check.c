//=========================================================================
//  Project Number: step_controller
//  File Name : Motor_fault_check.c
//  Description: 故障检测
//  Version    Date     Author     Description
//  1.0     2017.04.06  wangdayu        use
//=========================================================================

//step1:头文件声明
#include "F2806x_Device.h"     //DSP28030外设定义头文件
#include "Stepper-Settings.h"    // 参数设置头文件
#include "GzTest.h"

extern void CAN_TX_DLC8(void);
//step2:函数声明
//***********电机故障检测函数******************
#pragma CODE_SECTION(Motor_Fault_Check, "ramfuncs");
void Motor_Fault_Check(void) // 0.1ms*10=1ms
{
//---------------故障码定义-------------------------
    // 压脚驱动器故障：
    //F01--硬件过流;(Drv8412-FLT)
    //F02--软件过流;
    //F03--转矩过载;
    //F04--位置超差;
    //F05--过温;(Drv8412-OT)
    //F06--编码器A/B故障;
    //F07--24V母线过压保护
    //F08--A相ADC采样基准故障;
    //F09--B相ADC采样基准故障;
    //F10--A,B相都发生ADC采样基准故障;
    //F11--找零位超时
    //F12--曲线发生器初始化错误
	//F13--母线欠压保护
    //母线24V--AD采样数字量947--24V=
    adc1.Udc = (((int32)AdcResult.ADCRESULT6)*2928)>>15;   /*母线电压采样值*/

    static  Uint16 j_cnt=0;
    if(MC.Err_Value!=0)
   	{

   		if(j_cnt==0)
   		{
   			//CAN_TX_DLC8();
   			j_cnt=1;
   		}

   	}
    //DC电源上电检测
    static  Uint16 Power_Up_Flag=0;
    static  Uint32 Power_Up_Delay = 0;  //电源上电检测延时
    if((Power_Up_Flag==0)&&(adc1.Udc>=200))//20V
    {
        Power_Up_Delay++;
        if(Power_Up_Delay>=20000)
        {
            Power_Up_Delay=20000;
            Power_Up_Flag=1;
        }
    }
    else
    {
        Power_Up_Delay=0;
    }

    //母线电压掉电时，推荐Drv8412处在复位状态
    static  Uint16 Power_Down_Flag=0;
    static  Uint32 Power_Down_Delay = 0;  //电源掉电检测延时
    if((Power_Up_Flag==1)&&(adc1.Udc<=150))//15V
    {
        Power_Down_Delay++;
        if(Power_Down_Delay>50)
        {
            Power_Down_Delay=50;
            Power_Down_Flag=1;
            PWM1_OFF_MACRO()
            PWM2_OFF_MACRO()
        }
    }
    else
    {
        Power_Down_Delay=0;
    }


 if(MC.S_ERR == 0)
{
//**************step1.1:硬件过流保护F01（查询 Drv8412是否发生硬件过流保护）*****************//
   //SY200-过流保护电阻68kΩ，对应电流4.1A
   static Uint16 Drv8412_FLT_Delay=0;
   #define  Drv8412_FLT_Delay_Set   100  //编码器故障延时100*0.1ms=10ms
   //if((Drv8412_FLT==0)&&(Power_Down_Flag==0))
   if((INPUT_HC597Regs.bit.FAULT1_8412==0)&&(Power_Down_Flag==0))
   {
       Drv8412_FLT_Delay++;
       if(Drv8412_FLT_Delay>=Drv8412_FLT_Delay_Set)
       {
           PWM1_OFF_MACRO()
           MC.S_ERR=1;//置故障标志
           MC.Err_Value = 1;//故障码--f01
           MC.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_FLT_Delay=0;
   }
//-----------------------------压脚--------------------------------
   //if((Drv8412_FLT_B==0)&&(Power_Down_Flag==0))
   if((INPUT_HC597Regs.bit.FAULT2_8412==0)&&(Power_Down_Flag==0))
   {
	  Drv8412_FLT_Delay++;
	  if(Drv8412_FLT_Delay>=Drv8412_FLT_Delay_Set)
	  {
		  PWM2_OFF_MACRO()
		  MC.S_ERR=2;//置故障标志
		  MB.Err_Value = 1;//故障码--f01
		  MB.Err_Wri_Ena=1;
	  }
   }
   else
   {
	   Drv8412_FLT_Delay=0;
   }

//**************step1.2:软件相电流过流保护(在PWM中断中执行)F02************//

//**************step1.3:转矩过载检测(只在闭环时使能)F03******************//
   #define  Over_Load_Delay_Set_Work  50000L   //过载保护时间=40000*0.1ms=4s
   #define  Over_Load_Delay_Set_Stop  50000L   //过载保护时间=20000*0.1ms=2s

   #if(Run_Mode==Close_Loop)

    static Uint32 Over_Load_Delay = 0;//过载保护检测延时

    if(MC.Work_Stop_Flag==1)
    {
        //工作状态
        if( _IQabs(pi_iq.Fdb) > MC.Iq_overload_Limit) //q轴电流反馈值(80%*I_limit)
        {
            Over_Load_Delay++;
            if(Over_Load_Delay >= Over_Load_Delay_Set_Work) //过载保护时间=20000*0.1ms=2s
            {
                PWM1_OFF_MACRO()
                MC.S_ERR=1;//置故障保护标志
                MC.Err_Value = 3;// 故障码-f03
                MC.Err_Wri_Ena=1;
            }
        }
        else
        {
            Over_Load_Delay = 0;
        }
    }
    else
    {
       //停车状态
       if( _IQabs(pi_iq.Fdb) > MC.Iq_overload_Limit) //q轴电流反馈值(90%*I_limit)
       {
           Over_Load_Delay++;
           if(Over_Load_Delay >= Over_Load_Delay_Set_Stop) //过载保护时间=10000*0.1ms=1s
           {
               PWM1_OFF_MACRO()
               MC.S_ERR=1;//置故障保护标志
               MC.Err_Value = 3;// 故障码-f03
               MC.Err_Wri_Ena=1;
           }
       }
       else
       {
           Over_Load_Delay = 0;
       }
    }
    #endif

    //--------------------------------------------------------------------
    #if(Run_Mode_MotorB==Close_Loop) //只有闭环模式，才进行位置超差检测
    static Uint32 Over_Load_Delay_B = 0;//过载保护检测延时

    if(MB.Work_Stop_Flag==1)
       {
           //工作状态
           if( _IQabs(pi_iq2.Fdb) > MB.Iq_overload_Limit) //q轴电流反馈值(80%*I_limit)
           {
               Over_Load_Delay_B++;
               if(Over_Load_Delay_B >= Over_Load_Delay_Set_Work) //过载保护时间=20000*0.1ms=2s
               {
                   PWM2_OFF_MACRO()
                   MC.S_ERR=2;//置故障保护标志
                   MB.Err_Value = 3;// 故障码-f03
                   MB.Err_Wri_Ena=1;
               }
           }
           else
           {
        	   Over_Load_Delay_B = 0;
           }
       }
       else
       {
          //停车状态
          if( _IQabs(pi_iq2.Fdb) > MB.Iq_overload_Limit) //q轴电流反馈值(90%*I_limit)
          {
        	  Over_Load_Delay_B++;
              if(Over_Load_Delay_B >= Over_Load_Delay_Set_Stop) //过载保护时间=10000*0.1ms=1s
              {
                  PWM2_OFF_MACRO()
                  MC.S_ERR=2;//置故障保护标志
                  MB.Err_Value = 3;// 故障码-f03
                  MB.Err_Wri_Ena=1;
              }
          }
          else
          {
        	  Over_Load_Delay_B = 0;
          }
       }
    #endif
    if(GZ.Enable != 1)//非工装调试模式
    {
        //**************step1.4:位置超差检测(只在闭环时使能)F04***********************//
        //#define  Pos_Over_Limit_Work       EncoderLine*4    //工作时位置过超限幅值
        //#define  Pos_Over_Limit_Work       300    //工作时位置过超限幅值
        //#define  Pos_Over_Delay_Set_Work   50000L   //工作时位置过超延时=40000*0.1ms=4s
#define  Pos_Over_Limit_Work       20    //工作时位置过超限幅值
#define  Pos_Over_Delay_Set_Work   1500L   //工作时位置过超延时=40000*0.1ms=4s

        //#define  Pos_Over_Limit_Stop       EncoderLine*2    //停机时位置过超限幅值
        //#define  Pos_Over_Limit_Stop       100      //停机时位置过超限幅值
        //#define  Pos_Over_Delay_Set_Stop   50000L   //停机时位置过超延时=20000*0.1ms=2s
#define  Pos_Over_Limit_Stop       20      //停机时位置过超限幅值
#define  Pos_Over_Delay_Set_Stop   1500L   //停机时位置过超延时=20000*0.1ms=2s

#if(Run_Mode==Close_Loop) //只有闭环模式，才进行位置超差检测

        static Uint32 Pos_Over_Delay = 0; //位置过超检测延时


        if(MC.Work_Stop_Flag==1)
        {
            if( abs(MC.P_Out_Err) > Pos_Over_Limit_Work )  //工作状态<=2000
            {
                Pos_Over_Delay++;
                if(Pos_Over_Delay >= Pos_Over_Delay_Set_Work) //延时=10000*0.1ms=1s
                {
                    PWM1_OFF_MACRO()
                                      MC.S_ERR=1; //置故障标志
                    MC.Err_Value = 4;//故障码-f04
                    MC.Err_Wri_Ena=1;
                }
            }
            else
            {
                Pos_Over_Delay=0;
            }
        }
        else
        {
            if( abs(MC.P_Out_Err) > Pos_Over_Limit_Stop )  //停机状态<=4000/8=500
            {
                Pos_Over_Delay++;
                if(Pos_Over_Delay >= Pos_Over_Delay_Set_Stop) //延时=10000*0.1ms/4=0.25s
                {
                    PWM1_OFF_MACRO()
                                       MC.S_ERR=1; //置故障标志
                    MC.Err_Value = 4;//故障码-f04
                    MC.Err_Wri_Ena=1;
                }
            }
            else
            {
                Pos_Over_Delay=0;
            }
        }


#endif
        //-----------------------------------------------------------------------
#if(Run_Mode_MotorB==Close_Loop) //只有闭环模式，才进行位置超差检测

        static Uint32 Pos_Over_Delay_B = 0; //位置过超检测延时
        if(MB.Work_Stop_Flag==1)
        {
            if( abs(MB.P_Out_Err) > Pos_Over_Limit_Work )  //工作状态<=2000
            {
                Pos_Over_Delay_B++;
                if(Pos_Over_Delay_B >= Pos_Over_Delay_Set_Work) //延时=10000*0.1ms=1s
                {
                    PWM2_OFF_MACRO()
                         MC.S_ERR=2; //置故障标志
                    MB.Err_Value = 4;//故障码-f04
                    MB.Err_Wri_Ena=1;
                }
            }
            else
            {
                Pos_Over_Delay_B=0;
            }
        }
        else
        {
            if( abs(MB.P_Out_Err) > Pos_Over_Limit_Stop )  //停机状态<=4000/8=500
            {
                Pos_Over_Delay_B++;
                if(Pos_Over_Delay_B >= Pos_Over_Delay_Set_Stop) //延时=10000*0.1ms/4=0.25s
                {
                    PWM2_OFF_MACRO()
                          MC.S_ERR=2; //置故障标志
                    MB.Err_Value = 4;//故障码-f04
                    MB.Err_Wri_Ena=1;
                }
            }
            else
            {
                Pos_Over_Delay_B=0;
            }
        }
#endif
    }

//**************step1.5:Drv8412过温检测(F05)************************//
   static Uint32 Drv8412_OTW_Delay=0;
   static Uint32 Drv8412_OTW_Delay_B=0;
   #define  Drv8412_OTW_Delay_Set   20000L  //编码器故障延时2000*0.1ms=2s

   //if((Drv8412_OTW==0)&&(Drv8412_FLT==1)) //Drv8412结温超过125℃
   if((INPUT_HC597Regs.bit.OTW1_8412==0)&&(INPUT_HC597Regs.bit.FAULT1_8412==1)) //Drv8412结温超过125℃
   {
       Drv8412_OTW_Delay++;
       if(Drv8412_OTW_Delay>=Drv8412_OTW_Delay_Set)
       {
           PWM1_OFF_MACRO()
           MC.S_ERR=1;//置故障标志
           MC.Err_Value = 5;//故障码--f05
           MC.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_OTW_Delay=0;
   }
   //------------------------------------------------------------

//   if((Drv8412_OTW_B==0)&&(Drv8412_FLT_B==1)) //Drv8412结温超过125℃
   if((INPUT_HC597Regs.bit.OTW2_8412==0)&&(INPUT_HC597Regs.bit.FAULT2_8412==1))
   {
       Drv8412_OTW_Delay_B++;
       if(Drv8412_OTW_Delay_B>=Drv8412_OTW_Delay_Set)
       {
           PWM2_OFF_MACRO()
           MC.S_ERR=2;//置故障标志
           MB.Err_Value = 5;//故障码--f05
           MB.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_OTW_Delay_B=0;
   }

//**************step1.6:编码器A/B故障检测(F06)********************//
     #if(Run_Mode==Close_Loop)
     #define  Enc_AB_Delay_Set   5  //编码器故障
     static Uint32 Enc_AB_Delay = 0;//编码器AB故障检测延时
     if(EQep1Regs.QFLG.bit.PHE ==1)//正交编码器相位错误标志Quadrature phase error
     {
         EQep1Regs.QCLR.bit.PHE = 1; //清中断标志
         Enc_AB_Delay++;
         if(Enc_AB_Delay>=Enc_AB_Delay_Set)
         {
             MC.S_ERR=1;
             MC.Err_Value = 6;//故障码-f06
             MC.Err_Wri_Ena=1;
         }
     }
     else
     {
         Enc_AB_Delay = 0;
     }
     #endif
//----------------------------------------------------------
    #if(Run_Mode_MotorB==Close_Loop)
    #define  Enc_AB_Delay_Set_B   5  //编码器故障
    static Uint32 Enc_AB_Delay_B = 0;//编码器AB故障检测延时
    if(EQep2Regs.QFLG.bit.PHE ==1)//正交编码器相位错误标志Quadrature phase error
    {
        EQep2Regs.QCLR.bit.PHE = 1; //清中断标志
        Enc_AB_Delay_B++;
        if(Enc_AB_Delay_B>=Enc_AB_Delay_Set)
        {
            MC.S_ERR=2;
            MB.Err_Value = 6;//故障码-f06
            MB.Err_Wri_Ena=1;
        }
    }
    else
    {
    	Enc_AB_Delay_B = 0;
    }
    #endif
//**************step1.7:母线过压保护(F07)*******************//
    #define  Udc_Max_Limit   300   //母线电压限幅值(单位：0.1V)
    #define  OV_Delay_Set    1000  //过压保护时间5000*0.1ms=100ms
    static   Uint32 OV_Delay = 0;  //过压检测延时
     if(adc1.Udc>=Udc_Max_Limit)   //40V --Drv8412最高工作电压50V
     {
         OV_Delay++;
         if(OV_Delay>OV_Delay_Set)
         {
             PWM1_OFF_MACRO()
             MC.S_ERR=1;//置故障标志
             MC.Err_Value = 7;//故障码--f07母线过压保护
             MC.Err_Wri_Ena=1;
         }
     }
     else
     {
         OV_Delay=0;
     }
//***********step1.8:ADC采样基准故障检测(上电参数参数化时执行)F08,F09,F10*******//


//***********step1.9:母线欠压保护(F13)*******************//
    #define  Udc_Min_Limit   180  //母线电压限幅值(单位：0.1V)
    #define  LV_Delay_Set    20000L  //过压保护时间5000*0.1ms=2000ms
    static   Uint32 LV_Delay = 0;  //欠压检测延时

     if((Power_Up_Flag==1)&&(adc1.Udc<=Udc_Min_Limit))//18V
     {
         LV_Delay++;
         if(LV_Delay>LV_Delay_Set)
         {
             LV_Delay=LV_Delay_Set;
             PWM1_OFF_MACRO()
             MC.S_ERR=1;//置故障标志
             MC.Err_Value = 13;//故障码--f13母线欠压保护
             MC.Err_Wri_Ena=1;
         }
     }
     else
     {
         LV_Delay=0;
     }


  }//end of if(MC.S_ERR == 0)

//*****************step1.12:故障保护************************//
   if(MC.S_ERR==1)
   {
      PWM1_OFF_MACRO()
   }
   if(MC.S_ERR==2)
   {
	  PWM2_OFF_MACRO()
   }


}//end of Motor_Fault_Check


//===========================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED 
//===========================================================================
