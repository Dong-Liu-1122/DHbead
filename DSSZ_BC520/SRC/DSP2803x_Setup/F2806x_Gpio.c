//###########################################################################
//
// FILE:	F2806x_Gpio.c
//
// TITLE:	F2806x General Purpose I/O Initialization & Support Functions.
//
//###########################################################################
// $TI Release: F2806x C/C++ Header Files and Peripheral Examples V150 $
// $Release Date: June 16, 2015 $
// $Copyright: Copyright (C) 2011-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "Stepper-Settings.h"    //参数设置头文件

//---------------------------------------------------------------------------
// InitGpio:
//---------------------------------------------------------------------------
// This function initializes the Gpio to a known (default) state.
//
// For more details on configuring GPIO's as peripheral functions,
// refer to the individual peripheral examples and/or GPIO setup example.
void InitGpio(void)
{

    EALLOW; // below registers are "protected", allow access.

    //  GPIO-00 - PIN FUNCTION = PWM_1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO0 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-01 - PIN FUNCTION = PWM_1B
    GpioCtrlRegs.GPAMUX1.bit.GPIO1= 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO1 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-02 - PIN FUNCTION = PWM_2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO2 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-03 - PIN FUNCTION = Drv8412_OTW1
    //GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 2;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO3 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-04 - PIN FUNCTION = PWM_3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO4 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-05 - PIN FUNCTION =  Drv8412_FLT--input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO5 = 2;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 0;      // 1=OUTput,  0=INput
    //GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO5 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-06 - PIN FUNCTION == PWM_4A
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO6 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-07 - PIN FUNCTION = Drv8412_Reset
    // GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 2;   /* GPIO11--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;    // 0=GPIO,  1=EPWM6B,  2=LINRX-A,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO7  = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO7 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-08 - PIN FUNCTION = ----input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 2;    /* GPIO9--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;     // 0=GPIO,  1=EPWM5A,  2=Resv,  3=ADCSOC-A
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;      // 1=OUTput,  0=INput
    //  GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO8 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-09 - PIN FUNCTION =  ----input
    //GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 2;    /* GPIO9--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;     // 0=GPIO,  1=EPWM5B,  2=LINTX-A,  3=Resv
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;    // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO9 = 1;      // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-10 - PIN FUNCTION =-KEY3--input

    GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 2;   /* GPIO10--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;    // 0=GPIO,  1=EPWM6A,  2=Resv,  3=ADCSOC-B
    GpioCtrlRegs.GPADIR.bit.GPIO10  = 0;    // 1=OUTput,  0=INput
    //  GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO10 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-11 - PIN FUNCTION = --8412_OTW2
    //GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 2;   /* GPIO11--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 3;    // 0=GPIO,  1=EPWM6B,  2=LINRX-A,  3=Resv
    //GpioCtrlRegs.GPADIR.bit.GPIO11  = 0;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO11 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-12 - PIN FUNCTION =--DIP1--input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 2;   /* GPIO11--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;    // 0=GPIO,  1=TZ1,  2=SCITX-A,  3=SPISIMO-B
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO12 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO12 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-13 - PIN FUNCTION = ---DIP2--
    GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 2;
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;    // 0=GPIO,  1=TZ2,  2=Resv,  3=SPISOMI-B
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 0;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO13 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-14 - PIN FUNCTION = ---Spare---(未使用）
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;    // 0=GPIO,  1=TZ3,  2=LINTX-A,  3=SPICLK-B
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO14 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO14 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-15 - PIN FUNCTION = --CAP
    // GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0; // Enable pull-up on GPIO15 (CAP2)
    // GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0;   // Synch to SYSCLKOUT GPIO15 (CAP2)
    //GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 1;    // Configure GPIO15 as CAP2
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;    // 0=GPIO,  1=CAP2,  2=LINRX-A,  3=SPISTE-B
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO15 = 1;   // uncomment if --> Set Low initially
    //GpioDataRegs.GPASET.bit.GPIO15 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //  GPIO-16 - PIN FUNCTION =   --
    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 2;   /* GPIO16--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0;    // 0=GPIO,  1=SPISIMO-A,  2=Resv,  3=TZ2
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 0;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO16 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-17 - PIN FUNCTION =
    //GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 2;
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 0;    // 0=GPIO,  1=SPISOMI-A,  2=Resv,  3=TZ3
    GpioCtrlRegs.GPADIR.bit.GPIO17 =  1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO17 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO17 = 1;     // 初始时为"1",灯不亮
    //--------------------------------------------------------------------------------------
    //  GPIO-18 - PIN FUNCTION = --
    //GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 2;  /* GPIO18--Qualification using 6 samples*/
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 0;    // 0=GPIO,  1=SPICLK-A,  2=LINTX-A,  3=XCLKOUT
    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;   // 初始时为"0",灯亮
    //  GpioDataRegs.GPASET.bit.GPIO18 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-19 - PIN FUNCTION =
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;    // 0=GPIO,  1=SPISTE-A,  2=LINRX-A,  3=ECAP1
    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO19 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO19 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-20 - PIN FUNCTION = Encoder1A--Motor编码器A
    GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 2;
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;    // 0=GPIO,  1=EQEPA-1,  2=Resv,  3=COMP1OUT
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 0;     // 1=OUTput,  0=INput
    //--------------------------------------------------------------------------------------
    //  GPIO-21 - PIN FUNCTION = Encoder1B--Motor编码器B
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;    // 0=GPIO,  1=EQEPB-1,  2=Resv,  3=COMP2OUT
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPASET.bit.GPIO21 = 1;
    //--------------------------------------------------------------------------------------
    //  GPIO-22 - PIN FUNCTION = ----(onewire使用)
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;    // 0=GPIO,  1=EQEPS-1,  2=Resv,  3=LINTX-A
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO22 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO22 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-23 - PIN FUNCTION = Encoder1C--编码器Z
    GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 2;
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;    // 0=GPIO,  1=EQEPS-1,  2=Resv,  3=LINTX-A
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 0;     // 1=OUTput,  0=INput
    //--------------------------------------------------------------------------------------
    //  GPIO-24 - PIN FUNCTION = --eCAP1--AB_Pusle
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;    // 0=GPIO,  1=ECAP1,  2=Resv,  3=SPISIMO-B
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO24 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-25 - PIN FUNCTION = --Spare--（未使用）
    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=SPISOMI-B
    GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO25 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO25 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-26 - PIN FUNCTION = --Spare--（未使用）
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=SPICLK-B
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO26 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO26 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-27 - PIN FUNCTION = --Spare--（未使用）
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=SPISTE-B
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO27 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPASET.bit.GPIO27 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-28 - PIN FUNCTION = SCI
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;    // 0=GPIO,  1=SPICLK-A,  2=LINTX-A,  3=XCLKOUT
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPASET.bit.GPIO28 = 1;   // 初始时为"0",灯亮
    //  GpioDataRegs.GPASET.bit.GPIO28 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-29 - PIN FUNCTION = SCI
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0;    // 0=GPIO,  1=SPICLK-A,  2=LINTX-A,  3=XCLKOUT
    GpioCtrlRegs.GPADIR.bit.GPIO29 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPACLEAR.bit.GPIO29 = 0;   // 初始时为"0",灯亮
    //  GpioDataRegs.GPASET.bit.GPIO29 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-30 - PIN FUNCTION = --CANRX-A （CAN接收）
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;    // 0=GPIO,  1=CANRX-A,  2=Resv,  3=Resv
    //--------------------------------------------------------------------------------------
    //  GPIO-31 - PIN FUNCTION = --CANTX-A （CAN发送）
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;    // 0=GPIO,  1=CANTX-A,  2=Resv,  3=Resv
    //--------------------------------------------------------------------------------------
    //  GPIO-32 - PIN FUNCTION = IIC数据--I2CSDA-A  DSP28x_usDelay
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;    // 0=GPIO,  1=I2CSDA-A,  2=SYNCI,  3=ADCSOCA

    //--------------------------------------------------------------------------------------
    //  GPIO-33 - PIN FUNCTION = IIC时钟--I2CSCL-A
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;    // 0=GPIO,  1=I2CSCL-A,  2=SYNCO,  3=ADCSOCB

    //--------------------------------------------------------------------------------------
    //  GPIO-34 - PIN FUNCTION = ALARM_LED
    GpioCtrlRegs.GPBQSEL1.bit.GPIO34 = 2;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 0;     // 1=OUTput,  0=INput
    //GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;   // uncomment if --> Set Low initially
    //GpioDataRegs.GPBSET.bit.GPIO34 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO 35-38 are defaulted to JTAG usage, and are not shown here to enforce JTAG debug usage.
    //--------------------------------------------------------------------------------------
    //  GPIO-39 - PIN FUNCTION = --8412_Fault--（未使用）

    //GpioCtrlRegs.GPBQSEL1.bit.GPIO39 = 2;
    GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;     // 0=GPIO,  1=EPWM1A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;      // 1=OUTput,  0=INput
    GpioDataRegs.GPBSET.bit.GPIO39 = 1;   // uncomment if --> Set Low initially

    //  GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPBSET.bit.GPIO39 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-40 - PIN FUNCTION = --Spare--（未使用）
    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;    // 0=GPIO,  1=EPWM7A,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPBCLEAR.bit.GPIO40 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPBSET.bit.GPIO40 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-41 - PIN FUNCTION = --Spare--（未使用）

    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;    // 0=GPIO,  1=EPWM6B,  2=LINRX-A,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO41  = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;      // uncomment if --> Set High initially
    //  GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPBSET.bit.GPIO41 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-42 - PIN FUNCTION = --Spare--（未使用）
    GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=COMP1OUT
    GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPBCLEAR.bit.GPIO42 = 1;   // uncomment if --> Set Low initially
    //  GpioDataRegs.GPBSET.bit.GPIO42 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-43 - PIN FUNCTION = --LED_GREEN_OUT
    GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=COMP2OUT
    GpioCtrlRegs.GPBDIR.bit.GPIO43 = 1;     // 1=OUTput,  0=INput
    //  GpioDataRegs.GPBCLEAR.bit.GPIO43 = 1;   // uncomment if --> Set Low initially
    GpioDataRegs.GPBCLEAR.bit.GPIO43 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-44 - PIN FUNCTION = --LED_RED_OUT
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPBCLEAR.bit.GPIO44 = 1;   // uncomment if --> Set Low initially
    //GpioDataRegs.GPBSET.bit.GPIO44 = 1;     // uncomment if --> Set High initially
    //--------------------------------------------------------------------------------------
    //  GPIO-50 - PIN FUNCTION = --Head_RED--input
    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 1;    // 0=GPIO,  1=Resv,  2=Resv,  3=COMP2OUT
    //GpioCtrlRegs.GPBDIR.bit.GPIO50 = 0;     // 1=OUTput,  0=INput
    //GpioCtrlRegs.GPBQSEL2.bit.GPIO50 = 2;
    //--------------------------------------------------------------------------------------
    //  GPIO-51 - PIN FUNCTION = --Head_GREEN--input
    GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 1;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    //GpioCtrlRegs.GPBDIR.bit.GPIO51 = 0;     // 1=OUTput,  0=INput
    //GpioCtrlRegs.GPBQSEL2.bit.GPIO51 = 2;
    //--------------------------------------------------------------------------------------
    GpioCtrlRegs.GPBMUX2.bit.GPIO52 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPBSET.bit.GPIO52 = 1;   // uncomment if --> Set Low initiall

    GpioCtrlRegs.GPBMUX2.bit.GPIO53 = 1;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv

    //  GPIO-20 - PIN FUNCTION = Encoder1A--主轴编码器A
    GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 2;    // 0=GPIO,  1=EQEPA-1,  2=Resv,  3=COMP1OUT
    //--------------------------------------------------------------------------------------
    //  GPIO-21 - PIN FUNCTION = Encoder1B--主轴编码器B
    GpioCtrlRegs.GPBMUX2.bit.GPIO55 = 2;    // 0=GPIO,  1=EQEPB-1,  2=Resv,  3=COMP2OUT
    GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 2;

    GpioCtrlRegs.GPBMUX2.bit.GPIO57 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO57 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPBSET.bit.GPIO57 = 1;   // uncomment if --> Set Low initially

    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=Resv
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;     // 1=OUTput,  0=INput
    GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1;   // uncomment if --> Set Low initially

    GpioCtrlRegs.AIOMUX1.bit.AIO4 = 0;
    GpioCtrlRegs.AIODIR.bit.AIO4 =  1;
    GpioDataRegs.AIOCLEAR.bit.AIO4 = 1;

    GpioCtrlRegs.AIOMUX1.bit.AIO6 = 0;
    GpioCtrlRegs.AIODIR.bit.AIO6 =  1;
    GpioDataRegs.AIOCLEAR.bit.AIO6 = 1;

    GpioCtrlRegs.AIOMUX1.bit.AIO12 = 0;
    GpioCtrlRegs.AIODIR.bit.AIO12 =  1;
    GpioDataRegs.AIOCLEAR.bit.AIO12 = 1;

    GpioCtrlRegs.AIOMUX1.bit.AIO14 = 0;
    GpioCtrlRegs.AIODIR.bit.AIO14 =  1;
    GpioDataRegs.AIOCLEAR.bit.AIO14 = 1;

    EDIS;   // Disable register access
}

//===========================================================================
// End of file.
//===========================================================================
