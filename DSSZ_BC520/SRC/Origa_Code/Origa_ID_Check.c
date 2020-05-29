
#include "F2806x_Device.h"
#include "Stepper-Settings.h"

#if (Origa_Ena)

#include "origa_type.h"
#include "board.h"
#include "origa_ecc.h"
#include "origa_nvm.h"


BOOL AuthStat=FALSE;
ULONG ulp_LifeSpanCounter=0x00;
S_ORIGA_PUID stDetectedPuids[4];
extern gf2n_t gf2nReturnX;
extern gf2n_t gf2nReturnZ;

unsigned char channel;
unsigned long g_ulBaudLow;				//1 * BIF_DEFAULT_TIMEBASE
unsigned long g_ulBaudHigh;				//3 * BIF_DEFAULT_TIMEBASE
unsigned long g_ulBaudStop;				//5 * BIF_DEFAULT_TIMEBASE
unsigned long g_ulResponseTimeOut;		//20 * BIF_DEFAULT_TIMEBASE
unsigned long g_ulBaudPowerUpPulse;
unsigned long g_ulBaudPowerDownTime;
unsigned long g_ulBaudPowerUpTime;
unsigned long g_ulBaudResetTime;
unsigned long g_ulIntTimeoutDefault;
unsigned long g_ulImmediateIntTimeOut;
unsigned long g_culNvmTimeout;


void Delay(unsigned short delay)
{
	unsigned long data = delay;
	for(;data;data--);
}


void timing_init(void)
{
	g_ulBaudLow 				= 33;		//1 * BIF_DEFAULT_TIMEBASE;	5us//50
	g_ulBaudHigh 				= 100;	    //3 * BIF_DEFAULT_TIMEBASE; 15us //50*3+20
	g_ulBaudStop				= 170;		//5 * BIF_DEFAULT_TIMEBASE; 25us
	g_ulResponseTimeOut 		= 660;	    //20 * BIF_DEFAULT_TIMEBASE; 100 us

	g_ulBaudPowerUpPulse 		= 4600;		// 30us
	g_ulBaudPowerDownTime		= 4600;		//original 400000// 2ms
	g_ulBaudPowerUpTime			= 23000;	// 10 ms
	g_ulBaudResetTime			= 10000; 		//old 5000

	g_ulIntTimeoutDefault     	= 5160000; 	// 200ms
	g_ulImmediateIntTimeOut   	= 516000; 	// 20ms

	g_culNvmTimeout 			= 8600000;
}

unsigned char get_pin()//读IO
{
	#if(Origa_Ena)
	return GpioDataRegs.GPADAT.bit.GPIO16;
    #else
	return 0;
	#endif
}		   

void set_pin(unsigned char level)//写IO
{
    #if(Origa_Ena)
    if(level==1)
    {
       GpioDataRegs.GPASET.bit.GPIO16 = 1;  //IO="1"
    }
    else if(level==0)
    {
       GpioDataRegs.GPACLEAR.bit.GPIO16 = 1; //IO="0"
    }
	#endif
}

void set_pin_dir(unsigned char dir)
{
	if(dir==0)//input
	{
		EALLOW;
        #if(Origa_Ena)
		GpioCtrlRegs.GPADIR.bit.GPIO16 = 0;   // GPIO11 = input
		#endif
		EDIS;
	}
	else
	{
		EALLOW;
        #if(Origa_Ena)
		GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;   // GPIO11 = output
		#endif
		EDIS;
	}
}

void udelay(volatile unsigned long ul_ticks)
{
	for(;ul_ticks;ul_ticks--);
}


//加密认证函数
//#pragma CODE_SECTION(Origa_ID_Check, "ramfuncs");
void Origa_ID_Check(void)
{
    /*加密认证One-wire时序配置*/
    timing_init();

    #if(0)//Test_Num次加密认证
    /*Test_Num次加密认证,每次需要400ms*/
    #define Test_Num 1  //加密认证次数
    static Uint16 failed_count=0;
    static Uint16 sucess_count=0;
    register Uint16 i,j=0;
    for(j=0;j<Test_Num;j++)
    {
        /*加密认证函数*/
        Swi_PowerDown();//do hardware power down by pushing SWI to low
        Swi_PowerUp();  //start power up sequence by setting SWI to high
        Swi_Reset();    // bus reset
        Swi_SelectByAddress(0);

        for(i=0;i<12;i++)
        {
            if(Swi_ReadConfigSpace(SWI_UID0+11-i, ((UBYTE*)&stDetectedPuids[0])+i) == FALSE)
            { break; }
        }
        if(Ecc_DoAuthentication(&AuthStat) == FALSE)
        {
            failed_count++;
        }
        if(Ecc_DoAuthentication(&AuthStat) == TRUE)
        {
            sucess_count++;
        }
    }

    if(failed_count!=0)//认证不成功,置故障标志
    {
        MC.S_ERR=1; //置故障标志
        MC.Err_Value = 12;//故障码--f12
        MC.Err_Wri_Ena=1;
    }
   #else

    //只进行1次加密认证
    register Uint16 i;
    Swi_PowerDown();// do hardware power down by pushing SWI to low
    Swi_PowerUp();  // start power up sequence by setting SWI to high
    Swi_Reset();    // bus reset
    Swi_SelectByAddress(0);

    for(i=0;i<12;i++)
    {
        if(Swi_ReadConfigSpace(SWI_UID0+11-i, ((UBYTE*)&stDetectedPuids[0])+i) == FALSE)
        { break; }
    }

    if(Ecc_DoAuthentication(&AuthStat) == FALSE)//认证不成功
    {
       MC.S_ERR=1; //置故障标志
       MC.Err_Value |= BIT0;//加密失败
       MC.Err_Wri_Ena=1;
    }
   #endif
}

#endif
