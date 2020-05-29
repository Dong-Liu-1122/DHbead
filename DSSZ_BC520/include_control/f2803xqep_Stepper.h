/* =================================================================================
File name:        F2803XQEP_Stepper.H                     
                    
Originator:	Digital Control Systems Group
			Texas Instruments
Description:  
Header file containing data type, object, macro definitions and initializers. 
====================================================================================
History: 07-28-2010	Version 1.0
----------------------------------------------------------------------------------*/

#ifndef __F2803X_QEP_H__
#define __F2803X_QEP_H__

#include "f2803xbmsk.h"

/*-----------------------------------------------------------------------------
    Initialization states for EQEP Decode Control Register
------------------------------------------------------------------------------*/
#define QDECCTL_INIT_STATE     ( XCR_X2 + QSRC_QUAD_MODE )  //��������4��Ƶ��ѡ��QEPģʽ

/*-----------------------------------------------------------------------------
    Initialization states for EQEP Control Register
------------------------------------------------------------------------------*/
/*Z�źŸ�λ*/
/*
#define QEPCTL_INIT_STATE      ( QEP_EMULATION_FREE + \
                                 PCRM_INDEX + \
                                 IEI_RISING + \
                                 IEL_RISING + \
                                 QPEN_ENABLE + \
                                 QCLM_TIME_OUT + \
                                 UTE_ENABLE )  
*/

 /*�������ֵ��λ*/
#define QEPCTL_INIT_STATE      ( QEP_EMULATION_FREE + \
                                 PCRM_POSMAX + \
                                 IEL_FALLING + \
                                 QPEN_ENABLE + \
                                 QCLM_TIME_OUT + \
                                 UTE_ENABLE )

/*-----------------------------------------------------------------------------
    Initialization states for EQEP Position-Compare Control Register
------------------------------------------------------------------------------*/
#define QPOSCTL_INIT_STATE    PCE_DISABLE

/*-----------------------------------------------------------------------------
    Initialization states for EQEP Capture Control Register
------------------------------------------------------------------------------*/
/*����һ������*/
#define QCAPCTL_INIT_STATE    ( UPPS_X1 + CCPS_X1 + CEN_DISABLE )

/*-----------------------------------------------------------------------------
Define the structure of the QEP (Quadrature Encoder) Driver Object 
-----------------------------------------------------------------------------*/
typedef struct {int32 ElecTheta;        // Output: Motor Electrical angle (Q24)
                int32 MechTheta;        // Output: Motor Mechanical Angle (Q24) 
                Uint16 DirectionQep;    // Output: Motor rotation direction (Q0)
                Uint32 QepCountIndex;   // Variable: Encoder counter index (Q0) 
                 int16 RawTheta;        // Variable: Raw angle from EQEP Postiion counter (Q0)
                Uint32 MechScaler;      // Parameter: 0.9999/total count (Q30) 
                Uint16 LineEncoder;     // Parameter: Number of line encoder (Q0) 
                Uint16 PolePairs;       // Parameter: Number of pole pairs (Q0) 
                 int32 CalibratedAngle; // Parameter: Raw angular offset between encoder index and phase a (Q0)
                Uint16 IndexSyncFlag;   // Output: Index sync status (Q0) 0x0F:��ʾ��λ������(Ĭ��ֵ��Ҫ�ϵ綨λ)��0x00;��ʾ��λ���δ�ҵ�ԭ�㣻0XF0����ʾ�ҵ�ԭ��
                Uint16 OriginalFlag; 	//ԭʼ�Ƕȸ�ֵ��־
                int16 qeptmp;
                }  QEP;
/*-----------------------------------------------------------------------------
Default initializer for the QEP Object.
-----------------------------------------------------------------------------*/
#define QEP_DEFAULTS { 0x0,0x0,0x0,0x0,0x0,0x00020000,0x0,2,0,0x0F,0,0}

/*-----------------------------------------------------------------------------
	QEP Init and QEP Update Macro Definitions    eQEP����ܽŲ�������ΪAsynchronous�첽ģʽ
-----------------------------------------------------------------------------*/
#define QEP_INIT_MACRO(v)													\
																			\
     EQep1Regs.QDECCTL.all = QDECCTL_INIT_STATE;							\
     /*EQep1Regs.QDECCTL.bit.QIP = 1; Negates QEPI input*/                  \
     EQep1Regs.QEPCTL.all  = QEPCTL_INIT_STATE;								\
     EQep1Regs.QPOSCTL.all = QPOSCTL_INIT_STATE;							\
     EQep1Regs.QUPRD = 600000;	/* Unit Timer for 100Hz*/               	\
     EQep1Regs.QCAPCTL.all = QCAPCTL_INIT_STATE;							\
     EQep1Regs.QPOSMAX = 4*v.LineEncoder - 1;								\
     EQep1Regs.QCLR.all= 0xFFFF;  /*���жϱ�־*/                               \
     	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	\
                                                                            \
     EALLOW;                       /* Enable EALLOW*/						\
     /*ʹ��DSPƬ����������*/                                                     \
     GpioCtrlRegs.GPBPUD.bit.GPIO50 = 0;  /*Enable pull-up on GPIO20 */     \
     GpioCtrlRegs.GPBPUD.bit.GPIO51 = 0;  /*Enable pull-up on GPIO20 */     \
     GpioCtrlRegs.GPBPUD.bit.GPIO53 = 0;  /*Enable pull-up on GPIO23 */     \
                                                                            \
     GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 1;  /* GPIO20 is EQEP1A */			\
     GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 1;  /* GPIO21 is EQEP1B */			\
     GpioCtrlRegs.GPBMUX2.bit.GPIO53 = 1;  /* GPIO23 is EQEP1I */			\
                                                                            \
     /*����ʱ�����ã�Specifies the sampling period for pins GPIO16 to GPIO31*/               \
     GpioCtrlRegs.GPBCTRL.bit.QUALPRD2= 1; /* 00~0xFF,1--Sampling Period =2*TSYSCLKOUT&*QUALPRD2 */\
                                                                                         \
 /*00  synchronize to SYSCLKOUT only. Valid for both peripheral and GPIO pins. */        \
 /*01  Qualification using 3 samples. Valid for both peripheral and GPIO pins. */        \
 /*10  Qualification using 6 samples. Valid for both peripheral and GPIO pins. */        \
 /*11  Asynchronous. (no synchronization or qualification).Valid for peripherals only.*/ \
       /*11  The asynchronous mode should not be used for eQEP module input pins.*/      \
     GpioCtrlRegs.GPBQSEL2.bit.GPIO50 = 1;  /* GPIO20--Qualification using 6 samples*/   \
     GpioCtrlRegs.GPBQSEL2.bit.GPIO51 = 1;  /* GPIO21--Qualification using 6 samples*/   \
     GpioCtrlRegs.GPBQSEL2.bit.GPIO53 = 1;  /*��λ�ź�����ʱ��1us GPIO23--Qualification using 6 samples*/   \
     EDIS;                         /* Disable EALLOW*/                                    \
                                                                                           \
      EQep2Regs.QDECCTL.all = QDECCTL_INIT_STATE;							\
	  /*EQep1Regs.QDECCTL.bit.QIP = 1; Negates QEPI input*/                  \
	  EQep2Regs.QEPCTL.all  = QEPCTL_INIT_STATE;								\
	  EQep2Regs.QPOSCTL.all = QPOSCTL_INIT_STATE;							\
	  EQep2Regs.QUPRD = 600000;	/* Unit Timer for 100Hz*/               	\
	  EQep2Regs.QCAPCTL.all = QCAPCTL_INIT_STATE;							\
	  EQep2Regs.QPOSMAX = 4*v.LineEncoder - 1;							\
	  EQep2Regs.QCLR.all= 0xFFFF;  /*���жϱ�־*/                               \
																			\
																			 \
	  EALLOW;                       /* Enable EALLOW*/						\
	  /*ʹ��DSPƬ����������*/                                                     \
	  GpioCtrlRegs.GPBPUD.bit.GPIO54 = 0;  /*Enable pull-up on GPIO20 */     \
	  GpioCtrlRegs.GPBPUD.bit.GPIO55 = 0;  /*Enable pull-up on GPIO20 */     \
      GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0;  /*Enable pull-up on GPIO20 */   		\
	  GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 2;  /* GPIO20 is EQEP1A */			\
	  GpioCtrlRegs.GPBMUX2.bit.GPIO55 = 2;  /* GPIO21 is EQEP1B */			\
	  GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 2; 								\
	  /*����ʱ�����ã�Specifies the sampling period for pins GPIO48 to GPIO55*/               \
	  GpioCtrlRegs.GPBCTRL.bit.QUALPRD2= 1; /* 00~0xFF,1--Sampling Period =2*TSYSCLKOUT&*QUALPRD2 */\
																						  \
  /*00  synchronize to SYSCLKOUT only. Valid for both peripheral and GPIO pins. */        \
  /*01  Qualification using 3 samples. Valid for both peripheral and GPIO pins. */        \
  /*10  Qualification using 6 samples. Valid for both peripheral and GPIO pins. */        \
  /*11  Asynchronous. (no synchronization or qualification).Valid for peripherals only.*/ \
		/*11  The asynchronous mode should not be used for eQEP module input pins.*/      \
	  GpioCtrlRegs.GPBQSEL2.bit.GPIO54 = 1;  /* GPIO20--Qualification using 6 samples*/   \
	  GpioCtrlRegs.GPBQSEL2.bit.GPIO55 = 1;  /* GPIO21--Qualification using 6 samples*/   \
	  GpioCtrlRegs.GPBQSEL2.bit.GPIO56 = 1;                           \
	  EDIS;                         /* Disable EALLOW*/

//���ü��������ֵ����λeQEP������(�����ջ�ʵ��ʹ�ã�
#define QEP1_MACRO(v)																			\
																								\
/* Check the rotational direction */					                                        \
    v.DirectionQep = EQep1Regs.QEPSTS.bit.QDF;                                                  \
																								\
/* Check the position counter for EQEP1  */									                    \
     v.RawTheta = EQep1Regs.QPOSCNT- v.CalibratedAngle;  /* + v.QepCountIndex; */				\
      if (v.RawTheta < 0)                                                                       \
          v.RawTheta = v.RawTheta + EQep1Regs.QPOSMAX;                                          \
      else if (v.RawTheta > EQep1Regs.QPOSMAX)                                                  \
          v.RawTheta = v.RawTheta - EQep1Regs.QPOSMAX;   										\
                                                                                                \
/* Compute the mechanical angle in Q24(���ڵ��ת�ټ���) */											\
     v.MechTheta = v.MechScaler * v.RawTheta;  /* Q30 = Q30*Q0 */								\
     v.MechTheta &= 0x3FFFFFFF;                /* Wrap around 0x3FFFFFFF */	                    \
     v.MechTheta >>= 6;                        /* Q30 -> Q24 */			                        \
																								\
/* Compute the electrical angle in Q24 (����foc����)*/												\
     v.ElecTheta = v.PolePairs*v.MechTheta;    /* Q24 = Q0*Q24 */						        \
     v.ElecTheta &= 0x00FFFFFF;                /* Wrap around 0x00FFFFFF*/				        \
                                                                                                \
    if(qep1.IndexSyncFlag != 0x0F)/*0x0F��־����������λ�ж�*/                                        \
    {                                                                                           \
       /* Check an index occurrence,ʹ��Z�źŸ�λʱ��Ҫ���д���*/                                       \
      if (EQep1Regs.QFLG.bit.IEL == 1)                                                          \
      {                                                                                         \
         v.IndexSyncFlag = 0x00F0;   /*�ҵ���λZ��־*/                                              \
         EQep1Regs.QCLR.bit.IEL = 1; /* Clear interrupt flag */                                 \
      }                                                                                         \
    }


#define QEP2_MACRO(v)																			\
																								\
/* Check the rotational direction */					                                        \
    v.DirectionQep = EQep2Regs.QEPSTS.bit.QDF;                                                  \
																								\
/* Check the position counter for EQEP1  */									                    \
     v.RawTheta = EQep2Regs.QPOSCNT- v.CalibratedAngle;  /* + v.QepCountIndex; */				\
      if (v.RawTheta < 0)                                                                       \
          v.RawTheta = v.RawTheta + EQep2Regs.QPOSMAX;                                          \
      else if (v.RawTheta > EQep2Regs.QPOSMAX)                                                  \
          v.RawTheta = v.RawTheta - EQep2Regs.QPOSMAX;   										\
                                                                                                \
/* Compute the mechanical angle in Q24(���ڵ��ת�ټ���) */											\
     v.MechTheta = v.MechScaler * v.RawTheta;  /* Q30 = Q30*Q0 */								\
     v.MechTheta &= 0x3FFFFFFF;                /* Wrap around 0x3FFFFFFF */	                    \
     v.MechTheta >>= 6;                        /* Q30 -> Q24 */			                        \
																								\
/* Compute the electrical angle in Q24 (����foc����)*/												\
     v.ElecTheta = v.PolePairs*v.MechTheta;    /* Q24 = Q0*Q24 */						        \
     v.ElecTheta &= 0x00FFFFFF;                /* Wrap around 0x00FFFFFF*/				        \
                                                                                                \
    if(qep2.IndexSyncFlag != 0x0F)/*0x0F��־����������λ�ж�*/                                        \
    {                                                                                           \
       /* Check an index occurrence,ʹ��Z�źŸ�λʱ��Ҫ���д���*/                                       \
      if (EQep2Regs.QFLG.bit.IEL == 1)                                                          \
      {                                                                                         \
         v.IndexSyncFlag = 0x00F0;   /*�ҵ���λZ��־*/                                              \
         EQep2Regs.QCLR.bit.IEL = 1; /* Clear interrupt flag */                                 \
      }                                                                                         \
    }



//----------------------���ת�ټ���(���õ�Ƕȼ���)-------------------------------//
typedef struct {
                 _iq ElecTheta;      // Input: Electrical angle(pu)
                 _iq OldElecTheta;   // History: Electrical angle at previous step(pu)
                 _iq Speed;          // Output: Speed in per-unit(pu)

                 Uint32 BaseRpm;     // Parameter: Base speed in rpm (Q0) - independently with global Q
                 _iq21 K1;           // Parameter: Constant for differentiator (Q21) - independently with global Q
                 _iq K2;             // Parameter: Constant for low-pass filter (pu)
                 _iq K3;             // Parameter: Constant for low-pass filter (pu)

                 int32 SpeedRpm;     // Output : Speed in rpm  (Q0) - independently with global Q
                 _iq   RpmScaler;    // ת�ٱ任ϵ��--1/4500
                 _iq DeltaTheta;      //��Ƕ�����ֵ
                 _iq Tmp1;            //��ʱ����
                 } SPEED_MEAS_QEP;    // Data type created

/*-----------------------------------------------------------------------------
Default initalizer for the SPEED_MEAS_QEP object.
-----------------------------------------------------------------------------*/
#define SPEED_MEAS_QEP_DEFAULTS   { 0,0,0,\
                                    0,0,0,0,\
                                    0,0,0,0}

/*------------------------------------------------------------------------------
 SPEED_FR Macro Definition
------------------------------------------------------------------------------*/
//���õ�Ƕȼ�����ת��
#define SPEED_FR_MACRO(v)                              \
                                                        \
    v.DeltaTheta = v.ElecTheta - v.OldElecTheta;        \
    if(v.DeltaTheta > _IQ(0.5))                         \
    {                                                   \
        v.DeltaTheta = v.DeltaTheta - _IQ(1.0);         \
    }                                                   \
    if(v.DeltaTheta < -_IQ(0.5))                         \
    {                                                   \
        v.DeltaTheta = v.DeltaTheta + _IQ(1.0);          \
    }                                                   \
    v.Tmp1 = _IQmpy(v.K1,v.DeltaTheta);  /*IQ(21)*/     \
                                                        \
    /* Low-pass filter */                               \
    /* Q21 = Q0*GLOBAL_Q*Q21+GLOBAL_Q*Q21 */            \
    v.Tmp1 = _IQmpy(v.K2,_IQtoIQ21(v.Speed))+_IQmpy(v.K3,v.Tmp1); \
                                                          \
    /* Saturate the output */                             \
    v.Tmp1 = _IQsat(v.Tmp1,_IQ21(1),_IQ21(-1));           \
    v.Speed = _IQ21toIQ(v.Tmp1);                          \
    /* ���ת�ٷ���С��0.12rpmʱ���������ֱ�ӵ���0*/                \
     if(_IQabs(v.Speed)<_IQ(0.00005))                     \
     v.Speed = 0;                                         \
    /* Update the electrical angle */                     \
   v.OldElecTheta = v.ElecTheta;                          \
                                                          \
/*Change motor speed from pu value to rpm value (GLOBAL_Q -> Q0)*/  \
/* Q0 = Q0*GLOBAL_Q => _IQXmpy(),X=GLOBAL_Q */            \
   v.SpeedRpm = _IQmpy(v.BaseRpm,v.Speed);                \



#endif // __F2803X_QEP_H__

