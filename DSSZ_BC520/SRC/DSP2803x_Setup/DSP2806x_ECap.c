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
//bit0---CAP1POL_RISING_EDGE + \�����¼�1�źŵ������ش���
//bit1---CTRRST1_DIFFERENCE_TS + \���������¼�1ʱ��������λ
//bit2~3��ӦCAP2
//bit4~5��ӦCAP3
//bit6~7��ӦCAP4
//bit8---CAPLDEN_ENABLE + \ʹ�ܲ����¼�����ʱ��װ�ص�����Ĵ���CAP1
//bit13~9---EVTFLTPS_X_1��0x0000�� + \�¼�����������Ƶ
//bit15~14---EMULATION_FREE )ʱ���Ǽ�������TSCTR�����ܷ������Ӱ��


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
//bit0---CONTINUOUS_MODE + \��������ģʽ
//bit2~1--- ONESHT_CAP_EV1 + \����1�¼��󸲸�ǰ���ֵ
//bit4---TSCNTSTP_FREE + \ʱ���Ǽ�������������
//bit5---SYNCI_DISABLE + \��ֹʱ���Ǽ�������TSCTR��ͬ������
//bit7~6---SYNCO_DISABLE + \��ֹͬ�����
//bit9---CAPTURE_MODE )eCAP�����ڲ���ģʽ
//bit10---0



interrupt void CapISR(void);

void InitECap(void)
{
     /*******step0:ʹ��ECAPʱ��********/
     EALLOW;
     SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1; // ECAP1--APL
     EDIS;

    /*******step1:eCAP�ܽ�����********/
     EALLOW;     /* Enable EALLOW */

     //GpioCtrlRegs.GPAPUD.bit.GPIO15   = 0;  /*ʹ����������*/
     //GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 2;  /*Qualification using 6 samples*/
     //GpioCtrlRegs.GPAMUX1.bit.GPIO15  = 1;  /* GPIO15 is ECAP2 */

     GpioCtrlRegs.GPAPUD.bit.GPIO11   = 0;  /*ʹ����������*/
     GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 2;  /*Qualification using 6 samples*/
     GpioCtrlRegs.GPAMUX1.bit.GPIO11  = 3;  // 0=GPIO,  1=ECAP1,  2=Resv,  3=SPISIMO-B

     EDIS;      /* Disable EALLOW */

    /*******step2: ECAP����********/
    /* Init ECAP Control Registers 1 and 2 for ECAP1*/
//    ECap2Regs.ECCTL1.all = ECCTL1_INIT_STATE;
//    ECap2Regs.ECCTL2.all = ECCTL2_INIT_STATE;
//    ECap2Regs.ECEINT.bit.CEVT1 = 1;/*ʹ�ܲ���1�ж�*/
//    ECap2Regs.ECCLR.all = 0xFFFF;  /*���жϱ�־*/

     ECap1Regs.ECCTL1.all = ECCTL1_INIT_STATE;
     ECap1Regs.ECCTL2.all = ECCTL2_INIT_STATE;
     ECap1Regs.ECEINT.bit.CEVT1 = 1;/*ʹ�ܲ���1�ж�*/
     ECap1Regs.ECCLR.all = 0xFFFF;  /*���жϱ�־*/

    /********step3:ECAP�ж�����**************/
    EALLOW;
    PieVectTable.ECAP1_INT = &CapISR;
    EDIS;
    PieCtrlRegs.PIEIER4.all |= M_INT1; //��ӦPIE4.1--CAPISR
    IER |= M_INT4;   //Enable CPU INT4 for CAP1_INT(INT4.1)
}

#endif
//===========================================================================
// End of file.
//===========================================================================
