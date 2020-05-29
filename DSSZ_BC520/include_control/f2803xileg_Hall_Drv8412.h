/* ==================================================================================
File name:       F2803XILEG_Hall_Stepper.H
Description: This header file contains macro definition for ADC initilization 
Target: TMS320F2803x family
History: 07-28-2015
Version 1.0

//(1) 第一次采样结果可能不正确,应discarding ADCRESULT0
//(2) 采样温度时采样时间应大于550ns（35+1+6）=30*16.67ns=700ns
----------------------------------------------------------------------------*/

#ifndef __F280XILEG_VDC_H__
#define __F280XILEG_VDC_H__

//#include "f2803xbmsk.h"

/*------------------------------------------------------------------------------
 ADC Initialization Macro Definition 
------------------------------------------------------------------------------*/
//#define CPU_CLOCK_SPEED      16.667L     //16.667L   for a 60MHz CPU clock speed
#define ADC_usDELAY 10000L

//#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_CLOCK_SPEED) - 9.0L) / 5.0L)
//extern void DSP28x_usDelay(unsigned long Count);

//DSP温度计算用参数
//Slope of temperature sensor (deg. C / ADC code, fixed pt Q15 format)
#define getTempSlope()  (*(int (*)(void))0x3D7E82)()
//ADC code corresponding to temperature sensor output at 0-degreesC
#define getTempOffset() (*(int (*)(void))0x3D7E85)()


//for 28030/28031 Sample Window: 24<=ACQPS<=64
//for 28032~28035 Sample Window:  7<=ACQPS<=64
#define ACQPS_Set  40   //采样窗口宽度

//1.65V--数字量32768--(1.65±0.213V)
#define  OFFSET_MIN  28000  /*ADC基准最小值 */
#define  OFFSET_MAX  37000  /*ADC基准最大值 */

/*-----------------------------------------------------------------------------
	Define the structure of the ADC Driver Object
-----------------------------------------------------------------------------*/
typedef struct {
        Uint32 ADC_OFFSETA;//A相电流采样偏置值
        Uint32 ADC_OFFSETB;//B相电流采样偏置值

        Uint16 ADC_OFFSET_MIN; //ADC基准最小值
        Uint16 ADC_OFFSET_MAX; //ADC基准最大值
        Uint16 ADC_OFFSETA_ERR;//A相电流采样偏置值错误
        Uint16 ADC_OFFSETB_ERR;//B相电流采样偏置值错误

        int32 Ia;    //输出：A相电流采样值
        int32 Ib;    //输出：B相电流采样值
        int32 Udc;   //输出：母线电压采样值

        int32 DSP_Temp;      //输出 摄氏温度：oC
        int16 Temp_senor;  //Result0,1--DSP内部结温传感器
        int16 TempSensorOffset; //温度计算偏置值
        int32 TempSensorSlope; //温度计算斜率
        Uint16 i_ADC;   //初始采样基准时，采样次数
        Uint16 ADC_Index;   //初始采样基准时，采样次数
} ADConvter;

#define ADC_DEFAULTS  {0x0,0x0,\
                       0x0,0x0,0x0,\
                       0x0,0x0,0,0,\
                       25,1932,0x0,0x0,0x0,0}

//ADC模块初始化
#define ADC_INIT_MACRO()																		\
                                                                                                \
	EALLOW;																						\
    DELAY_US(ADC_usDELAY);																		\
    AdcRegs.ADCCTL1.all = 1;    /*Resets entire ADC module */   								\
	asm(" NOP ");																				\
	asm(" NOP ");    																			\
																								\
	AdcRegs.ADCCTL1.bit.ADCBGPWD   = 1;	/* Power up band gap */								    \
	DELAY_US(ADC_usDELAY);			 /* Delay before powering up rest of ADC */			        \
	AdcRegs.ADCCTL1.bit.ADCREFSEL  = 0;	/*Internal Bandgap used for reference generation*/	    \
   	AdcRegs.ADCCTL1.bit.ADCREFPWD  = 1;	/* Power up reference */							    \
   	AdcRegs.ADCCTL1.bit.ADCPWDN    = 1;	/* Power up rest of ADC */							    \
	AdcRegs.ADCCTL1.bit.ADCENABLE  = 1;	/* Enable ADC */									    \
																								\
	asm(" RPT#100 || NOP");																		\
																								\
	AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;	                                                    \
	AdcRegs.ADCCTL1.bit.TEMPCONV = 0;	/*"1"使能内部温度传感器--对应模拟输入通道ADCINA5*/	                \
																								\
	/*为了提高ADC采用精度*/																			\
	AdcRegs.ADCCTL2.bit.CLKDIV2EN     = 1;  /*ADCCLK=SYSCLK/2=30MHz*/                           \
	AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;	/*1:Enable non-overlap mode*/                       \
	DELAY_US(ADC_usDELAY);																		\
																								\
    /***AD采样通道设置(DSP28030单通道采样总时间：25(采样保持时间)+1+13(AD转换时间)=39*16.67ns=650ns*****/	        \
    /*PWM1下溢中断触发AD采样*/                                                                       \
    /*采样通道选择CHSEL = A0(0),A1(1),A2(2),A3(3),A4(4),A5(5),A6(6),A7(7);*/                        \
    /*采样通道选择CHSEL = B0(8),B1(9),B2(10),B3(11),B4(12),B5(13),B6(14),B7(15)*/                   \
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN0 = 0; /* Single sample mode set for SOC0 and SOC1*/       \
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN2 = 0; /* Single sample mode set for SOC2 and SOC3*/       \
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN4 = 0; /* Single sample mode set for SOC4 and SOC5*/       \
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN6 = 0; /* Single sample mode set for SOC6 and SOC7*/       \
   /* Other invalid selections(无效的采样保持时间选择): */                                              \
   /* 10h(16), 11h(17), 12h(18), 13h(19), 14h(20), */                                           \
   /* 1Dh(29), 1Eh(30), 1Fh(31), 20h(32), 21h(33), */                                           \
   /* 2Ah(42), 2Bh(43), 2Ch(44), 2Dh(45), 2Eh(46), */                                           \
   /* 37h(55), 38h(56), 39h(57), 3Ah(58), 3Bh(59), */                                           \
                                                                                                \
     /*AD采样结果寄存器Resultx变换与SOCx对应  */                                                          \
    AdcRegs.ADCSOC0CTL.bit.CHSEL   = 0;  /* A0通道 第一次采样结果可能不正确,A0--Ia1 */                     \
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
    AdcRegs.ADCSOC0CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
                                                                                                \
    AdcRegs.ADCSOC1CTL.bit.CHSEL   = 1;  /* A1通道 第一次采样结果可能不正确,A1--Ia2 */                     \
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
    AdcRegs.ADCSOC1CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
                                                                                                \
    AdcRegs.ADCSOC2CTL.bit.CHSEL   = 2;  /*A2--I_Air*/                                          \
    AdcRegs.ADCSOC2CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
    AdcRegs.ADCSOC2CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
                                                                                                \
    AdcRegs.ADCSOC3CTL.bit.CHSEL   = 8;  /*B0--Ib1*/                                            \
    AdcRegs.ADCSOC3CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
    AdcRegs.ADCSOC3CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
                                                                                                \
	AdcRegs.ADCSOC4CTL.bit.CHSEL   = 9;  /*B1--Ib2*/                                            \
	AdcRegs.ADCSOC4CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
	AdcRegs.ADCSOC4CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
	                                                                                            \
	AdcRegs.ADCSOC5CTL.bit.CHSEL   = 10;  /*B2--3.3V*/                                          \
	AdcRegs.ADCSOC5CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
	AdcRegs.ADCSOC5CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
																							    \
	AdcRegs.ADCSOC6CTL.bit.CHSEL   = 11;  /*B3--DC_BUS*/                                        \
	AdcRegs.ADCSOC6CTL.bit.TRIGSEL = 5;  /* Set SOC0 start trigger on EPWM1-SOCA */             \
	AdcRegs.ADCSOC6CTL.bit.ACQPS   = ACQPS_Set; /* 采样温度时采样保持时间应大于550ns(35*16.7=584ns)*/     \
	EDIS;																						\
	                                                                                            \
	/*ePWM1下溢事件触发寄存器设置*/                                                                     \
   /* Set up Event Trigger with CNT_zero enable for Time-base of EPWM1 */	     	            \
    EPwm1Regs.ETSEL.bit.SOCAEN  = 1;    /* Enable SOCA(ePWM1触发AD采样 ）*/						    \
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;    /* Enable CNT_zero event for SOCA 下溢中断触发*/		        \
    EPwm1Regs.ETPS.bit.SOCAPRD  = 1;    /* Generate SOCA on the 1st event */					\
	EPwm1Regs.ETCLR.bit.SOCA    = 1;    /* Clear SOCA flag */								    \
    DELAY_US(ADC_usDELAY);                                                                      \
    EPwm1Regs.ETSEL.bit.SOCBEN  = 1;    /* Enable SOCB(ePWM1触发AD采样 ）*/						    \
    EPwm1Regs.ETSEL.bit.SOCBSEL = 1;    /* Enable CNT_zero event for SOCA 下溢中断触发*/		        \
    EPwm1Regs.ETPS.bit.SOCBPRD  = 1;    /* Generate SOCA on the 1st event */					\
	EPwm1Regs.ETCLR.bit.SOCB    = 1;    /* Clear SOCA flag */								    \
    DELAY_US(ADC_usDELAY);                                                                      \


#define ADC_OFFSET_Reset_MACRO(v)                   \
for(v.i_ADC=0;v.i_ADC<64;v.i_ADC++)                 \
{                                                   \
   DELAY_US(2000);                                  \
   AdcRegs.ADCSOCFRC1.bit.SOC0 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC1 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC2 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC3 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC4 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC5 = 1;/*软件启动AD转换*/   \
   AdcRegs.ADCSOCFRC1.bit.SOC6 = 1;/*软件启动AD转换*/   \
   while(AdcRegs.ADCCTL1.bit.ADCBSY==1) {}          \
   DELAY_US(2000);                                  \
}                                                   \


//AD初始电压校正
#define ADC_OFFSET_INIT_MACRO(v)                    \
for(v.i_ADC=0;v.i_ADC<64;v.i_ADC++)                 \
{                                                   \
   DELAY_US(2000);                                  \
   if(v.ADC_Index==1)                               \
   {                                                \
	   AdcRegs.ADCSOCFRC1.bit.SOC0 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC1 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC2 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC3 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC4 = 1;/*软件启动AD转换*/   \
	   while(AdcRegs.ADCCTL1.bit.ADCBSY==1) {}          \
	   v.ADC_OFFSETA  += AdcResult.ADCRESULT0;          \
	   v.ADC_OFFSETB  += AdcResult.ADCRESULT3;          \
   }                                                    \
   else if(v.ADC_Index==2)                              \
   {                                                    \
	   AdcRegs.ADCSOCFRC1.bit.SOC0 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC1 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC2 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC3 = 1;/*软件启动AD转换*/   \
	   AdcRegs.ADCSOCFRC1.bit.SOC4 = 1;/*软件启动AD转换*/   \
	   while(AdcRegs.ADCCTL1.bit.ADCBSY==1) {}          \
	   v.ADC_OFFSETA  += AdcResult.ADCRESULT1;          \
	   v.ADC_OFFSETB  += AdcResult.ADCRESULT4;          \
   }                                                    \
}                                                   \
v.TempSensorSlope  = getTempSlope(); /*IQ15*/       \
v.TempSensorOffset = getTempOffset();               \
v.ADC_OFFSET_MIN = OFFSET_MIN;   /*ADC基准最小值 */     \
v.ADC_OFFSET_MAX = OFFSET_MAX;   /*ADC基准最大值 */     \
v.ADC_OFFSETA = (v.ADC_OFFSETA>>2);/*Q12*Q6(2^6=64)>>3=Q15*/  \
v.ADC_OFFSETB = (v.ADC_OFFSETB>>2);/*电流A,B两相采样存在偏差*/      \


/****ADC电压基准故障检测*******/
#define ADC_OFFSET_CHEKE_MACRO(v)                                             \
 if( (v.ADC_OFFSETA<=v.ADC_OFFSET_MIN) || (v.ADC_OFFSETA>=v.ADC_OFFSET_MAX) ) \
 {v.ADC_OFFSETA_ERR=1;}                                                       \
 if( (v.ADC_OFFSETB<=v.ADC_OFFSET_MIN) || (v.ADC_OFFSETB>=v.ADC_OFFSET_MAX) ) \
 {v.ADC_OFFSETB_ERR=1;}                                                       \


//读入AD采样结果
#define ADC1_READ_MACRO(v)                                         \
while(AdcRegs.ADCCTL1.bit.ADCBSY==1) {}                            \
v.Ia = ((AdcResult.ADCRESULT0<<4) - v.ADC_OFFSETA); /*A相电流采样值*/  \
v.Ib = ((AdcResult.ADCRESULT3<<4) - v.ADC_OFFSETB); /*B相电流采样值*/  \

#define ADC2_READ_MACRO(v)                                         \
while(AdcRegs.ADCCTL1.bit.ADCBSY==1) {}                            \
v.Ia = ((AdcResult.ADCRESULT1<<4) - v.ADC_OFFSETA); /*A相电流采样值*/  \
v.Ib = ((AdcResult.ADCRESULT4<<4) - v.ADC_OFFSETB); /*B相电流采样值*/  \

#endif // __F280XILEG_VDC_H__

//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
