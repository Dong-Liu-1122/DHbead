//###########################################################################
//
// FILE:   DSP2803x_ECap.c
//
// TITLE:  DSP2803x ECAP Initialization & Support Functions.
//
//###########################################################################
// $TI Release: F2803x C/C++ Header Files and Peripheral Examples V126 $
// $Release Date: November 30, 2011 $
//###########################################################################

#include "F2806x_Device.h"     // DSP2803x Headerfile Include File
#include "Stepper-Settings.h"
#include "f2803xbmsk.h"
#include "f2803xdrvlib.h"


#if(ECAP_Ena)
/*-----------------------------------------------------------------------------
    Initialization states for ECAP Control Registers 1 and 2 for ECAP1
------------------------------------------------------------------------------*/

#define ECCTL1_INIT_STATE   ( CAP1POL_RISING_EDGE +  \
                              CAPLDEN_ENABLE + \
                              CTRRST1_DIFFERENCE_TS + \
                              EVTFLTPS_X_1 + \
                              EMULATION_FREE  )

//#define CAP1_ECCTL1_INIT_STATE (
//bit0---CAP1POL_RISING_EDGE + \捕获事件1信号的上升沿触发
//bit1---CTRRST1_DIFFERENCE_TS + \发生捕获事件1时计数器复位
//bit2~3对应CAP2
//bit4~5对应CAP3
//bit6~7对应CAP4
//bit8---CAPLDEN_ENABLE + \使能捕获事件发生时间装载到捕获寄存器CAP1
//bit13~9---EVTFLTPS_X_1（0x0000） + \事件过滤器不分频
//bit15~14---EMULATION_FREE )时间标记计数器（TSCTR）不受仿真挂起影响


#define ECCTL2_INIT_STATE   ( CONTINUOUS_MODE + \
                              TSCNTSTP_FREE + \
                              SYNCI_DISABLE + \
                              SYNCO_DISABLE + \
                              CAPTURE_MODE )

/*
#define ECCTL2_INIT_STATE  ( CONTINUOUS_MODE + \
                            ONESHT_CAP_EV1 + \
                            TSCNTSTP_FREE + \
                            SYNCI_DISABLE + \
                            SYNCO_DISABLE + \
                            CAPTURE_MODE )
*/

//#define CAP1_ECCTL2_INIT_STATE (
//bit0---CONTINUOUS_MODE + \连续捕获模式
//bit2~1--- ONESHT_CAP_EV1 + \捕获1事件后覆盖前面的值
//bit4---TSCNTSTP_FREE + \时间标记计数器自由运行
//bit5---SYNCI_DISABLE + \禁止时间标记计数器（TSCTR）同步输入
//bit7~6---SYNCO_DISABLE + \禁止同步输出
//bit9---CAPTURE_MODE )eCAP工作在捕获模式
//bit10---0



interrupt void CapISR(void);

void InitECap(void)
{
     /*******step0:使能ECAP时钟********/
     EALLOW;
     SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1; // ECAP1--APL
     EDIS;

    /*******step1:eCAP管脚设置********/
     EALLOW;     /* Enable EALLOW */

     //GpioCtrlRegs.GPAPUD.bit.GPIO15   = 0;  /*使能上拉电阻*/
     //GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 2;  /*Qualification using 6 samples*/
     //GpioCtrlRegs.GPAMUX1.bit.GPIO15  = 1;  /* GPIO15 is ECAP2 */

     GpioCtrlRegs.GPAPUD.bit.GPIO11   = 0;  /*使能上拉电阻*/
     GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 2;  /*Qualification using 6 samples*/
     GpioCtrlRegs.GPAMUX1.bit.GPIO11  = 3;  // 0=GPIO,  1=ECAP1,  2=Resv,  3=SPISIMO-B

     EDIS;      /* Disable EALLOW */

    /*******step2: ECAP设置********/
    /* Init ECAP Control Registers 1 and 2 for ECAP1*/
//    ECap2Regs.ECCTL1.all = ECCTL1_INIT_STATE;
//    ECap2Regs.ECCTL2.all = ECCTL2_INIT_STATE;
//    ECap2Regs.ECEINT.bit.CEVT1 = 1;/*使能捕获1中断*/
//    ECap2Regs.ECCLR.all = 0xFFFF;  /*清中断标志*/

     ECap1Regs.ECCTL1.all = ECCTL1_INIT_STATE;
     ECap1Regs.ECCTL2.all = ECCTL2_INIT_STATE;
     ECap1Regs.ECEINT.bit.CEVT1 = 1;/*使能捕获1中断*/
     ECap1Regs.ECCLR.all = 0xFFFF;  /*清中断标志*/

    /********step3:ECAP中断设置**************/
    EALLOW;
    PieVectTable.ECAP1_INT = &CapISR;
    EDIS;
    PieCtrlRegs.PIEIER4.all |= M_INT1; //对应PIE4.1--CAPISR
    IER |= M_INT4;   //Enable CPU INT4 for CAP1_INT(INT4.1)
}

#endif
//===========================================================================
// End of file.
//===========================================================================
