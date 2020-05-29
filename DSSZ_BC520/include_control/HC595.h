/*
 * HC595.h
 *
 *  Created on: 2016Äê8ÔÂ17ÈÕ
 *      Author: liudong
 */
//#include "Parameter.h"

#ifndef HC595_H_
#define HC595_H_

#define DataLow  GpioDataRegs.GPACLEAR.bit.GPIO22
#define DataHigh GpioDataRegs.GPASET.bit.GPIO22
#define RclkLow  GpioDataRegs.GPACLEAR.bit.GPIO29
#define RclkHigh GpioDataRegs.GPASET.bit.GPIO29
#define SclkLow  GpioDataRegs.GPACLEAR.bit.GPIO12
#define SclkHigh GpioDataRegs.GPASET.bit.GPIO12

//#define LED_OUT_DATA  GpioDataRegs.GPADAT.bit.GPIO3
#define LED_OUT_DATA_H  GpioDataRegs.GPASET.bit.GPIO3
#define LED_OUT_DATA_L  GpioDataRegs.GPACLEAR.bit.GPIO3
//#define LED_OUT_CLK   GpioDataRegs.GPADAT.bit.GPIO1
#define LED_OUT_CLK_H   GpioDataRegs.GPASET.bit.GPIO1
#define LED_OUT_CLK_L   GpioDataRegs.GPACLEAR.bit.GPIO1

//#define HC597_SHCP GpioDataRegs.GPADAT.bit.GPIO26
#define HC597_SHCP_H GpioDataRegs.GPASET.bit.GPIO26
#define HC597_SHCP_L GpioDataRegs.GPACLEAR.bit.GPIO26
//#define HC597_STCP GpioDataRegs.GPADAT.bit.GPIO27
#define HC597_STCP_H GpioDataRegs.GPASET.bit.GPIO27
#define HC597_STCP_L GpioDataRegs.GPACLEAR.bit.GPIO27
//#define HC597_PL   GpioDataRegs.GPBDAT.bit.GPIO41
#define HC597_PL_H   GpioDataRegs.GPBSET.bit.GPIO41
#define HC597_PL_L   GpioDataRegs.GPBCLEAR.bit.GPIO41

#define HC597_DATA GpioDataRegs.GPADAT.bit.GPIO23

//#define KEY_SHCP GpioDataRegs.GPADAT.bit.GPIO15
#define KEY_SHCP_H GpioDataRegs.GPASET.bit.GPIO15
#define KEY_SHCP_L GpioDataRegs.GPACLEAR.bit.GPIO15
//#define KEY_STCP GpioDataRegs.GPBDAT.bit.GPIO58
#define KEY_STCP_H GpioDataRegs.GPBSET.bit.GPIO58
#define KEY_STCP_L GpioDataRegs.GPBCLEAR.bit.GPIO58
//#define KEY_PL   GpioDataRegs.GPBDAT.bit.GPIO57
#define KEY_PL_H   GpioDataRegs.GPBSET.bit.GPIO57
#define KEY_PL_L   GpioDataRegs.GPBCLEAR.bit.GPIO57

#define KEY_DATA GpioDataRegs.GPADAT.bit.GPIO13

#define M1_CLK GpioDataRegs.GPATOGGLE.bit.GPIO9
#define M1_EN  GpioDataRegs.GPASET.bit.GPIO18
#define M1_DIS GpioDataRegs.GPACLEAR.bit.GPIO18
#define M1_CCW GpioDataRegs.GPACLEAR.bit.GPIO28
#define M1_CW GpioDataRegs.GPASET.bit.GPIO28

#define M2_CLK GpioDataRegs.GPATOGGLE.bit.GPIO17
#define M2_EN  GpioDataRegs.GPBSET.bit.GPIO44
#define M2_DIS GpioDataRegs.GPBCLEAR.bit.GPIO44
#define M2_CCW GpioDataRegs.GPBCLEAR.bit.GPIO52
#define M2_CW GpioDataRegs.GPBSET.bit.GPIO52

#define M3_CLK GpioDataRegs.GPATOGGLE.bit.GPIO7
#define M3_CLK_HI GpioDataRegs.GPASET.bit.GPIO7
#define M3_CLK_LO GpioDataRegs.GPACLEAR.bit.GPIO7
#define M3_EN  GpioDataRegs.GPASET.bit.GPIO14
#define M3_DIS GpioDataRegs.GPACLEAR.bit.GPIO14
#define M3_CCW GpioDataRegs.GPACLEAR.bit.GPIO19
#define M3_CW GpioDataRegs.GPASET.bit.GPIO19

typedef struct{
	Uint16 NewState;
	Uint16 OldState;
	Uint16 TemState;
}HC595CONTROL;

#define HC595CONTROL_DEFAULTS {0,0,0}

typedef struct {      // bit  description
    Uint16      M3_ATE:1;
    Uint16      M2_TOFF:1;
    Uint16      M2_ATE:1;
    Uint16      M3_TQ0:1;
    Uint16      M3_TQ1:1;
    Uint16      M3_DY0:1;
    Uint16      M3_DY1:1;
    Uint16      M3_TOFF:1;
    Uint16      M2_DY1:1;
    Uint16      M1_TQ0:1;
    Uint16      M1_TQ1:1;
    Uint16      M1_TOFF:1;
    Uint16      M1_ATE:1;
    Uint16      M2_TQ0:1;
    Uint16      M2_TQ1:1;
    Uint16      M2_DY0:1;
}HC595_BITS ;

/* Allow access to the bit fields or entire register */
typedef union {
   Uint16 all;
   HC595_BITS bit;
}HC595_REG;
extern HC595_REG HC595Regs;

typedef struct {      // bit  description
    Uint16      D0:1;
    Uint16      Down:1;
    Uint16      OK:1;
    Uint16      Up:1;
    Uint16      Dir:1;
    Uint16      Mode:1;
    Uint16      rev:10;
}KEY_HC597_BITS ;

/* Allow access to the bit fields or entire register */
typedef union {
   Uint16 all;
   KEY_HC597_BITS bit;
}KEY_HC597_REG;

extern KEY_HC597_REG KEY_HC597Regs;

typedef struct {      // bit  description
    Uint16      DIP2:1;
    Uint16      DIP1:1;
    Uint16      OPTO1:1;
    Uint16      OPTO2:1;
    Uint16      OPTO3:1;
    Uint16      OPTO4:1;
    Uint16      OPTO5:1;
    Uint16      D7:1;
    Uint16      OTW1_8412:1;
    Uint16      OTW2_8412:1;
    Uint16      FAULT1_8412:1;
    Uint16      FAULT2_8412:1;
    Uint16      D4:1;
    Uint16      M1_ALARM:1;
    Uint16      M2_ALARM:1;
    Uint16      M3_ALARM:1;
}INPUT_HC597_BITS ;

/* Allow access to the bit fields or entire register */
typedef union {
   Uint16 all;
   INPUT_HC597_BITS bit;
}INPUT_HC597_REG;
extern INPUT_HC597_REG INPUT_HC597Regs;

extern void HC595SendData(Uint16 SendVal);
extern void LED_HC595SendData(Uint16 SendVal);
extern Uint16 READ_HC597(void);
extern Uint16 KEY_HC597(void);


#endif /* HC595_H_ */
