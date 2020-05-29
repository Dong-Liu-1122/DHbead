//========================================================================
//  Project Number: step_controller
//  File Name :  SPTA.c
//  Description: SPTA�㷨ʵ��ָ���������
//  Version    Date     Author     Description
//  1.0    2015.10.13   wangdayu        use
//=========================================================================

//1����ͷ�ļ�
#include "F2806x_Device.h"    //DSP28030���趨��ͷ�ļ�
#include "Stepper-Settings.h"

//2������������
MOTOR_CONTROL_SPTA motorC = MOTOR_CONTROL_SPTA_DEFAULTE;  //����C���
MOTOR_CONTROL_SPTA motorD = MOTOR_CONTROL_SPTA_DEFAULTE;  //����D���
MOTOR_CONTROL_SPTA motorE = MOTOR_CONTROL_SPTA_DEFAULTE;  //����E���

//3������������
/*���SPTA�㷨������*/
#pragma CODE_SECTION(IRQHandler_SPTA,"ramfuncs");
void IRQHandler_SPTA(MOTOR_CONTROL_SPTA *pmotor)
{
    int32 carry=0;

    /*���ݲ����ۼ����Ƿ�����������Ƿ����һ����������
         �����ۼ����ɲ����ٶ����ۼ��������*/
    pmotor->step_frac += pmotor->step_speed;
  //�ж��Ƿ����
    carry = pmotor->step_frac >> 16;
    pmotor->step_frac -= carry << 16;
    if(carry!=0)
    {
    	//��������һ����������
    	pmotor->step_count+=1;

    	if(pmotor->step_state!=IDLE)
    	{
    		if(pmotor->dir==0)
    		{
    			pmotor->SPTA_PULSE++;
    		}
    		else
    		{
    			pmotor->SPTA_PULSE--;
    		}
    	}
    }

    if(pmotor->TempTest == 1)
    {
        if(pmotor->step_count > (pmotor->step_move + 600))//�����ٶ�ģʽ��ԭ�㣬��ִ������������Զ��ر�
        {
            pmotor->step_state = 4;
            pmotor->speedenbale = 0;
            pmotor->TempFlag = 1;
        }
    }

    //�ٶȿ��ƣ������ٶ�����ʱ���أ�3���ٶȵ���SPTA���ٶȵȼ��Ǹ�������ó��ģ���������
    if(pmotor->speedenbale)
    {
        if( (pmotor->step_speed>=pmotor->step_spmax&&pmotor->step_speed-pmotor->step_spmax<=3)||
                (pmotor->step_speed<=pmotor->step_spmax&&pmotor->step_spmax-pmotor->step_speed<=3))
        {
            if(pmotor->step_move-pmotor->step_count<pmotor->step_acced)
            {
                pmotor->step_speed = 20000;
                pmotor->step_spmax = 20000;
            }
            return;
        }
    }

    /*���ݵ����״̬����״̬ת���Լ������任*/
    switch(pmotor->step_state)
    {
       /*******************************************************/
         case ACCELERATING:
            //������ٽ׶εĲ���
            if(carry)
            {
                pmotor->step_acced++;
            }
            //���ٽ׶Σ��ٶ��ۼ���Ҫ�����趨�ļ��ٶ�ֵ�����ۼ�
            //pmotor->speed_frac+=pmotor->step_accel;
            pmotor->speed_frac+=pmotor->step_accel_up;
            //����ж�
            carry=pmotor->speed_frac>>17;
            pmotor->speed_frac-=(carry<<17);
            if(carry)
            {
                //����ٶ��ۼ�������������ٶ�����Ҫ���ϸ�ֵ���Ա��ƶ�
                //�����ٶ�����������Ϊ�����ۼ������׼��
                pmotor->step_speed+=carry;
            }
            if(!pmotor->speedenbale)
            {
                //�ڷ��ٶȿ����£���������ֵ��Ҫ��תΪ����
                /*although we are ACCELERATING,but middle point reached,we need DECELERATING*/
                if(pmotor->step_middle!=0)
                {
                    if(pmotor->step_count>pmotor->step_middle)
                    {
                       pmotor->step_state=DECELERATING;
                    }
                }
                else if(pmotor->step_count>0)
                {
                    //ֻ����һ����
                    pmotor->step_state=DECELERATING;
                }
            }
            if(pmotor->step_speed>=pmotor->step_spmax)
            {
                //���ٽ׶ε���һ����ʱ�̣��ʹﵽ���趨������ٶ�
                pmotor->step_speed=pmotor->step_spmax;
               //תΪ����ٶ�״̬
                pmotor->step_state=AT_MAX;
            }
            break;
       /*******************************************************/
        case AT_MAX:
      //������ٶ�����ʱ�����ʣ����Ҫ���еĲ���С����ڼ��ٲ�����Ҫ�����ˣ����������
      //������ͬ
            if(pmotor->step_move-pmotor->step_count<pmotor->step_acced)
            {
            	pmotor->step_state=DECELERATING;
            }
            break;
     /*******************************************************/
        case DECELERATING:
            //���ٽ׶�����ٽ׶����෴�Ĺ��̣�ԭ��Ҳ��ͬ��ֻ�Ǻܶ�+�ű��-��
            if(carry&&pmotor->step_acced>0)
            {
                pmotor->step_acced--;
            }
            //pmotor->speed_frac+=pmotor->step_accel;
            pmotor->speed_frac+=pmotor->step_accel_down;
            carry=pmotor->speed_frac>>17;
            pmotor->speed_frac-=(carry<<17);
            if(carry && pmotor->step_speed>carry)
            {
                pmotor->step_speed-=carry;
            }

            if(!pmotor->speedenbale)
            {
            	//�����ٶȿ��ƣ��������趨�����Ժ�ֹͣ����
            	if(pmotor->step_count>=pmotor->step_move)
            	{
            		//�趨���״̬���رն�ʱ��
            		pmotor->step_state=IDLE;
            		pmotor->running=0;
            		pmotor->step_spmax=0;
            		//SPTA_Pulse=0;//�����ϴ�������
            		pmotor->SPTA_PULSE=0;

                    pmotor->divnum=1;
                    pmotor->enable=0;
                    pmotor->dir=0;
                    pmotor->running=0;
                    pmotor->speedenbale=0;
                    pmotor->speedenbale=0;
                    pmotor->step_move=0;
                    pmotor->step_middle=0; //������������һ��
                    pmotor->step_spmax=0;
                    pmotor->step_accel_up=0;
                    pmotor->step_accel_down=0;

                    pmotor->step_state=0;  //��ʼΪidle
                    pmotor->step_frac=0;
                    pmotor->speed_frac=0;
                    pmotor->step_acced=0;
                    pmotor->step_speed=0;
                    pmotor->step_count=0;
                    pmotor->reseting=0;/*��λ��־����*/

                    pmotor->SPTA_PULSE=0;
                    pmotor->SPTA_PULSE_ERR=0;
                    pmotor->SPTA_PULSE_OLD=0;
                    pmotor->PulseEnable=0;

                    pmotor->DisableFlag=0;
            	}
            }
            break;
        /*******************************************************/
        case STOP:
        	pmotor->divnum=1;
        	pmotor->enable=0;
        	pmotor->dir=0;
        	pmotor->running=0;
        	pmotor->speedenbale=0;
        	pmotor->speedenbale=0;
        	pmotor->step_move=0;
        	pmotor->step_middle=0; //������������һ��
        	pmotor->step_spmax=0;
        	pmotor->step_accel_up=0;
        	pmotor->step_accel_down=0;

        	pmotor->step_state=0;  //��ʼΪidle
        	pmotor->step_frac=0;
        	pmotor->speed_frac=0;
        	pmotor->step_acced=0;
        	pmotor->step_speed=0;
        	//pmotor->step_count=0;
        	pmotor->reseting=0;/*��λ��־����*/

        	pmotor->SPTA_PULSE=0;
        	pmotor->SPTA_PULSE_ERR=0;
        	pmotor->SPTA_PULSE_OLD=0;
        	pmotor->PulseEnable=0;

        	pmotor->DisableFlag=0;

        	pmotor->TempTest=0;
        	break;
        /*******************************************************/
        default:break;
    }
}
/**************************************************************************************
  Spta������ʼ��
    ��ڲ���:  MOTOR_CONTROL_SPTA *pmotor
    ���ز����� ��
**************************************************************************************/
void Init_Motor_SPTA(MOTOR_CONTROL_SPTA *pmotor)
{
        /*������в�����ʼ��*/
        pmotor->divnum=1;
        pmotor->enable=0;
        pmotor->dir=0;
        pmotor->running=0;
	#if ControlMode
        pmotor->speedenbale=0;
	#else
        pmotor->speedenbale=0;
	#endif
        pmotor->step_move=0;
        pmotor->step_middle=0; //������������һ��
        /*FIXME:�����������������û����иı����*/
        pmotor->step_spmax=0;
        pmotor->step_accel_up=0;
        pmotor->step_accel_down=0;

        pmotor->step_state=0;  //��ʼΪidle
        pmotor->step_frac=0;
        pmotor->speed_frac=0;
        pmotor->step_acced=0;
        pmotor->step_speed=0;
        pmotor->step_count=0;
        pmotor->reseting=0;/*��λ��־����*/

        pmotor->SPTA_PULSE=0;
        pmotor->SPTA_PULSE_ERR=0;
        pmotor->SPTA_PULSE_OLD=0;
        pmotor->PulseEnable=0;

        pmotor->DisableFlag=0;
}
/**************************************************************************************
    �����������Spta��������
    ��ڲ���:  dir�����з��� Degree��Ҫ���еĲ���
    ���ز����� ��
 **************************************************************************************/
//#pragma CODE_SECTION(Start_Motor_SPTA,"ramfuncs");
void Start_Motor_SPTA(Uint16 dir,Uint16 Degree,MOTOR_CONTROL_SPTA *pmotor)
{
        Uint32 Pulses_Set=0;

        if(Degree==0)
        {
           return;
        }
        /*������в�����ʼ��*/
        pmotor->divnum=1;
        pmotor->enable=1;
        pmotor->dir=dir;
        pmotor->running=1;
/*
		#if ControlMode
			pmotor->speedenbale=0;
		#else
			pmotor->speedenbale=1;
		#endif
*/
        Pulses_Set=Degree;
        pmotor->step_move=(Uint32)(Pulses_Set*pmotor->divnum);
        pmotor->step_middle=pmotor->step_move>>1; //������������һ��

/*      pmotor->step_spmax=52650;
        pmotor->step_accel_up=120000;
        pmotor->step_accel_down=120000;*/

        pmotor->step_state=ACCELERATING;
        pmotor->step_frac=0;
        pmotor->speed_frac=0;
        pmotor->step_acced=0;
        pmotor->step_speed=0;
        pmotor->step_count=0;
        pmotor->reseting=0;/*��λ��־����*/

        pmotor->SPTA_PULSE_ERR = 0;
        pmotor->SPTA_PULSE_OLD = 0;
        pmotor->SPTA_PULSE = 0;
}

//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
