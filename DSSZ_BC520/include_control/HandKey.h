/*
 * HandKey.h
 *
 *  Created on: 2016年8月17日
 *      Author: liudong
 */
//#include "Parameter.h"

#ifndef HandKey_H_
#define HandKey_H_

#define Air1_Valve_Set GpioDataRegs.GPASET.bit.GPIO21
#define Air1_Valve_Clc GpioDataRegs.GPACLEAR.bit.GPIO21
#define Air2_Valve_Set GpioDataRegs.GPBSET.bit.GPIO39
#define Air2_Valve_Clc GpioDataRegs.GPBCLEAR.bit.GPIO39

extern void KEY_Hand_Handle(void);

typedef struct {      // bit  description
    Uint16      Trg;
    Uint16      Cont;
    Uint16      DownTrg;
    Uint16      OkTrg;
    Uint16      UpTrg;
    Uint16      AirTrg;
    Uint16      ModeTrg;
    Uint16      Enable;
    Uint16      OkTrgCnt;

    Uint16      Mode;
    Uint16      Para;
    Uint16      UpCnt;
    Uint16      DownCnt;
    Uint16      LEDEnable;

    Uint16      DisplayData;
    Uint16      DisplayEnable;

    Uint16      LED_DATA[4];
    Uint16      NixieTube_DATA[16];
}KEY_HANDLE ;

#define KEY_HANDLE_DEFAULT {0,0,0,0,0,0,0,0,0,\
                            0,1,0,0,1,\
                            0,0,\
                            {0x02,0x04,0x01,0x06},{0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71}}

extern KEY_HANDLE KeyHandle;

typedef struct {      // bit  description
    Uint16      Enable;
    Uint16      State;
    Uint16      OPTO_Flag;
    Uint16      Conter;
    Uint16      Mount;
    Uint16      StartFlag;
    Uint16      Delay;
    Uint16      KeyCnt;
    Uint16      KeyEnable;
    Uint16      ValidKey;
    Uint16      InvalidStart;//夹住珠子时仍启动5次以上则报错停车
    Uint16      ErrorMount;//记录珠子个数，在140颗珠子内霍尔一直无感应则报错停车
    Uint16      ErrorEnable;
    Uint16      ErrorRestartMount;

    Uint16      OPTO_His;
    Uint16      Step_His;
    Uint16      DIS;
    Uint16      OnCnt;
    Uint16      OffCnt;
    Uint16      Z_Flag;
    Uint16      Dis_Flag1;
    Uint16      Dis_Flag2;

    Uint16      C_FLAG;
    Uint16      OPTO_Flag_HIS;

    Uint16      TimerEnable;
    Uint32      Timer;
}PREPEARL_HANDLE ;

#define PrePearl_HANDLE_DEFAULT {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

extern PREPEARL_HANDLE PrePearl_C;
extern PREPEARL_HANDLE PrePearl_D;

extern void PrePearlSFM(PREPEARL_HANDLE *pPearl,MOTOR_CONTROL_SPTA *pmotor);

#endif /* HC595_H_ */
