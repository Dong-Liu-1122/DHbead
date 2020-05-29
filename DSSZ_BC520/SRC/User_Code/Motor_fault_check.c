//=========================================================================
//  Project Number: step_controller
//  File Name : Motor_fault_check.c
//  Description: ���ϼ��
//  Version    Date     Author     Description
//  1.0     2017.04.06  wangdayu        use
//=========================================================================

//step1:ͷ�ļ�����
#include "F2806x_Device.h"     //DSP28030���趨��ͷ�ļ�
#include "Stepper-Settings.h"    // ��������ͷ�ļ�
#include "GzTest.h"

extern void CAN_TX_DLC8(void);
//step2:��������
//***********������ϼ�⺯��******************
#pragma CODE_SECTION(Motor_Fault_Check, "ramfuncs");
void Motor_Fault_Check(void) // 0.1ms*10=1ms
{
//---------------�����붨��-------------------------
    // ѹ�����������ϣ�
    //F01--Ӳ������;(Drv8412-FLT)
    //F02--�������;
    //F03--ת�ع���;
    //F04--λ�ó���;
    //F05--����;(Drv8412-OT)
    //F06--������A/B����;
    //F07--24Vĸ�߹�ѹ����
    //F08--A��ADC������׼����;
    //F09--B��ADC������׼����;
    //F10--A,B�඼����ADC������׼����;
    //F11--����λ��ʱ
    //F12--���߷�������ʼ������
	//F13--ĸ��Ƿѹ����
    //ĸ��24V--AD����������947--24V=
    adc1.Udc = (((int32)AdcResult.ADCRESULT6)*2928)>>15;   /*ĸ�ߵ�ѹ����ֵ*/

    static  Uint16 j_cnt=0;
    if(MC.Err_Value!=0)
   	{

   		if(j_cnt==0)
   		{
   			//CAN_TX_DLC8();
   			j_cnt=1;
   		}

   	}
    //DC��Դ�ϵ���
    static  Uint16 Power_Up_Flag=0;
    static  Uint32 Power_Up_Delay = 0;  //��Դ�ϵ�����ʱ
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

    //ĸ�ߵ�ѹ����ʱ���Ƽ�Drv8412���ڸ�λ״̬
    static  Uint16 Power_Down_Flag=0;
    static  Uint32 Power_Down_Delay = 0;  //��Դ��������ʱ
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
//**************step1.1:Ӳ����������F01����ѯ Drv8412�Ƿ���Ӳ������������*****************//
   //SY200-������������68k������Ӧ����4.1A
   static Uint16 Drv8412_FLT_Delay=0;
   #define  Drv8412_FLT_Delay_Set   100  //������������ʱ100*0.1ms=10ms
   //if((Drv8412_FLT==0)&&(Power_Down_Flag==0))
   if((INPUT_HC597Regs.bit.FAULT1_8412==0)&&(Power_Down_Flag==0))
   {
       Drv8412_FLT_Delay++;
       if(Drv8412_FLT_Delay>=Drv8412_FLT_Delay_Set)
       {
           PWM1_OFF_MACRO()
           MC.S_ERR=1;//�ù��ϱ�־
           MC.Err_Value = 1;//������--f01
           MC.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_FLT_Delay=0;
   }
//-----------------------------ѹ��--------------------------------
   //if((Drv8412_FLT_B==0)&&(Power_Down_Flag==0))
   if((INPUT_HC597Regs.bit.FAULT2_8412==0)&&(Power_Down_Flag==0))
   {
	  Drv8412_FLT_Delay++;
	  if(Drv8412_FLT_Delay>=Drv8412_FLT_Delay_Set)
	  {
		  PWM2_OFF_MACRO()
		  MC.S_ERR=2;//�ù��ϱ�־
		  MB.Err_Value = 1;//������--f01
		  MB.Err_Wri_Ena=1;
	  }
   }
   else
   {
	   Drv8412_FLT_Delay=0;
   }

//**************step1.2:����������������(��PWM�ж���ִ��)F02************//

//**************step1.3:ת�ع��ؼ��(ֻ�ڱջ�ʱʹ��)F03******************//
   #define  Over_Load_Delay_Set_Work  50000L   //���ر���ʱ��=40000*0.1ms=4s
   #define  Over_Load_Delay_Set_Stop  50000L   //���ر���ʱ��=20000*0.1ms=2s

   #if(Run_Mode==Close_Loop)

    static Uint32 Over_Load_Delay = 0;//���ر��������ʱ

    if(MC.Work_Stop_Flag==1)
    {
        //����״̬
        if( _IQabs(pi_iq.Fdb) > MC.Iq_overload_Limit) //q���������ֵ(80%*I_limit)
        {
            Over_Load_Delay++;
            if(Over_Load_Delay >= Over_Load_Delay_Set_Work) //���ر���ʱ��=20000*0.1ms=2s
            {
                PWM1_OFF_MACRO()
                MC.S_ERR=1;//�ù��ϱ�����־
                MC.Err_Value = 3;// ������-f03
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
       //ͣ��״̬
       if( _IQabs(pi_iq.Fdb) > MC.Iq_overload_Limit) //q���������ֵ(90%*I_limit)
       {
           Over_Load_Delay++;
           if(Over_Load_Delay >= Over_Load_Delay_Set_Stop) //���ر���ʱ��=10000*0.1ms=1s
           {
               PWM1_OFF_MACRO()
               MC.S_ERR=1;//�ù��ϱ�����־
               MC.Err_Value = 3;// ������-f03
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
    #if(Run_Mode_MotorB==Close_Loop) //ֻ�бջ�ģʽ���Ž���λ�ó�����
    static Uint32 Over_Load_Delay_B = 0;//���ر��������ʱ

    if(MB.Work_Stop_Flag==1)
       {
           //����״̬
           if( _IQabs(pi_iq2.Fdb) > MB.Iq_overload_Limit) //q���������ֵ(80%*I_limit)
           {
               Over_Load_Delay_B++;
               if(Over_Load_Delay_B >= Over_Load_Delay_Set_Work) //���ر���ʱ��=20000*0.1ms=2s
               {
                   PWM2_OFF_MACRO()
                   MC.S_ERR=2;//�ù��ϱ�����־
                   MB.Err_Value = 3;// ������-f03
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
          //ͣ��״̬
          if( _IQabs(pi_iq2.Fdb) > MB.Iq_overload_Limit) //q���������ֵ(90%*I_limit)
          {
        	  Over_Load_Delay_B++;
              if(Over_Load_Delay_B >= Over_Load_Delay_Set_Stop) //���ر���ʱ��=10000*0.1ms=1s
              {
                  PWM2_OFF_MACRO()
                  MC.S_ERR=2;//�ù��ϱ�����־
                  MB.Err_Value = 3;// ������-f03
                  MB.Err_Wri_Ena=1;
              }
          }
          else
          {
        	  Over_Load_Delay_B = 0;
          }
       }
    #endif
    if(GZ.Enable != 1)//�ǹ�װ����ģʽ
    {
        //**************step1.4:λ�ó�����(ֻ�ڱջ�ʱʹ��)F04***********************//
        //#define  Pos_Over_Limit_Work       EncoderLine*4    //����ʱλ�ù����޷�ֵ
        //#define  Pos_Over_Limit_Work       300    //����ʱλ�ù����޷�ֵ
        //#define  Pos_Over_Delay_Set_Work   50000L   //����ʱλ�ù�����ʱ=40000*0.1ms=4s
#define  Pos_Over_Limit_Work       20    //����ʱλ�ù����޷�ֵ
#define  Pos_Over_Delay_Set_Work   1500L   //����ʱλ�ù�����ʱ=40000*0.1ms=4s

        //#define  Pos_Over_Limit_Stop       EncoderLine*2    //ͣ��ʱλ�ù����޷�ֵ
        //#define  Pos_Over_Limit_Stop       100      //ͣ��ʱλ�ù����޷�ֵ
        //#define  Pos_Over_Delay_Set_Stop   50000L   //ͣ��ʱλ�ù�����ʱ=20000*0.1ms=2s
#define  Pos_Over_Limit_Stop       20      //ͣ��ʱλ�ù����޷�ֵ
#define  Pos_Over_Delay_Set_Stop   1500L   //ͣ��ʱλ�ù�����ʱ=20000*0.1ms=2s

#if(Run_Mode==Close_Loop) //ֻ�бջ�ģʽ���Ž���λ�ó�����

        static Uint32 Pos_Over_Delay = 0; //λ�ù��������ʱ


        if(MC.Work_Stop_Flag==1)
        {
            if( abs(MC.P_Out_Err) > Pos_Over_Limit_Work )  //����״̬<=2000
            {
                Pos_Over_Delay++;
                if(Pos_Over_Delay >= Pos_Over_Delay_Set_Work) //��ʱ=10000*0.1ms=1s
                {
                    PWM1_OFF_MACRO()
                                      MC.S_ERR=1; //�ù��ϱ�־
                    MC.Err_Value = 4;//������-f04
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
            if( abs(MC.P_Out_Err) > Pos_Over_Limit_Stop )  //ͣ��״̬<=4000/8=500
            {
                Pos_Over_Delay++;
                if(Pos_Over_Delay >= Pos_Over_Delay_Set_Stop) //��ʱ=10000*0.1ms/4=0.25s
                {
                    PWM1_OFF_MACRO()
                                       MC.S_ERR=1; //�ù��ϱ�־
                    MC.Err_Value = 4;//������-f04
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
#if(Run_Mode_MotorB==Close_Loop) //ֻ�бջ�ģʽ���Ž���λ�ó�����

        static Uint32 Pos_Over_Delay_B = 0; //λ�ù��������ʱ
        if(MB.Work_Stop_Flag==1)
        {
            if( abs(MB.P_Out_Err) > Pos_Over_Limit_Work )  //����״̬<=2000
            {
                Pos_Over_Delay_B++;
                if(Pos_Over_Delay_B >= Pos_Over_Delay_Set_Work) //��ʱ=10000*0.1ms=1s
                {
                    PWM2_OFF_MACRO()
                         MC.S_ERR=2; //�ù��ϱ�־
                    MB.Err_Value = 4;//������-f04
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
            if( abs(MB.P_Out_Err) > Pos_Over_Limit_Stop )  //ͣ��״̬<=4000/8=500
            {
                Pos_Over_Delay_B++;
                if(Pos_Over_Delay_B >= Pos_Over_Delay_Set_Stop) //��ʱ=10000*0.1ms/4=0.25s
                {
                    PWM2_OFF_MACRO()
                          MC.S_ERR=2; //�ù��ϱ�־
                    MB.Err_Value = 4;//������-f04
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

//**************step1.5:Drv8412���¼��(F05)************************//
   static Uint32 Drv8412_OTW_Delay=0;
   static Uint32 Drv8412_OTW_Delay_B=0;
   #define  Drv8412_OTW_Delay_Set   20000L  //������������ʱ2000*0.1ms=2s

   //if((Drv8412_OTW==0)&&(Drv8412_FLT==1)) //Drv8412���³���125��
   if((INPUT_HC597Regs.bit.OTW1_8412==0)&&(INPUT_HC597Regs.bit.FAULT1_8412==1)) //Drv8412���³���125��
   {
       Drv8412_OTW_Delay++;
       if(Drv8412_OTW_Delay>=Drv8412_OTW_Delay_Set)
       {
           PWM1_OFF_MACRO()
           MC.S_ERR=1;//�ù��ϱ�־
           MC.Err_Value = 5;//������--f05
           MC.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_OTW_Delay=0;
   }
   //------------------------------------------------------------

//   if((Drv8412_OTW_B==0)&&(Drv8412_FLT_B==1)) //Drv8412���³���125��
   if((INPUT_HC597Regs.bit.OTW2_8412==0)&&(INPUT_HC597Regs.bit.FAULT2_8412==1))
   {
       Drv8412_OTW_Delay_B++;
       if(Drv8412_OTW_Delay_B>=Drv8412_OTW_Delay_Set)
       {
           PWM2_OFF_MACRO()
           MC.S_ERR=2;//�ù��ϱ�־
           MB.Err_Value = 5;//������--f05
           MB.Err_Wri_Ena=1;
       }
   }
   else
   {
       Drv8412_OTW_Delay_B=0;
   }

//**************step1.6:������A/B���ϼ��(F06)********************//
     #if(Run_Mode==Close_Loop)
     #define  Enc_AB_Delay_Set   5  //����������
     static Uint32 Enc_AB_Delay = 0;//������AB���ϼ����ʱ
     if(EQep1Regs.QFLG.bit.PHE ==1)//������������λ�����־Quadrature phase error
     {
         EQep1Regs.QCLR.bit.PHE = 1; //���жϱ�־
         Enc_AB_Delay++;
         if(Enc_AB_Delay>=Enc_AB_Delay_Set)
         {
             MC.S_ERR=1;
             MC.Err_Value = 6;//������-f06
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
    #define  Enc_AB_Delay_Set_B   5  //����������
    static Uint32 Enc_AB_Delay_B = 0;//������AB���ϼ����ʱ
    if(EQep2Regs.QFLG.bit.PHE ==1)//������������λ�����־Quadrature phase error
    {
        EQep2Regs.QCLR.bit.PHE = 1; //���жϱ�־
        Enc_AB_Delay_B++;
        if(Enc_AB_Delay_B>=Enc_AB_Delay_Set)
        {
            MC.S_ERR=2;
            MB.Err_Value = 6;//������-f06
            MB.Err_Wri_Ena=1;
        }
    }
    else
    {
    	Enc_AB_Delay_B = 0;
    }
    #endif
//**************step1.7:ĸ�߹�ѹ����(F07)*******************//
    #define  Udc_Max_Limit   300   //ĸ�ߵ�ѹ�޷�ֵ(��λ��0.1V)
    #define  OV_Delay_Set    1000  //��ѹ����ʱ��5000*0.1ms=100ms
    static   Uint32 OV_Delay = 0;  //��ѹ�����ʱ
     if(adc1.Udc>=Udc_Max_Limit)   //40V --Drv8412��߹�����ѹ50V
     {
         OV_Delay++;
         if(OV_Delay>OV_Delay_Set)
         {
             PWM1_OFF_MACRO()
             MC.S_ERR=1;//�ù��ϱ�־
             MC.Err_Value = 7;//������--f07ĸ�߹�ѹ����
             MC.Err_Wri_Ena=1;
         }
     }
     else
     {
         OV_Delay=0;
     }
//***********step1.8:ADC������׼���ϼ��(�ϵ����������ʱִ��)F08,F09,F10*******//


//***********step1.9:ĸ��Ƿѹ����(F13)*******************//
    #define  Udc_Min_Limit   180  //ĸ�ߵ�ѹ�޷�ֵ(��λ��0.1V)
    #define  LV_Delay_Set    20000L  //��ѹ����ʱ��5000*0.1ms=2000ms
    static   Uint32 LV_Delay = 0;  //Ƿѹ�����ʱ

     if((Power_Up_Flag==1)&&(adc1.Udc<=Udc_Min_Limit))//18V
     {
         LV_Delay++;
         if(LV_Delay>LV_Delay_Set)
         {
             LV_Delay=LV_Delay_Set;
             PWM1_OFF_MACRO()
             MC.S_ERR=1;//�ù��ϱ�־
             MC.Err_Value = 13;//������--f13ĸ��Ƿѹ����
             MC.Err_Wri_Ena=1;
         }
     }
     else
     {
         LV_Delay=0;
     }


  }//end of if(MC.S_ERR == 0)

//*****************step1.12:���ϱ���************************//
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
