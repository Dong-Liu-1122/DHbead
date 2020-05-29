 /* =================================================================================
//  File name:       FOC.H
//  Description: FOC������غ��ļ�
//  (1)FOC_MACRO
//  (2)PI_MACRO
//  (3)ONE_LOW_FILTER_MACRO(һ�׵�ͨ�˲���)
//  (4)SVGEN_DQ_MACRO(����SVPWMͷ�ļ�)
//  (5)SPEED_FR Macro (���ת�ټ���)
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

                  _iq  Angle;       // Input: ��Ƕ� for PARK
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
		PI Macro Definition(�����ֱ���)
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
typedef struct LOWPASS_FILTER   /*һ�׵�ͨ�˲����ṹ�嶨��*/
 {
	_iq  input;        //Input: �˲���������
	_iq  K2;           //Parameter: Constant for low-pass filter (pu)
	_iq  K3;           //Parameter: Constant for low-pass filter (pu)
	_iq  output_last;  //Variable: temp variable--�ݴ��ϴ�����ź�
	_iq  output;       //Oupput: �˲��������
 } LOWPASS_FILTER;

#define LOWPASS_FILTER_DEFAULTS {0,0,0,0,0}

	/*һ�׵�ͨ�˲����궨��*/
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
                   _iq  Va;          //��ʱ����
                   _iq  Vb;          //��ʱ����
                   _iq  Vc;          //��ʱ����
                   _iq  t1;          //��ʱ����
                   _iq  t2;          //��ʱ����
                   _iq  temp_sv1;    //��ʱ����
                   _iq  temp_sv2;    //��ʱ����
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
        /*Ta=(0~1)*PWM_MaxPeriod,���õ�Duty*/

/********************(6)---SPTA---*********************************************/
/*SPTA����״̬����*/
#define IDLE                0
#define ACCELERATING        1
#define AT_MAX              2
#define DECELERATING        3

/*SPTA�㷨���ƽṹ��*/
typedef struct
  {
	  int32   Pulse_Out;       //�����������SPTA�������
	  int32   Pulse_Out_last;  //�����������SPTA�������
	  int32   Pulse_e;         //�����������SPTA�������
	  int32   carry;           //��ʱ�����ñ���

      Uint16  step_state ;     //move profile state;�����ת״̬���������١����С�����
	  Uint16  dir;             //�����������巽��
	  Uint32  step_move ;      //���������趨���еĲ���(ָ������ֵ��
	  Uint16  flag;            //��������GPIO_TOGGLE���������߲�������
	  Uint16  Pulse_set_Ena;   //���߲���ʹ���ź�

	  Uint32  step_middle;     //mid-point of move, = (step_move - 1) >> 1;�趨���в�����1/2�������ò�����Ҫ����
	  Uint32  step_count;      //step counter;�Ѿ����еĲ���

	  Uint32  step_acced ;     //steps in acceled stage;���ٽ׶εļ��ٲ���

	  Uint32  step_accel ;     //accel/decel rate, 8.8 bit format���趨�ĵ�����ٶ���ֵ
	  Uint32  speed_frac ;     //speed counter fraction;�ٶ�Ƭ���ۼ�����ÿ�ζ��ۼ�step_accel������ֵ����ĳ��ֵ��step_speed����

	  Uint32  step_speed ;     //current speed, 16.8 bit format (HI byte always 0)������Ƭ���ۼ��ٶ�ֵ��speed_frac���ֵ�ӵ�����
	  Uint32  step_frac  ;     //step counter fraction;����Ƭ���ۼ�����ÿ���ۼ�step_speed������ֵ����ĳ��ֵ�����һ���������壬����һ��

	  Uint32  step_spmax ;     //maximum speed,�趨�ĵ������ٶ�

  } SPTA ;
/*-----------------------------------------------------------------------------
	  Default initalizer for the SPTA object.
-----------------------------------------------------------------------------*/
#define SPTA_DEFAULTS { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
/*********************************************************************************/
  /*���SPTA�㷨�����*/
#define Handle_SPTA_MACRO(pmotor)                                         \
 {                                                                        \
    /*���ݲ����ۼ����Ƿ�����������Ƿ����һ����������                                                                                           \
         �����ۼ����ɲ����ٶ����ۼ��������*/                                             \
    pmotor.step_frac += pmotor.step_speed; /*�ۼ� step_speed,���������������*/ \
                                                                          \
  /*�ж��Ƿ����*/                                                         \
    pmotor.carry = pmotor.step_frac >> 16;                                \
    pmotor.step_frac -= pmotor.carry << 16;                               \
    if(pmotor.carry!=0)                                                   \
    {                                                                     \
        /*��������һ����������*/                                          \
        pmotor.step_count+=1;                                             \
                                                                          \
        if(pmotor.step_state!=IDLE)/*�����������  */                       \
        {                                                                 \
          if(pmotor.dir==0)                                               \
             pmotor.Pulse_Out++;                                          \
          else                                                            \
             pmotor.Pulse_Out--;                                          \
          if(pmotor.flag == 1)  STEPTOGGLE = 1;                            \
        }                                                                 \
    }                                                                     \
                                                                          \
    /*���ݵ����״̬����״̬ת���Լ������任*/                                        \
    switch(pmotor.step_state)                                             \
    {                                                                     \
        case IDLE: break;                                                 \
   /*******************************************************/              \
         case ACCELERATING:                                               \
            /*������ٽ׶εĲ���*/                                              \
            if(pmotor.carry)                                              \
            {pmotor.step_acced++;}                                        \
                                                                          \
            /*���ٽ׶Σ��ٶ��ۼ���Ҫ�����趨�ļ��ٶ�ֵ�����ۼ�*/                         \
            /*pmotor.speed_frac += (pmotor.step_accel*5)>>2;*/ /*���ټ��ٶȴ��ڼ��ټ��ٶ�*/ \
            pmotor.speed_frac += (pmotor.step_accel);                     \
                                                                          \
            /*����ж�*/                                                     \
            pmotor.carry= pmotor.speed_frac>>17;                           \
            pmotor.speed_frac -= (pmotor.carry<<17);                       \
            if(pmotor.carry)                                               \
            {                                                              \
              /*����ٶ��ۼ�������������ٶ�����Ҫ���ϸ�ֵ��*/                          \
              /*�Ա��ƶ������ٶ���������,Ϊ�����ۼ������׼��*/                          \
                pmotor.step_speed += pmotor.carry;                         \
            }                                                              \
                                                                           \
       /*�ڷ��ٶȿ����£���������������ֵ��Ҫ��תΪ����*/                                 \
       /*although we are ACCELERATING,but middle point reached,             \
       / * we need DECELERATING*/                                           \
            if(pmotor.step_middle!=0)                                       \
            {                                                               \
                if(pmotor.step_count > pmotor.step_middle)/*�������������һ���ʼ����*/ \
                {                                                           \
                   pmotor.step_state = DECELERATING;                        \
                }                                                           \
            }                                                               \
            else if(pmotor.step_count>0)                                    \
            {                                                               \
                /*ֻ����һ����*/                                                \
                pmotor.step_state = DECELERATING;                           \
            }                                                               \
                                                                            \
            if(pmotor.step_speed >= pmotor.step_spmax)/*�ٶ����ֵ�޷�*/        \
            {                                                               \
                /*���ٽ׶ε���һ����ʱ�̣��ʹﵽ���趨������ٶ�*/                        \
                pmotor.step_speed = pmotor.step_spmax;                      \
               /*תΪ����ٶ�״̬*/                                              \
                pmotor.step_state = AT_MAX;                                 \
            }                                                               \
            break;                                                          \
     /*******************************************************/              \
        case AT_MAX:                                                        \
        /*������ٶ�����ʱ�����ʣ����Ҫ���еĲ���С����ڼ��ٲ�����Ҫ�����ˣ���������ٲ�����ͬ*/     \
           if( (pmotor.step_move-pmotor.step_count) < pmotor.step_acced )   \
           {pmotor.step_state=DECELERATING;}                                \
         break;                                                             \
     /*******************************************************/              \
        case DECELERATING:                                                  \
            /*���ٽ׶�����ٽ׶����෴�Ĺ��̣�ԭ��Ҳ��ͬ��ֻ�Ǻܶ�+�ű��-��*/                 \
            if(pmotor.carry && pmotor.step_acced>0)                         \
            {pmotor.step_acced--;}                                          \
            pmotor.speed_frac += (pmotor.step_accel);  /*���ټ��ٶ�*/          \
            pmotor.carry = pmotor.speed_frac>>17;                           \
            pmotor.speed_frac -= (pmotor.carry<<17);                        \
                                                                            \
            if(pmotor.carry && pmotor.step_speed>pmotor.carry)              \
            {pmotor.step_speed -= pmotor.carry;}                            \
                                                                            \
            /*�����ٶȿ��ƣ��������趨�����Ժ�ֹͣ����*/                                 \
            if(pmotor.step_count >= pmotor.step_move)/*�������Ѵ��趨����*/       \
            {                                                               \
               pmotor.step_state=IDLE; /*�趨���״̬ΪIDLE;*/                   \
            }                                                               \
         break;                                                             \
     /*******************************************************/              \
        default:break;                                                      \
     } /*end of switch*/                                                    \
  }                                                                         \

/*******************************************************************************
      �����������Spta��������
      ��ڲ���:  step_dir�����з��� step_move��Ҫ���еĲ���
      ���ز����� ��
********************************************************************************/
#define Start_SPTA_MACRO(pmotor)                                       \
{                                                                      \
    if(pmotor.step_move!=0)                                            \
    {                                                                  \
      /*������в�����ʼ��*/                                                 \
      pmotor.Pulse_Out = 0;    /*����SPTA������*/                         \
      pmotor.Pulse_Out_last = 0;    /*����SPTA������*/                    \
      pmotor.Pulse_e = 0;    /*����SPTA������*/                           \
      /*FIXME:pmotor.step_middle������������һ��*/                          \
      pmotor.step_middle = (pmotor.step_move)>>1; /*������������һ�� (�Ƿ����Ϊ65*����������*/ \
                                                                       \
      /*FIXME:�����������������û����иı����*/                                 \
     /* pmotor.step_spmax = 100000;SPTA ����ٶ��趨ֵ*/                   \
     /* pmotor.step_accel =150000 ;SPTA ���߼��ٶ��趨ֵ*/                  \
                                                                       \
      pmotor.step_state = ACCELERATING;/*����״̬��Ϊ����״̬*/              \
                                                                       \
      pmotor.step_frac  = 0;                                           \
      pmotor.speed_frac = 0;                                           \
      pmotor.step_acced = 0;                                           \
      pmotor.step_speed = 0;                                           \
      pmotor.step_count = 0;                                           \
   }                                                                   \
}                                                                      \
/******************************************************************************/

/********************(7)-��������-*********************************************/
//�������߽ṹ��
typedef struct
{
	int16	DeltaDistance;		//��λ���������
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

//�������߳�ʼ��  ���¸�ֵRiseTime��DownTime��DelayTime
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
				p->DeltaDistance = -p->DeltaDistance;//��תģʽ
				//GpioDataRegs.GPATOGGLE.bit.GPIO9=1;//����
				//p->SelfRunCounter++;
				if(p->SelfRunCounter>=p->SelfRunLim)
				{p->SelfEn = 0;}
			}
			else
			{
				p->DeltaDistance = 0;//�ȴ��ⲿָ��
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

