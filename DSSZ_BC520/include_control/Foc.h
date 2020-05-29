 /* =================================================================================
//  File name:       FOC.H
//  Description: FOC控制相关宏文件
//  (1)FOC_MACRO
//  (2)PI_MACRO
//  (3)ONE_LOW_FILTER_MACRO(一阶低通滤波器)
//  (4)SVGEN_DQ_MACRO(三相SVPWM头文件)
//  (5)SPEED_FR Macro (电机转速计算)
//  (6)SPTA,Handler_SPTA_MACRO,Start_SPTA_MACRO

====================================================================================
//  Version       Date        Author     Description
//    1.0       2015.10.25   wangdayu        use
===================================================================================*/

#ifndef __FOC_H__
#define __FOC_H__


#include "IQmathLib.h"

/*********************(1)--FOC_MACRO--********************************************/
typedef struct {  _iq  I_As;        // Input: phase-a current variable
                  _iq  I_Bs;        // Input: phase-b current variable
                  _iq  I_Alpha;     // Input:
                  _iq  I_Beta;      // Input:
                  _iq  I_Ds;        // Output: rotating d-axis stator variable
                  _iq  I_Qs;        // Output: rotating q-axis stator variable

                  _iq  U_Alpha;     // Input: stationary d-axis stator variable
                  _iq  U_Beta;      // Input: stationary q-axis stator variable
                  _iq  U_Ds;        // Output: rotating d-axis stator variable
                  _iq  U_Qs;        // Output: rotating q-axis stator variable

                  _iq  Angle;       // Input: 电角度 for PARK
                  _iq  Sine;        // Input: Sine term
                  _iq  Cosine;      // Input: Cosine term

                  _iq  Iqref_filter_K2; //Parameter: Constant for low-pass filter (pu)
                  _iq  Iqref_filter_K3; //Parameter: Constant for low-pass filter (pu)
                  _iq  Idqfdb_filter_K2; //Parameter: Constant for low-pass filter (pu)
                  _iq  Idqfdb_filter_K3; //Parameter: Constant for low-pass filter (pu)

                 } FOC;
/*-----------------------------------------------------------------------------
 Default initalizer for the FOC object.
-----------------------------------------------------------------------------*/
 #define FOC_DEFAULTS {0,0,0,0,0,0,\
                       0,0,0,0,\
                       0,0,0,\
                       0,0,0,0}

 /*------------------------------------------------------------------------------
     FOC Transformation Macro Definition   1/sqrt(3) = 0.57735026918963
 ------------------------------------------------------------------------------*/
#define CLARKE_MACRO(v)                                            \
v.I_Alpha = v.I_As;                                                \
v.IBeta = _IQmpy((v.I_As +_IQmpy2(v.I_Bs)),_IQ(0.57735026918963));


#define PARK_MACRO(v)                                          \
v.I_Ds = _IQmpy(v.I_Alpha,v.Cosine) + _IQmpy(v.I_Beta,v.Sine); \
v.I_Qs = _IQmpy(v.I_Beta,v.Cosine) - _IQmpy(v.I_Alpha,v.Sine);

#define CLARKE_PARK_MACRO(v)                                                \
v.I_Alpha = v.I_As;                                                         \
v.I_Beta  = _IQmpy((v.I_As + _IQmpy(_IQ(2),v.I_Bs)),_IQ(0.57735026918963)); \
v.I_Ds    = _IQmpy(v.I_Alpha,v.Cosine) + _IQmpy(v.I_Beta,v.Sine);           \
v.I_Qs    = _IQmpy(v.I_Beta,v.Cosine) - _IQmpy(v.I_Alpha,v.Sine);

#define IPARK_MACRO(v)                                       \
v.U_Alpha = _IQmpy(v.U_Ds,v.Cosine) - _IQmpy(v.U_Qs,v.Sine); \
v.U_Beta  = _IQmpy(v.U_Qs,v.Cosine) + _IQmpy(v.U_Ds,v.Sine);


/*********************(2)--PI_MACRO--********************************************/
typedef struct {  _iq  Ref;        // Input: reference set-point
		 	      _iq  Fdb;        // Input: feedback
		 	      _iq  Out;        // Output: controller output

		 	      _iq  Kp;         // Parameter: proportional loop gain
		 	      _iq  Ki;         // Parameter: integral gain

		 	      _iq  Umax;       // Parameter: upper saturation limit
		 	      _iq  Umin;       // Parameter: lower saturation limit

		 	      _iq  up;         // Data: proportional term
		 	      _iq  ui;         // Data: integral term
		 	      _iq  v1;         // Data: pre-saturated controller output
		 	      _iq  i1;         // Data: integrator storage: ui(k-1)
		 	    } PI_CONTROLLER;
/*-----------------------------------------------------------------------------
	Default initalisation values for the PI objects
-----------------------------------------------------------------------------*/
#define PI_CONTROLLER_DEFAULTS {0,0,0,        \
		 	                     _IQ(1.0),_IQ(0.0), \
		 	                     _IQ(0.707),_IQ(-0.707), \
		 	                     _IQ(0.0), _IQ(0.0),   \
		 	                     _IQ(0.0), _IQ(0.0),   \
		 	                   }
/*------------------------------------------------------------------------------
		PI Macro Definition(防积分饱和)
------------------------------------------------------------------------------*/
#define PI_MACRO(v)                                         \
/* proportional term */                                     \
v.up = v.Ref - v.Fdb;                                       \
/* integral term */                                         \
v.ui = (v.Out == v.v1)?(_IQmpy(v.Ki, v.up)+ v.i1) : v.i1;   \
v.i1 = v.ui;                                                \
/* control output */                                        \
v.v1 = _IQmpy(v.Kp, v.up) + v.ui;                           \
v.Out= _IQsat(v.v1, v.Umax, v.Umin);
/**********************************************************************************/


/*********************(3)--LOWPASS_FILTER_MACRO--********************************************/
typedef struct LOWPASS_FILTER   /*一阶低通滤波器结构体定义*/
 {
	_iq  input;        //Input: 滤波器输入量
	_iq  K2;           //Parameter: Constant for low-pass filter (pu)
	_iq  K3;           //Parameter: Constant for low-pass filter (pu)
	_iq  output_last;  //Variable: temp variable--暂存上次输出信号
	_iq  output;       //Oupput: 滤波器输出量
 } LOWPASS_FILTER;

#define LOWPASS_FILTER_DEFAULTS {0,0,0,0,0}

	/*一阶低通滤波器宏定义*/
#define LOWPASS_FILTER_MACRO(v)                             \
v.output = _IQmpy(v.K2,v.output_last)+_IQmpy(v.K3,v.input); \
v.output_last = v.output;
/**********************************************************************************/

/*********************(4)---SVGEN_DQ_MACRO---****************************************/
 typedef struct  { _iq  Ualpha;      // Input: reference alpha-axis phase voltage
                   _iq  Ubeta;       // Input: reference beta-axis phase voltage
                   _iq  Ta;          // Output: reference phase-a switching function
                   _iq  Tb;          // Output: reference phase-b switching function
                   _iq  Tc;          // Output: reference phase-c switching function
                   _iq  Va;          //临时变量
                   _iq  Vb;          //临时变量
                   _iq  Vc;          //临时变量
                   _iq  t1;          //临时变量
                   _iq  t2;          //临时变量
                   _iq  temp_sv1;    //临时变量
                   _iq  temp_sv2;    //临时变量
                   Uint16 Sector;    // Sector is treated as Q0 - independently with global Q
                  } SVGENDQ;
/*-----------------------------------------------------------------------------
    Default initalizer for the SVGENDQ object.
-----------------------------------------------------------------------------*/
 #define SVGENDQ_DEFAULTS { 0,0,0,0,0,0,0,0,0,0,0,0,0}
/*------------------------------------------------------------------------------
     Space Vector PWM Generator (SVGEN_DQ) Macro Definition
------------------------------------------------------------------------------*/
 #define SVGEN_MACRO(v)                                                            \
                                                                                   \
 v.Sector = 0;                                                                     \
 v.temp_sv1=_IQdiv2(v.Ubeta);                  /*divide by 2*/                     \
 v.temp_sv2=_IQmpy(_IQ(0.8660254),v.Ualpha);   /* 0.8660254 = sqrt(3)/2*/          \
                                                                                   \
 /* Inverse clarke transformation */                                               \
 v.Va = v.Ubeta;                                                                   \
 v.Vb = -v.temp_sv1 + v.temp_sv2;                                                  \
 v.Vc = -v.temp_sv1 - v.temp_sv2;                                                  \
 /* 60 degree Sector determination */                                              \
 if (v.Va>_IQ(0)) v.Sector = 1;                                                    \
 if (v.Vb>_IQ(0)) v.Sector = v.Sector+2;                                           \
 if (v.Vc>_IQ(0)) v.Sector = v.Sector+4;                                           \
 /* X,Y,Z (Va,Vb,Vc) calculations X = Va, Y = Vb, Z = Vc */                        \
 v.Va = v.Ubeta;                                                                   \
 v.Vb = v.temp_sv1 + v.temp_sv2;                                                   \
 v.Vc = v.temp_sv1 - v.temp_sv2;                                                   \
                                                                                   \
 /* Sector 0: this is special case for (Ualpha,Ubeta) = (0,0)*/                    \
 switch(v.Sector)                                                                  \
 {                                                                                 \
     case 0:                                                                       \
     v.Ta = _IQ(0.5);                                                              \
     v.Tb = _IQ(0.5);                                                              \
     v.Tc = _IQ(0.5);                                                              \
     break;                                                                        \
     case 1:   /*Sector 1: t1=Z and t2=Y (abc ---> Tb,Ta,Tc)*/                     \
     v.t1 = v.Vc;                                                                  \
     v.t2 = v.Vb;                                                                  \
     v.Tb=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Ta = v.Tb+v.t1;             /* taon = tbon+t1       */                      \
     v.Tc = v.Ta+v.t2;             /* tcon = taon+t2       */                      \
     break;                                                                        \
     case 2:   /* Sector 2: t1=Y and t2=-X (abc ---> Ta,Tc,Tb)*/                   \
     v.t1 = v.Vb;                                                                  \
     v.t2 = -v.Va;                                                                 \
     v.Ta=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Tc = v.Ta+v.t1;             /*  tcon = taon+t1      */                      \
     v.Tb = v.Tc+v.t2;             /*  tbon = tcon+t2      */                      \
     break;                                                                        \
     case 3:   /* Sector 3: t1=-Z and t2=X (abc ---> Ta,Tb,Tc)*/                   \
     v.t1 = -v.Vc;                                                                 \
     v.t2 = v.Va;                                                                  \
     v.Ta=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Tb = v.Ta+v.t1;             /*  tbon = taon+t1      */                      \
     v.Tc = v.Tb+v.t2;             /*  tcon = tbon+t2      */                      \
     break;                                                                        \
     case 4:   /* Sector 4: t1=-X and t2=Z (abc ---> Tc,Tb,Ta)*/                   \
     v.t1 = -v.Va;                                                                 \
     v.t2 = v.Vc;                                                                  \
     v.Tc=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Tb = v.Tc+v.t1;             /*  tbon = tcon+t1      */                      \
     v.Ta = v.Tb+v.t2;             /*  taon = tbon+t2      */                      \
     break;                                                                        \
     case 5:   /* Sector 5: t1=X and t2=-Y (abc ---> Tb,Tc,Ta)*/                   \
     v.t1 = v.Va;                                                                  \
     v.t2 = -v.Vb;                   /*  tbon = (1-t1-t2)/2  */                    \
     v.Tb=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Tc = v.Tb+v.t1;             /*  taon = tcon+t2      */                      \
     v.Ta = v.Tc+v.t2;                                                             \
     break;                                                                        \
     case 6:   /* Sector 6: t1=-Y and t2=-Z (abc ---> Tc,Ta,Tb)*/                  \
     v.t1 = -v.Vb;                                                                 \
     v.t2 = -v.Vc;                                                                 \
     v.Tc=_IQdiv2((_IQ(1)-v.t1-v.t2));                                             \
     v.Ta = v.Tc+v.t1;             /*  taon = tcon+t1      */                      \
     v.Tb = v.Ta+v.t2;             /*  tbon = taon+t2      */                      \
     break;                                                                        \
 }

 /*  Convert the unsigned GLOBAL_Q format Ta,Tb,Tc (ranged (0,1)) ->.. */
        /*Ta=(0~1)*PWM_MaxPeriod,即得到Duty*/

/********************(6)---SPTA---*********************************************/
/*SPTA控制状态定义*/
#define IDLE                0
#define ACCELERATING        1
#define AT_MAX              2
#define DECELERATING        3

/*SPTA算法控制结构体*/
typedef struct
  {
	  int32   Pulse_Out;       //控制输出量：SPTA输出脉冲
	  int32   Pulse_Out_last;  //控制输出量：SPTA输出脉冲
	  int32   Pulse_e;         //控制输出量：SPTA输出脉冲
	  int32   carry;           //临时计算用变量

      Uint16  step_state ;     //move profile state;电机运转状态变量，加速、运行、减速
	  Uint16  dir;             //输入量：脉冲方向
	  Uint32  step_move ;      //输入量：设定运行的步数(指令输入值）
	  Uint16  flag;            //用于区别GPIO_TOGGLE和正常曲线产生数据
	  Uint16  Pulse_set_Ena;   //曲线产生使能信号

	  Uint32  step_middle;     //mid-point of move, = (step_move - 1) >> 1;设定运行步数的1/2，超过该步数就要减速
	  Uint32  step_count;      //step counter;已经运行的步数

	  Uint32  step_acced ;     //steps in acceled stage;加速阶段的加速步数

	  Uint32  step_accel ;     //accel/decel rate, 8.8 bit format；设定的电机加速度数值
	  Uint32  speed_frac ;     //speed counter fraction;速度片段累加器，每次都累加step_accel，该数值大于某个值后step_speed增加

	  Uint32  step_speed ;     //current speed, 16.8 bit format (HI byte always 0)；步数片段累加速度值，speed_frac溢出值加到这里
	  Uint32  step_frac  ;     //step counter fraction;步数片段累加器，每次累加step_speed，该数值大于某个值后产生一个步进脉冲，运行一步

	  Uint32  step_spmax ;     //maximum speed,设定的电机最大速度

  } SPTA ;
/*-----------------------------------------------------------------------------
	  Default initalizer for the SPTA object.
-----------------------------------------------------------------------------*/
#define SPTA_DEFAULTS { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
/*********************************************************************************/
  /*电机SPTA算法处理宏*/
#define Handle_SPTA_MACRO(pmotor)                                         \
 {                                                                        \
    /*根据步数累加器是否溢出，决定是否产生一个步进脉冲                                                                                           \
         步数累加器由步数速度器累加溢出产生*/                                             \
    pmotor.step_frac += pmotor.step_speed; /*累加 step_speed,溢出后产生脉冲输出*/ \
                                                                          \
  /*判断是否溢出*/                                                         \
    pmotor.carry = pmotor.step_frac >> 16;                                \
    pmotor.step_frac -= pmotor.carry << 16;                               \
    if(pmotor.carry!=0)                                                   \
    {                                                                     \
        /*溢出则产生一个步进脉冲*/                                          \
        pmotor.step_count+=1;                                             \
                                                                          \
        if(pmotor.step_state!=IDLE)/*产生脉冲计数  */                       \
        {                                                                 \
          if(pmotor.dir==0)                                               \
             pmotor.Pulse_Out++;                                          \
          else                                                            \
             pmotor.Pulse_Out--;                                          \
          if(pmotor.flag == 1)  STEPTOGGLE = 1;                            \
        }                                                                 \
    }                                                                     \
                                                                          \
    /*根据电机的状态进行状态转换以及参数变换*/                                        \
    switch(pmotor.step_state)                                             \
    {                                                                     \
        case IDLE: break;                                                 \
   /*******************************************************/              \
         case ACCELERATING:                                               \
            /*计算加速阶段的步数*/                                              \
            if(pmotor.carry)                                              \
            {pmotor.step_acced++;}                                        \
                                                                          \
            /*加速阶段，速度累加器要根据设定的加速度值进行累加*/                         \
            /*pmotor.speed_frac += (pmotor.step_accel*5)>>2;*/ /*升速加速度大于减少加速度*/ \
            pmotor.speed_frac += (pmotor.step_accel);                     \
                                                                          \
            /*溢出判断*/                                                     \
            pmotor.carry= pmotor.speed_frac>>17;                           \
            pmotor.speed_frac -= (pmotor.carry<<17);                       \
            if(pmotor.carry)                                               \
            {                                                              \
              /*如果速度累加器溢出，则步数速度器需要加上该值，*/                          \
              /*以便推动步数速度器的增长,为步数累加器溢出准备*/                          \
                pmotor.step_speed += pmotor.carry;                         \
            }                                                              \
                                                                           \
       /*在非速度控制下，到达了脉冲数中值就要反转为减速*/                                 \
       /*although we are ACCELERATING,but middle point reached,             \
       / * we need DECELERATING*/                                           \
            if(pmotor.step_middle!=0)                                       \
            {                                                               \
                if(pmotor.step_count > pmotor.step_middle)/*输出脉冲数超过一半后开始减速*/ \
                {                                                           \
                   pmotor.step_state = DECELERATING;                        \
                }                                                           \
            }                                                               \
            else if(pmotor.step_count>0)                                    \
            {                                                               \
                /*只运行一步？*/                                                \
                pmotor.step_state = DECELERATING;                           \
            }                                                               \
                                                                            \
            if(pmotor.step_speed >= pmotor.step_spmax)/*速度最大值限幅*/        \
            {                                                               \
                /*加速阶段到达一定的时刻，就达到了设定的最大速度*/                        \
                pmotor.step_speed = pmotor.step_spmax;                      \
               /*转为最大速度状态*/                                              \
                pmotor.step_state = AT_MAX;                                 \
            }                                                               \
            break;                                                          \
     /*******************************************************/              \
        case AT_MAX:                                                        \
        /*在最大速度运行时，如果剩余需要运行的步数小余等于加速步则需要减速了，减速与加速步数相同*/     \
           if( (pmotor.step_move-pmotor.step_count) < pmotor.step_acced )   \
           {pmotor.step_state=DECELERATING;}                                \
         break;                                                             \
     /*******************************************************/              \
        case DECELERATING:                                                  \
            /*减速阶段与加速阶段是相反的过程，原理也相同，只是很多+号变成-号*/                 \
            if(pmotor.carry && pmotor.step_acced>0)                         \
            {pmotor.step_acced--;}                                          \
            pmotor.speed_frac += (pmotor.step_accel);  /*减速加速度*/          \
            pmotor.carry = pmotor.speed_frac>>17;                           \
            pmotor.speed_frac -= (pmotor.carry<<17);                        \
                                                                            \
            if(pmotor.carry && pmotor.step_speed>pmotor.carry)              \
            {pmotor.step_speed -= pmotor.carry;}                            \
                                                                            \
            /*不是速度控制，在运行设定步数以后停止运行*/                                 \
            if(pmotor.step_count >= pmotor.step_move)/*脉冲数已达设定个数*/       \
            {                                                               \
               pmotor.step_state=IDLE; /*设定电机状态为IDLE;*/                   \
            }                                                               \
         break;                                                             \
     /*******************************************************/              \
        default:break;                                                      \
     } /*end of switch*/                                                    \
  }                                                                         \

/*******************************************************************************
      启动电机按照Spta参数运行
      入口参数:  step_dir，运行方向； step_move，要运行的步数
      返回参数： 无
********************************************************************************/
#define Start_SPTA_MACRO(pmotor)                                       \
{                                                                      \
    if(pmotor.step_move!=0)                                            \
    {                                                                  \
      /*电机运行参数初始化*/                                                 \
      pmotor.Pulse_Out = 0;    /*清零SPTA脉冲数*/                         \
      pmotor.Pulse_Out_last = 0;    /*清零SPTA脉冲数*/                    \
      pmotor.Pulse_e = 0;    /*清零SPTA脉冲数*/                           \
      /*FIXME:pmotor.step_middle运行脉冲数的一半*/                          \
      pmotor.step_middle = (pmotor.step_move)>>1; /*运行脉冲数的一半 (是否可设为65*总脉冲数）*/ \
                                                                       \
      /*FIXME:这两个参数可以由用户自行改变测试*/                                 \
     /* pmotor.step_spmax = 100000;SPTA 最大速度设定值*/                   \
     /* pmotor.step_accel =150000 ;SPTA 曲线加速度设定值*/                  \
                                                                       \
      pmotor.step_state = ACCELERATING;/*控制状态设为加速状态*/              \
                                                                       \
      pmotor.step_frac  = 0;                                           \
      pmotor.speed_frac = 0;                                           \
      pmotor.step_acced = 0;                                           \
      pmotor.step_speed = 0;                                           \
      pmotor.step_count = 0;                                           \
   }                                                                   \
}                                                                      \
/******************************************************************************/

/********************(7)-梯形曲线-*********************************************/
//梯形曲线结构体
typedef struct
{
	int16	DeltaDistance;		//单位是脉冲个数
	int32	Distance;
	Uint16	RiseTime;
	Uint16	DownTime;
	Uint16	RunTime;
	Uint16	DelayTime;
	Uint32	RiseTCounter;
	Uint32	DawnTCounter;
	Uint32	RunTCounter;
	Uint32	DelayTCounter;
	int32	MaxFrequency;
	int32	IsrFrequency;
	int32	PositionPulse;
	int32	PositionPulseOld;
	int32	OutputPulseErr;
	Uint32	tCounter;
	Uint16	GenFlg;
	int32	InvRiseT;
	int32	InvDawnT;
	int32	Kt;
	Uint16	SelfEn;
	Uint16	ErrFlg;
	Uint16	ChangeFlg;
	Uint16  SelfRunCounter;
	Uint16  SelfRunLim;
	Uint16	Cur_mode;
}CURVEGEN;

#define CURVEGEN_DEFAULTE 	{0,0,1,1,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,6,0}

//梯形曲线初始化  重新赋值RiseTime，DownTime及DelayTime
inline void CurveInit(CURVEGEN *p)
{
	if((p->RiseTime + p->DownTime)>p->RunTime)
	{
		p->ErrFlg = 1;
		return;
	}
	p->RiseTCounter = p->RiseTime * p->IsrFrequency;
	p->DawnTCounter = p->DownTime * p->IsrFrequency;
	p->RunTCounter = p->RunTime * p->IsrFrequency;
	p->DelayTCounter = p->DelayTime * p->IsrFrequency;
	p->InvRiseT = (p->RiseTime==0)?(0):(((int32)1000<<21)/p->RiseTime);//Q21
	p->InvDawnT = (p->DownTime==0)?(0):(((int32)1000<<21)/p->DownTime);//Q21
}
#pragma CODE_SECTION(CurveGen,"ramfuncs");
inline void CurveGen(CURVEGEN *p)
{

	volatile int16 tmp;
	volatile int64 tmp1;
	volatile int64 tmp2,tmp3;

	if(p->GenFlg == 2)
	{
		if(p->DeltaDistance!=0)
		{
			if(p->ChangeFlg == 1)
			{
				p->ChangeFlg = 0;
				CurveInit(p);
			}
			p->tCounter = 0;
			p->PositionPulseOld = 0;
			p->PositionPulse = 0;
			p->GenFlg = 1;
			p->Distance = (int32)p->DeltaDistance;
			tmp = (p->RunTime<<1) - p->RiseTime - p->DownTime;
			p->MaxFrequency = (((int64)p->Distance)*2*1000)/tmp;//Q0
		}
	}
	else
	{
		 if(p->Cur_mode==0)
		  {

			p->tCounter++;
		  }
		if(p->tCounter <= p->RiseTCounter)
		{
			tmp1 = ((int32)p->tCounter*p->Kt);//Q30
			tmp2 = ((int64)tmp1*tmp1)>>30;//Q30
			tmp3 = (tmp2*p->InvRiseT)>>21;//Q30
			p->PositionPulse = ((int64)p->MaxFrequency*tmp3)>>31;
		}
		if((p->tCounter > p->RiseTCounter)&&(p->tCounter <= (p->RunTCounter - p->DawnTCounter)))
		{
			tmp1 = ((int64)(p->tCounter*2 - p->RiseTCounter)*p->Kt);//Q30
			p->PositionPulse = ((int64)p->MaxFrequency * tmp1)>>31;
		}
		if((p->tCounter > (p->RunTCounter - p->DawnTCounter))&&(p->tCounter < p->RunTCounter))
		{
			tmp1 = ((int64)(p->DawnTCounter + p->tCounter-p->RunTCounter)*p->Kt);//Q30
			tmp2 = ((int64)p->InvDawnT*tmp1)>>21;//Q30
			tmp2 = ((int64)p->MaxFrequency*tmp2)>>15;//Q15
			tmp2 = ((int64)tmp2 * tmp1)>>15;//Q30
			tmp3 = (((int64)p->tCounter*2 - p->RiseTCounter)*p->Kt);//Q30
			tmp3 = ((int64)(p->MaxFrequency)*tmp3);//Q30
			p->PositionPulse = (tmp3 - tmp2)>>31;
		}
		if(p->tCounter >= p->RunTCounter)
		{
			p->PositionPulse = p->Distance;
		}
		if(p->tCounter > (p->RunTCounter+p->DelayTCounter))
		{
			p->GenFlg = 2;
			p->tCounter=0;
			if(p->SelfEn)
			{
				p->DeltaDistance = -p->DeltaDistance;//自转模式
				//GpioDataRegs.GPATOGGLE.bit.GPIO9=1;//气阀
				//p->SelfRunCounter++;
				if(p->SelfRunCounter>=p->SelfRunLim)
				{p->SelfEn = 0;}
			}
			else
			{
				p->DeltaDistance = 0;//等待外部指令
			}

			//p->GenFlg = 0;
		}
		if(p->Cur_mode==0)
		{
			p->OutputPulseErr = p->PositionPulse - p->PositionPulseOld;
			p->PositionPulseOld = p->PositionPulse;
		}
	}
}

#endif // __FOC_H__

