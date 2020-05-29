//========================================================================
//  Project Number: step_controller
//  File Name :  SPTA.c
//  Description: SPTA算法实现指令脉冲输出
//  Version    Date     Author     Description
//  1.0    2015.10.13   wangdayu        use
//=========================================================================

//1——头文件
#include "F2806x_Device.h"    //DSP28030外设定义头文件
#include "Stepper-Settings.h"

//2——变量声明
MOTOR_CONTROL_SPTA motorC = MOTOR_CONTROL_SPTA_DEFAULTE;  //拌珠C电机
MOTOR_CONTROL_SPTA motorD = MOTOR_CONTROL_SPTA_DEFAULTE;  //拌珠D电机
MOTOR_CONTROL_SPTA motorE = MOTOR_CONTROL_SPTA_DEFAULTE;  //拌珠E电机

//3——函数声明
/*电机SPTA算法处理函数*/
#pragma CODE_SECTION(IRQHandler_SPTA,"ramfuncs");
void IRQHandler_SPTA(MOTOR_CONTROL_SPTA *pmotor)
{
    int32 carry=0;

    /*根据步数累加器是否溢出，决定是否产生一个步进脉冲
         步数累加器由步数速度器累加溢出产生*/
    pmotor->step_frac += pmotor->step_speed;
  //判断是否溢出
    carry = pmotor->step_frac >> 16;
    pmotor->step_frac -= carry << 16;
    if(carry!=0)
    {
    	//溢出则产生一个步进脉冲
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
        if(pmotor->step_count > (pmotor->step_move + 600))//拨叉速度模式回原点，当执行两倍距离后自动关闭
        {
            pmotor->step_state = 4;
            pmotor->speedenbale = 0;
            pmotor->TempFlag = 1;
        }
    }

    //速度控制，则在速度满足时返回，3是速度的误差，SPTA的速度等级是浮点运算得出的，会存在误差
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

    /*根据电机的状态进行状态转换以及参数变换*/
    switch(pmotor->step_state)
    {
       /*******************************************************/
         case ACCELERATING:
            //计算加速阶段的步数
            if(carry)
            {
                pmotor->step_acced++;
            }
            //加速阶段，速度累加器要根据设定的加速度值进行累加
            //pmotor->speed_frac+=pmotor->step_accel;
            pmotor->speed_frac+=pmotor->step_accel_up;
            //溢出判断
            carry=pmotor->speed_frac>>17;
            pmotor->speed_frac-=(carry<<17);
            if(carry)
            {
                //如果速度累加器溢出，则步数速度器需要加上该值，以便推动
                //步数速度器的增长，为步数累加器溢出准备
                pmotor->step_speed+=carry;
            }
            if(!pmotor->speedenbale)
            {
                //在非速度控制下，到达了中值就要反转为减速
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
                    //只运行一步？
                    pmotor->step_state=DECELERATING;
                }
            }
            if(pmotor->step_speed>=pmotor->step_spmax)
            {
                //加速阶段到达一定的时刻，就达到了设定的最大速度
                pmotor->step_speed=pmotor->step_spmax;
               //转为最大速度状态
                pmotor->step_state=AT_MAX;
            }
            break;
       /*******************************************************/
        case AT_MAX:
      //在最大速度运行时，如果剩余需要运行的步数小余等于加速步则需要减速了，减速与加速
      //步数相同
            if(pmotor->step_move-pmotor->step_count<pmotor->step_acced)
            {
            	pmotor->step_state=DECELERATING;
            }
            break;
     /*******************************************************/
        case DECELERATING:
            //减速阶段与加速阶段是相反的过程，原理也相同，只是很多+号变成-号
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
            	//不是速度控制，在运行设定步数以后停止运行
            	if(pmotor->step_count>=pmotor->step_move)
            	{
            		//设定电机状态，关闭定时器
            		pmotor->step_state=IDLE;
            		pmotor->running=0;
            		pmotor->step_spmax=0;
            		//SPTA_Pulse=0;//清零上次脉冲数
            		pmotor->SPTA_PULSE=0;

                    pmotor->divnum=1;
                    pmotor->enable=0;
                    pmotor->dir=0;
                    pmotor->running=0;
                    pmotor->speedenbale=0;
                    pmotor->speedenbale=0;
                    pmotor->step_move=0;
                    pmotor->step_middle=0; //运行脉冲数的一半
                    pmotor->step_spmax=0;
                    pmotor->step_accel_up=0;
                    pmotor->step_accel_down=0;

                    pmotor->step_state=0;  //初始为idle
                    pmotor->step_frac=0;
                    pmotor->speed_frac=0;
                    pmotor->step_acced=0;
                    pmotor->step_speed=0;
                    pmotor->step_count=0;
                    pmotor->reseting=0;/*复位标志清零*/

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
        	pmotor->step_middle=0; //运行脉冲数的一半
        	pmotor->step_spmax=0;
        	pmotor->step_accel_up=0;
        	pmotor->step_accel_down=0;

        	pmotor->step_state=0;  //初始为idle
        	pmotor->step_frac=0;
        	pmotor->speed_frac=0;
        	pmotor->step_acced=0;
        	pmotor->step_speed=0;
        	//pmotor->step_count=0;
        	pmotor->reseting=0;/*复位标志清零*/

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
  Spta参数初始化
    入口参数:  MOTOR_CONTROL_SPTA *pmotor
    返回参数： 无
**************************************************************************************/
void Init_Motor_SPTA(MOTOR_CONTROL_SPTA *pmotor)
{
        /*电机运行参数初始化*/
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
        pmotor->step_middle=0; //运行脉冲数的一半
        /*FIXME:这两个参数可以由用户自行改变测试*/
        pmotor->step_spmax=0;
        pmotor->step_accel_up=0;
        pmotor->step_accel_down=0;

        pmotor->step_state=0;  //初始为idle
        pmotor->step_frac=0;
        pmotor->speed_frac=0;
        pmotor->step_acced=0;
        pmotor->step_speed=0;
        pmotor->step_count=0;
        pmotor->reseting=0;/*复位标志清零*/

        pmotor->SPTA_PULSE=0;
        pmotor->SPTA_PULSE_ERR=0;
        pmotor->SPTA_PULSE_OLD=0;
        pmotor->PulseEnable=0;

        pmotor->DisableFlag=0;
}
/**************************************************************************************
    启动电机按照Spta参数运行
    入口参数:  dir，运行方向； Degree，要运行的步数
    返回参数： 无
 **************************************************************************************/
//#pragma CODE_SECTION(Start_Motor_SPTA,"ramfuncs");
void Start_Motor_SPTA(Uint16 dir,Uint16 Degree,MOTOR_CONTROL_SPTA *pmotor)
{
        Uint32 Pulses_Set=0;

        if(Degree==0)
        {
           return;
        }
        /*电机运行参数初始化*/
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
        pmotor->step_middle=pmotor->step_move>>1; //运行脉冲数的一半

/*      pmotor->step_spmax=52650;
        pmotor->step_accel_up=120000;
        pmotor->step_accel_down=120000;*/

        pmotor->step_state=ACCELERATING;
        pmotor->step_frac=0;
        pmotor->speed_frac=0;
        pmotor->step_acced=0;
        pmotor->step_speed=0;
        pmotor->step_count=0;
        pmotor->reseting=0;/*复位标志清零*/

        pmotor->SPTA_PULSE_ERR = 0;
        pmotor->SPTA_PULSE_OLD = 0;
        pmotor->SPTA_PULSE = 0;
}

//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
