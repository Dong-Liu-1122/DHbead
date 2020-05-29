/* ==================================================================================
File name:        F2803xPWM.H
Originator:	 Digital Control Systems Group Texas Instruments
Description: Header file containing data type, object, macro definitions and
			 initializers.
Target: TMS320F2803x family
=====================================================================================
History: 2015-12-10	Version 1.0
------------------------------------------------------------------------------------*/
#ifndef __F280X_PWM_H__
#define __F280X_PWM_H__

#include "f2803xbmsk.h"
#include "F2806x_EPwm_defines.h"

//50kHz--PWM����ֵ=600
//40kHz--PWM����ֵ=750
//30kHz--PWM����ֵ=1000
//20kHz--PWM����ֵ=1500


//Drv8412 Minimum PWM pulse duration = 50ns
//Drv8412 PWM switching frequency <=500kHz
#define Duty_min_set  45  //��С����ʱ��>=2*DT(����ʱ��)=500ns
#define Deadtime_set  45  //����ʱ��=30*11.1ns=500ns
/*----------------------------------------------------------------------------
//step1: ePWMģ��ʱ��TB����
----------------------------------------------------------------------------*/
#define PWM_INIT_STATE ( FREE_RUN_FLAG +          \
                         PRDLD_IMMEDIATE  +       \
                         TIMER_CNT_UPDN +         \
                         HSPCLKDIV_PRESCALE_X_1 + \
                         CLKDIV_PRESCALE_X_1  +   \
                         PHSDIR_CNT_UP    +       \
                         CNTLD_DISABLE )

 /*PRDLD_IMMEDIATE--ʱ����������װ�� */
/*TIMER_CNT_UPDN ��������ģʽ*/
/*HSPCLKDIV_PRESCALE_X_1 ��������ʱ�Ӳ���Ƶ*/
/*CLKDIV_PRESCALE_X_1ʱ��ʱ�Ӳ���Ƶ*/
 /*PHSDIR_CNT_UPͬ���¼�����ʱ������ */
/*ʱ�����ڼĴ���TBPRD��Ӱ�ӼĴ���װ��ֵ*/

/*----------------------------------------------------------------------------
//step2: �ȽϿ��ƼĴ�������(Counter and Compare)
----------------------------------------------------------------------------*/
//�ȽϿ��ƼĴ������ã�����Ӱ��ģʽ���ڼ���ֵCTR=0ʱ������ʱ����CTR=PRDʱ,��Ӱ�ӼĴ���װ�رȽ�ֵ
//LOADAMODE_ZRO
//LOADAMODE_ZRO_PRD(��ӦPWM˫����)
#define CMPCTL_INIT_STATE ( LOADAMODE_ZRO     + LOADBMODE_ZRO + \
                            SHDWAMODE_SHADOW  + SHDWBMODE_SHADOW )

/*----------------------------------------------------------------------------
//step3: ����������ƼĴ�������(AQ)
----------------------------------------------------------------------------*/
//��������޶����ƼĴ���A
#define AQCTLA_INIT_STATE ( CAU_SET + CAD_CLEAR )  //PWM����Ч
//��������޶����ƼĴ���B
#define AQCTLB_INIT_STATE ( CAU_CLEAR + CAD_SET )  //PWM����Ч

/*----------------------------------------------------------------------------
//step4: ����ģ������(Dead-Band)
----------------------------------------------------------------------------*/
//����ģ������
//�趨����ʱ�䣬�Լ�������Ӧ�ĵ�ƽ
#define DBCTL_INIT_STATE  (BP_DISABLE)    //��ʹ������
//#define DBCTL_INIT_STATE  (BP_ENABLE + POLSEL_ACTIVE_LO_CMP ) //ʹ������(������Ӧ�ߵ�ƽ��
//#define DBCTL_INIT_STATE  (BP_ENABLE + POLSEL_ACTIVE_HI_CMP )  //ʹ������(������Ӧ�͵�ƽ��

/*----------------------------------------------------------------------------
//step5: ն��ģ������(Dead-Band)
----------------------------------------------------------------------------*/
#define  PCCTL_INIT_STATE  CHPEN_DISABLE   //ն������ģ�鲻ʹ��

/*----------------------------------------------------------------------------
//step6: ���ϴ���ģ������(Trip-Zone)
----------------------------------------------------------------------------*/
//TZģ���¼���������
#define  TZSEL_INIT_STATE ( ENABLE_DCAEVT1_OST + ENABLE_DCBEVT1_OST ) //����Դѡ��

//One-Shot (OST):��Ӧ���ι����������Զ��������λ��
//Cycle-by-Cycle (CBC):��Ӧ�����޷������Զ��������λ
//Output EPWMxA: TZA (highest) -> DCAEVT1(CBC) -> DCAEVT2 (lowest)
//Output EPWMxB: TZB (highest) -> DCBEVT1(CBC) -> DCBEVT2 (lowest)

//�������Ϻ�ePWM��A��ePWM-B��ǿ��Ϊ����̬
#define  TZCTL_INIT_STATE ( TZA_HI_Z     + TZB_HI_Z  +    \
                            DCAEVT1_HI_Z + DCAEVT2_HI_Z + \
                            DCBEVT1_HI_Z + DCBEVT2_HI_Z  )

/*----------------------------------------------------------------------------
//step7: �¼�����ģ������(Enent-Trigger)
----------------------------------------------------------------------------*/
//ET ģ������
// ��ADCת�����ã�����ePWM1����ʱ����SOCA����ADת��
// ��ADCת�����ã�����ePWM2����ʱ����SOCA����ADת��

// PWM�ṹ�嶨��
typedef struct {
        Uint16 PeriodMax;     // Parameter: PWM Half-Period in CPU clock cycles (Q0)
        Uint16 HalfPerMax;    // Parameter: Half of PeriodMax(Q0)
		Uint16 Duty_min;      // Parameter: ��С��������ʱ��(Q0)
		Uint16 Duty_max;      // Parameter: �����������ʱ��(Q0)
		Uint16 Deadtime;      // Parameter: ����ʱ��(Q0)

		_iq    Ualpha;        // Input: reference alpha-axis phase voltage
		_iq    Ubeta;         // Input: reference beta-axis phase voltage
        int16 PWM1out;        // output:  Duty cycle ratio (Q0)
		int16 PWM2out;        // output:  Duty cycle ratio (Q0)
		int16 PWM3out;        // output:  Duty cycle ratio (Q0)
		int16 PWM4out;        // output:  Duty cycle ratio (Q0)
		int32 Tmp_A;
		int32 Tmp_B;
} PWMGEN ;

/*------------------------------------------------------------------------------
	Default Initializers for the F280X PWMGEN Object
------------------------------------------------------------------------------*/
#define PWMGEN_DEFAULTS   {750,375,30,720,30,\
                          0,0,375,375,375,375,0,0}
/*------------------------------------------------------------------------------
	PWM Init & PWM Update Macro Definitions
------------------------------------------------------------------------------*/

//ePWMģ���ʼ��
#define PWM_INIT_MACRO(v)												\
      /***********step1��Init Timer-Base *****************/	            \
     /* Setup Sync*/													\
     EPwm1Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm4Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
                                                                        \
     /* Allow each timer to be sync'ed*/								\
     EPwm1Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm2Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm3Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm4Regs.TBCTL.bit.PHSEN = 1;								    	\
                                                                        \
     /* Init Timer-Base Period Register for EPWM1-EPWM3*/				\
     EPwm1Regs.TBPRD = v.PeriodMax;										\
     EPwm2Regs.TBPRD = v.PeriodMax;										\
     EPwm3Regs.TBPRD = v.PeriodMax;										\
     EPwm4Regs.TBPRD = v.PeriodMax;										\
                                                                        \
     /* Init Compare Register for EPWM1,EPWM2*/                         \
     EPwm1Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm2Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm3Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm4Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
                                                                                 \
    /* Compute the timer period (Q0) from the period modulation input (Q15)*/    \
     v.HalfPerMax = v.PeriodMax>>1;                                     \
     v.Duty_min = Duty_min_set;  /*��С��������ʱ��*/                        \
     v.Duty_max = v.PeriodMax - v.Duty_min;  /*�����������ʱ��*/            \
     v.Deadtime = Deadtime_set;                                         \
                                                                        \
     /* Init Timer-Base Phase Register for EPWM1-EPWM3*/				\
     EPwm1Regs.TBPHS.half.TBPHS = 0;									\
     EPwm2Regs.TBPHS.half.TBPHS = 0;									\
     EPwm3Regs.TBPHS.half.TBPHS = 0;									\
     EPwm4Regs.TBPHS.half.TBPHS = 0;									\
                                                                        \
     /* Init Timer-Base Control Register for EPWM1-EPWM3*/				\
     EPwm1Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm2Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm3Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm4Regs.TBCTL.all = PWM_INIT_STATE;								\
                                                                        \
     /* step2��Init Compare Control Register for EPWM1-EPWM3*/			\
     EPwm1Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm2Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm3Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm4Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
                                                                        \
     /* Init Action Qualifier Output A Register for EPWM1-EPWM3*/		\
     EPwm1Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm2Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm3Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm4Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
                                                                        \
     /* Init Action Qualifier Output B Register for EPWM1-EPWM3*/		\
     EPwm1Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm2Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm3Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm4Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
                                                                        \
     /*Init Dead-Band Generator Control Register for EPWM1-EPWM3*/	    \
     EPwm1Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm2Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm3Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm4Regs.DBCTL.all = DBCTL_INIT_STATE;							\
                                                                        \
     /* Init Dead-Band Generator for EPWM1-EPWM3*/						\
     EPwm1Regs.DBFED = v.Deadtime; /*����������ʱ��*/				        \
     EPwm1Regs.DBRED = v.Deadtime; /*�½�������ʱ��*/				        \
     EPwm2Regs.DBFED = v.Deadtime;								        \
     EPwm2Regs.DBRED = v.Deadtime;								        \
     EPwm3Regs.DBFED = v.Deadtime;								        \
     EPwm3Regs.DBRED = v.Deadtime;								        \
     EPwm4Regs.DBFED = v.Deadtime;								        \
     EPwm4Regs.DBRED = v.Deadtime;								        \
                                                                        \
     /*step4�� Init PWM Chopper Control Register for EPWM1-EPWM3*/		\
     EPwm1Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm2Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm3Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm4Regs.PCCTL.all = PCCTL_INIT_STATE;							\
                                                                        \
      /*******step5��Init Trip Zone*******************/                  \
     EALLOW;   /* Enable EALLOW */					                    \
     /* Init Trip Zone Select Register*/								\
     EPwm1Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm2Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm3Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm4Regs.TZSEL.all = TZSEL_INIT_STATE;							\
                                                                        \
     /* Init Trip Zone Control Register*/								\
     EPwm1Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm2Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm3Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm4Regs.TZCTL.all = TZCTL_INIT_STATE;							\
                                                                        \
     /*��TZ event trip condition*/                                       \
     EPwm1Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm2Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm3Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm4Regs.TZCLR.all = 0xFFFF;                                      \
     EDIS;    /* Disable EALLOW*/                                       \
                                                                        \
    /*******step6��Interruqt for ePWM*******************/                \
    /*PWM1�ж�����--ʹ��PWM1�����ж� CNT_zero interrupt*/                      \
     EPwm1Regs.ETSEL.bit.INTEN  = 1; /*Enable EPWM1INT generation*/     \
     EPwm1Regs.ETSEL.bit.INTSEL = 1; /*Enable  CNT_zero event*/         \
     EPwm1Regs.ETPS.bit.INTPRD  = 1; /*Generate interrupt on the 1st event*/\
     EPwm1Regs.ETCLR.bit.INT    = 1; /*Enable more interrupts*/         \
                                                                        \
    /*******step7��GPIO for ePWM*******************/                     \
     EALLOW;                       /* Enable EALLOW */                  \
     GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;    /*GPIO0 as EPWM1A*/         \
     GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;    /*GPIO2 as EPWM2A*/         \
     GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;    /*GPIO4 as EPWM3A*/         \
     GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;    /*GPIO6 as EPWM4A*/         \
     EDIS;                         /* Disable EALLOW*/

#define PWM2_INIT_MACRO(v)												\
      /***********step1��Init Timer-Base *****************/	            \
     /* Setup Sync*/													\
     EPwm5Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm6Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm7Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
     EPwm8Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			\
                                                                        \
     /* Allow each timer to be sync'ed*/								\
     EPwm5Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm6Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm7Regs.TBCTL.bit.PHSEN = 1;										\
     EPwm8Regs.TBCTL.bit.PHSEN = 1;								    	\
                                                                        \
     /* Init Timer-Base Period Register for EPWM1-EPWM3*/				\
     EPwm5Regs.TBPRD = v.PeriodMax;										\
     EPwm6Regs.TBPRD = v.PeriodMax;										\
     EPwm7Regs.TBPRD = v.PeriodMax;										\
     EPwm8Regs.TBPRD = v.PeriodMax;										\
                                                                        \
     /* Init Compare Register for EPWM1,EPWM2*/                         \
     EPwm5Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm6Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm7Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
     EPwm8Regs.CMPA.half.CMPA = v.PeriodMax>>1; /*˫����PWM������ֵ��һ���Ӧ�����ѹΪ0V*/ \
                                                                                 \
    /* Compute the timer period (Q0) from the period modulation input (Q15)*/    \
     v.HalfPerMax = v.PeriodMax>>1;                                     \
     v.Duty_min = Duty_min_set;  /*��С��������ʱ��*/                        \
     v.Duty_max = v.PeriodMax - v.Duty_min;  /*�����������ʱ��*/            \
     v.Deadtime = Deadtime_set;                                         \
                                                                        \
     /* Init Timer-Base Phase Register for EPWM1-EPWM3*/				\
     EPwm5Regs.TBPHS.half.TBPHS = 0;									\
     EPwm6Regs.TBPHS.half.TBPHS = 0;									\
     EPwm7Regs.TBPHS.half.TBPHS = 0;									\
     EPwm8Regs.TBPHS.half.TBPHS = 0;									\
                                                                        \
     /* Init Timer-Base Control Register for EPWM1-EPWM3*/				\
     EPwm5Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm6Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm7Regs.TBCTL.all = PWM_INIT_STATE;								\
     EPwm8Regs.TBCTL.all = PWM_INIT_STATE;								\
                                                                        \
     /* step2��Init Compare Control Register for EPWM1-EPWM3*/			\
     EPwm5Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm6Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm7Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
     EPwm8Regs.CMPCTL.all = CMPCTL_INIT_STATE;							\
                                                                        \
     /* Init Action Qualifier Output A Register for EPWM1-EPWM3*/		\
     EPwm5Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm6Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm7Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
     EPwm8Regs.AQCTLA.all = AQCTLA_INIT_STATE;							\
                                                                        \
     /* Init Action Qualifier Output B Register for EPWM1-EPWM3*/		\
     EPwm5Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm6Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm7Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
     EPwm8Regs.AQCTLB.all = AQCTLB_INIT_STATE;							\
                                                                        \
     /*Init Dead-Band Generator Control Register for EPWM1-EPWM3*/	    \
     EPwm5Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm6Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm7Regs.DBCTL.all = DBCTL_INIT_STATE;							\
     EPwm8Regs.DBCTL.all = DBCTL_INIT_STATE;							\
                                                                        \
     /* Init Dead-Band Generator for EPWM1-EPWM3*/						\
     EPwm5Regs.DBFED = v.Deadtime; /*����������ʱ��*/				        \
     EPwm5Regs.DBRED = v.Deadtime; /*�½�������ʱ��*/				        \
     EPwm6Regs.DBFED = v.Deadtime;								        \
     EPwm6Regs.DBRED = v.Deadtime;								        \
     EPwm7Regs.DBFED = v.Deadtime;								        \
     EPwm7Regs.DBRED = v.Deadtime;								        \
     EPwm8Regs.DBFED = v.Deadtime;								        \
     EPwm8Regs.DBRED = v.Deadtime;								        \
                                                                        \
     /*step4�� Init PWM Chopper Control Register for EPWM1-EPWM3*/		\
     EPwm5Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm6Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm7Regs.PCCTL.all = PCCTL_INIT_STATE;							\
     EPwm8Regs.PCCTL.all = PCCTL_INIT_STATE;							\
                                                                        \
      /*******step5��Init Trip Zone*******************/                  \
     EALLOW;   /* Enable EALLOW */					                    \
     /* Init Trip Zone Select Register*/								\
     EPwm5Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm6Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm7Regs.TZSEL.all = TZSEL_INIT_STATE;							\
     EPwm8Regs.TZSEL.all = TZSEL_INIT_STATE;							\
                                                                        \
     /* Init Trip Zone Control Register*/								\
     EPwm5Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm6Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm7Regs.TZCTL.all = TZCTL_INIT_STATE;							\
     EPwm8Regs.TZCTL.all = TZCTL_INIT_STATE;							\
                                                                        \
     /*��TZ event trip condition*/                                       \
     EPwm5Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm6Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm7Regs.TZCLR.all = 0xFFFF;                                      \
     EPwm8Regs.TZCLR.all = 0xFFFF;                                      \
     EDIS;    /* Disable EALLOW*/                                       \
                                                                        \
    /*******step6��Interruqt for ePWM*******************/                \
    /*PWM1�ж�����--ʹ��PWM1�����ж� CNT_zero interrupt*/                      \
                                                                        \
    /*******step7��GPIO for ePWM*******************/                     \
     EALLOW;                       /* Enable EALLOW */                  \
     GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;    /*GPIO0 as EPWM5A*/         \
     GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;    /*GPIO2 as EPWM6A*/         \
     GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 1;    /*GPIO4 as EPWM7A*/         \
     GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 1;    /*GPIO6 as EPWM8A*/         \
     EDIS;                         /* Disable EALLOW*/

//8PWM���ƣ����ñ�Ƶ�����Կ���
#define PWM1_H8_MACRO(v)												      \
    /*Compute the compare A_phase (Q0) */						          \
    v.Tmp_A = _IQmpy(v.HalfPerMax,v.Ualpha);			                  \
    v.PWM1out = v.HalfPerMax + v.Tmp_A;	                                  \
    v.PWM2out = v.HalfPerMax - v.Tmp_A;		                              \
    EPwm1Regs.CMPA.half.CMPA = _IQsat(v.PWM1out,v.Duty_max,v.Duty_min);   \
    EPwm2Regs.CMPA.half.CMPA = _IQsat(v.PWM2out,v.Duty_max,v.Duty_min);   \
                                                                          \
    /*Compute the compare  B_phase(Q0) */						          \
    v.Tmp_B = _IQmpy(v.HalfPerMax,v.Ubeta);			                      \
    v.PWM3out = v.HalfPerMax + v.Tmp_B;	                                  \
    v.PWM4out = v.HalfPerMax - v.Tmp_B;	                                  \
    EPwm3Regs.CMPA.half.CMPA = _IQsat(v.PWM3out,v.Duty_max,v.Duty_min);   \
    EPwm4Regs.CMPA.half.CMPA = _IQsat(v.PWM4out,v.Duty_max,v.Duty_min);

#define PWM2_H8_MACRO(v)												      \
    /*Compute the compare A_phase (Q0) */						          \
    v.Tmp_A = _IQmpy(v.HalfPerMax,v.Ualpha);			                  \
    v.PWM1out = v.HalfPerMax + v.Tmp_A;	                                  \
    v.PWM2out = v.HalfPerMax - v.Tmp_A;		                              \
    EPwm5Regs.CMPA.half.CMPA = _IQsat(v.PWM1out,v.Duty_max,v.Duty_min);   \
    EPwm6Regs.CMPA.half.CMPA = _IQsat(v.PWM2out,v.Duty_max,v.Duty_min);   \
                                                                          \
    /*Compute the compare  B_phase(Q0) */						          \
    v.Tmp_B = _IQmpy(v.HalfPerMax,v.Ubeta);			                      \
    v.PWM3out = v.HalfPerMax + v.Tmp_B;	                                  \
    v.PWM4out = v.HalfPerMax - v.Tmp_B;	                                  \
    EPwm7Regs.CMPA.half.CMPA = _IQsat(v.PWM3out,v.Duty_max,v.Duty_min);   \
    EPwm8Regs.CMPA.half.CMPA = _IQsat(v.PWM4out,v.Duty_max,v.Duty_min);

#define PWM1_ON_MACRO()               \
   EALLOW;                           \
   EPwm1Regs.TZCLR.bit.OST=1;        \
   EPwm2Regs.TZCLR.bit.OST=1;        \
   EPwm3Regs.TZCLR.bit.OST=1;        \
   EPwm4Regs.TZCLR.bit.OST=1;        \
   EDIS;                             \
   Drv8412_ENA;                      \

#define PWM2_ON_MACRO()               \
   EALLOW;                           \
   EPwm5Regs.TZCLR.bit.OST=1;        \
   EPwm6Regs.TZCLR.bit.OST=1;        \
   EPwm7Regs.TZCLR.bit.OST=1;        \
   EPwm8Regs.TZCLR.bit.OST=1;        \
   EDIS;                             \
   Drv8412_ENA_B;                      \

#define PWM1_OFF_MACRO()              \
   Drv8412_RESET;                    \
  /*ǿ��һ���׷���������¼�������λ��־λ*/     \
   EALLOW;                           \
   EPwm1Regs.TZFRC.bit.OST=1;        \
   EPwm2Regs.TZFRC.bit.OST=1;        \
   EPwm3Regs.TZFRC.bit.OST=1;        \
   EPwm4Regs.TZFRC.bit.OST=1;        \
   EDIS;                             \

#define PWM2_OFF_MACRO()             \
   Drv8412_RESET_B;                    \
  /*ǿ��һ���׷���������¼�������λ��־λ*/       \
   EALLOW;                           \
   EPwm5Regs.TZFRC.bit.OST=1;        \
   EPwm6Regs.TZFRC.bit.OST=1;        \
   EPwm7Regs.TZFRC.bit.OST=1;        \
   EPwm8Regs.TZFRC.bit.OST=1;        \
   EDIS;                             \

#endif  // __F280X_PWM_H__
