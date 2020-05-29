//###########################################################################
//
// FILE:    F2806x_ECan.c
//
// TITLE:   F2806x Enhanced CAN Initialization & Support Functions.
//
//###########################################################################
// $TI Release: F2806x C/C++ Header Files and Peripheral Examples V150 $
// $Release Date: June 16, 2015 $
// $Copyright: Copyright (C) 2011-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "Stepper-Settings.h"    //参数设置头文件


interrupt void ECAN0INT_ISR(void); //eCAN接收中断

#define TXaddr  0x50f
#define RXaddr  0x500


//CAN位时间设置
#define TQ8    0
#define TQ10   1
#define TQ15   2
#define TQ20   3
#define TQ_setup  TQ20   //采样点 62.5%（for MD801)
//---------------------------------------------------------------------------
// InitECan:
//---------------------------------------------------------------------------
// This function initializes the eCAN module to a known state.
//
#if DSP28_ECANA
void InitECan(void)
{
   InitECana();
}
#endif

#if DSP28_ECANA
void InitECana(void)        // Initialize eCAN-A module
{

	 struct ECAN_REGS ECanaShadow;
	    //eCAN的控制和状态寄存器只允许32位访问

	    //eCAN时钟必须使能，for DSP2803x CAN_CLK=SYSCLKOUT/2=60MHZ/2=30MHz

	    //*************step1: eCAN管脚配置***********************
	    EALLOW;

	      /* 使能eCAN时钟 */
	    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;  // eCAN-A(使用）1-SYSCLKOUT/2

	     /* 使能管脚内部上拉 */
		GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	 //"0" Enable pull-up for GPIO30 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	 //"1" Disable pull-up for GPIO31 (CANTXA)

		// GpioCtrlRegs.GPAPUD.bit.GPIO31 = 1;  //"1": Disable the internal pullup on GPIO31(CANTXA)
		                                         //(袜机，禁用CANTX的芯片内部上拉功能)

	     /* 配置输入量化时间 */
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3; // 异步 for GPIO30 (CANRXA)

	    /* 配置eCAN对应IO管脚 */
		GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1; // Configure GPIO30 for CANRXA
		GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1; // Configure GPIO31 for CANTXA
	    EDIS;

	    /* 配置eCAN IO控制寄存器 */
	    EALLOW;
	    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
	    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
	    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

	    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
	    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
	    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;
	    EDIS;

	/* Initialize all bits of 'Message Control Register' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero

	    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;

	// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
	//	as a matter of precaution.

		ECanaRegs.CANTA.all	= 0xFFFFFFFF;	/* Clear all TAn bits */

		ECanaRegs.CANRMP.all = 0xFFFFFFFF;	/* Clear all RMPn bits */

		ECanaRegs.CANGIF0.all = 0xFFFFFFFF; /* Clear all interrupt flag bits */
		ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

		ECanaRegs.CANES.all = 0xFFFFFFFF;   /* Clear all 错误标志*/

		//中断标志位CANGIFx必须通过向CANTA或CANRMP写1来清除，CANGIFx是只读的

	//*************step2: eCAN模块初始化***********************

	    //主控寄存器（CANMC），受EALLOW保护，仅支持32位访问
	     //bit.CCR = 1;//要求允许改变位时间配置寄存器（CANBTC）
	    EALLOW;
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	    ECanaShadow.CANMC.bit.CCR = 1; //要求允许改变配置寄存器
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	    EDIS;

	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 1);
	    //  CCE=1，表示可以对位时间配置寄存器（CANBTC）进行操作

	//--------------------------------------------------------
	    //位时间配置寄存器(CANBTC),受EALLOW保护,并且只能在初始化模式写入
	    //CAN时钟频率=CPU时钟60MHz/2=30MHz

	    EALLOW;
	    ECanaShadow.CANBTC.all= ECanaShadow.CANBTC.all;

	    #if(TQ_setup==TQ8)  //8TQ--62.5%
	   //原250kbit/s位时间配置
	    //通讯波特率=30MHz/(((14+1)*(1+(3+1)+(2+1))=250 kbit/s(8位时间配置）
	    //采样位时间为(TSEG1REG +1)+1/8*TQ=5/8=66.7%，(8位时间配置）
	    //ECanaShadow.CANBTC.bit.BRPREG = 14;//对应波特率250kbit/s(刺绣机使用,总共10TQ）
	   // ECanaShadow.CANBTC.bit.BRPREG = 5;//对应波特率500kbit/s
	    ECanaShadow.CANBTC.bit.BRPREG = 2;//对应波特率1Mbit/s（调试用）
	    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //
	    ECanaShadow.CANBTC.bit.TSEG2REG = 2;  //Bit time=10TQ
	    ECanaShadow.CANBTC.bit.SAM = 0;      //CAN总线数据采样3次(需BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910要求同步段为1TQ，同步跳转宽度=SJW+1（TQ）（SJW<=min(TSEG2,4TQ))
	    #endif

	    #if(TQ_setup==TQ10)  //10TQ--62.5%
	    //原250kbit/s位时间配置
	    //通讯波特率=30MHz/(((14+1)*(1+(4+1)+(3+1))=250 kbit/s(8位时间配置）
	    //采样位时间为(TSEG1REG +1)+1/8*TQ=6/10=60%，(8位时间配置）
	    ECanaShadow.CANBTC.bit.BRPREG = 11;//对应波特率250kbit/s(刺绣机使用,总共10TQ）
	    // ECanaShadow.CANBTC.bit.BRPREG = 5;//对应波特率500kbit/s
	    // ECanaShadow.CANBTC.bit.BRPREG = 2;//对应波特率1Mbit/s（调试用）
	    ECanaShadow.CANBTC.bit.TSEG1REG = 4;   //
	    ECanaShadow.CANBTC.bit.TSEG2REG = 3;  //Bit time=10TQ
	    ECanaShadow.CANBTC.bit.SAM = 1;      //CAN总线数据采样3次(需BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 0;   //DH910要求同步段为1TQ，同步跳转宽度=SJW+1（TQ）（SJW<=min(TSEG2,4TQ))
	    #endif

	    #if(TQ_setup==TQ15)  //15TQ
	     //原250kbit/s位时间配置,采样位时间为(TSEG1REG +1)+1/15*TQ=66.7%，(15位时间配置）
	      //通讯波特率=30MHz(((7+1)*(1+(8+1)+(4+1))=250 kbit/s(15位时间配置）
	     // ECanaShadow.CANBTC.bit.BRPREG = 7;//对应波特率250kbit/s(刺绣机使用,总共10TQ）
	     // ECanaShadow.CANBTC.bit.BRPREG = 3;//对应波特率500kbit/s
	      ECanaShadow.CANBTC.bit.BRPREG = 1;//对应波特率1Mbit/s（调试用）
	      ECanaShadow.CANBTC.bit.TSEG1REG = 8;   //8,采样点=15-5/15=66.7%    7,采样点=15-6/15=60%
	      ECanaShadow.CANBTC.bit.TSEG2REG = 4;  //Bit time=15TQ
	      ECanaShadow.CANBTC.bit.SAM = 0;      //CAN总线数据采样3次(需BPR>4)
	      ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910要求同步段为1TQ，同步跳转宽度=SJW+1（TQ）（SJW<=min(TSEG2,4TQ))
	    #endif

	    // MD801B--采用ISO1050--CAN波特率可达500kbit/s
	    #if(TQ_setup==TQ20)  //20TQ   DH910(8TQ,采样点5/8=62.5%）
	    //新250kbit/s位时间配置--DH910推荐采样位时间为(TSEG1REG +1)+1/(20*TQ)=13/20=65%，
	    //通讯波特率=30MHz(((5+1)*(1+(11+1)+(6+1))=250 kbit/s(20位时间配置）
	    //ECanaShadow.CANBTC.bit.BRPREG = 2;    //对应波特率500kbit/s(总共20TQ）
	    #if(0)
	    ECanaShadow.CANBTC.bit.BRPREG = 5;    //对应波特率250kbit/s(刺绣机使用,总共20TQ）
	    ECanaShadow.CANBTC.bit.TSEG1REG = 11; //10(采样点=60%),11(采样点=65%),12(采样点=70%)
	    ECanaShadow.CANBTC.bit.TSEG2REG = 6;  //7,6,5  Bit time=20TQ
	    ECanaShadow.CANBTC.bit.SAM = 0;      //CAN总线数据采样3次(需BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910要求同步段为1TQ，同步跳转宽度=SJWreg+1（TQ）（SJW<=min(TSEG2,4TQ))
	    #else
	    //Bootloader
	    ECanaShadow.CANBTC.bit.BRPREG = 8;    //对应波特率250kbit/s(刺绣机使用,总共20TQ）
	    ECanaShadow.CANBTC.bit.TSEG1REG = 10; //10(采样点=60%),11(采样点=65%),12(采样点=70%)
	    ECanaShadow.CANBTC.bit.TSEG2REG = 7;  //7,6,5  Bit time=20TQ
	    ECanaShadow.CANBTC.bit.SAM = 1;      //CAN总线数据采样3次(需BPR>4)
	    #endif

	    #endif

	    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

	    //CPU请求正常操作
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;//读主控制寄存器CANMC的值
	    ECanaShadow.CANMC.bit.CCR = 0; // Set CCR = 0，
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	    EDIS;
	//--------------------------------------------------------

	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 0);  // Wait for CCE bit to be cleared.
		//eCAN模块初始化完成

	//*************step3: eCAN邮箱设置***********************
	    //修改邮箱配置前，需先禁止相应邮箱，修改配置后，再重新使能相应邮箱

		 /*写MSGIDs之前必须禁止所有邮箱 */
	 	ECanaRegs.CANME.all = 0x0; //复位所有邮箱

	 	//刺绣机主控采用标准ID（11位）

	 	//邮箱0--ID设置(发送邮箱--_Kvaser)（发送ID号越小，发送优先级越高）
	 	ECanaMboxes.MBOX0.MSGID.bit.IDE = 0;  //"0":采用标准ID
	 	ECanaMboxes.MBOX0.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	 	ECanaMboxes.MBOX0.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
		ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = TXaddr;

		//邮箱1--ID设置（发送邮箱--给主控发送数据用）
	    ECanaMboxes.MBOX1.MSGID.bit.IDE = 0;  //"0":采用标准ID
	    ECanaMboxes.MBOX1.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	    ECanaMboxes.MBOX1.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
	    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = TXaddr;

	    //------邮箱0--发送；邮箱1--接收；(Bootloader)

	    //邮箱2--ID设置（接收邮箱--PN291和主控）
	    ECanaMboxes.MBOX2.MSGID.bit.IDE = 0;  //"0":采用标准ID
	    ECanaMboxes.MBOX2.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	    ECanaMboxes.MBOX2.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
	    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = RXaddr;

	   //邮箱3--ID--对应邮箱3（发送邮箱--给PN291发送数据用)（发送ID号越小，发送优先级越高）
	    ECanaMboxes.MBOX3.MSGID.bit.IDE = 0;  //"0":采用标准ID
	    ECanaMboxes.MBOX3.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	    ECanaMboxes.MBOX3.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
	    ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = TXaddr;

	    //邮箱4--ID--对应邮箱4（发送邮箱--给PN291发送数据用）（发送ID号越小，发送优先级越高）
	     ECanaMboxes.MBOX4.MSGID.bit.IDE = 0;  //"0":采用标准ID
	     ECanaMboxes.MBOX4.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	     ECanaMboxes.MBOX4.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
	     ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = TXaddr;

	     ECanaMboxes.MBOX5.MSGID.bit.IDE = 0;  //"0":采用标准ID
	     ECanaMboxes.MBOX5.MSGID.bit.AME = 1;  //对接收邮箱有效，"1"使能相应的接收屏蔽
	     ECanaMboxes.MBOX5.MSGID.bit.AAM = 0;  //对发送邮箱有效，"0"禁止自动应答模式位
	     ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = TXaddr;

	    // 邮箱方向设置（0：发送；1：接收）
	    ECanaRegs.CANMD.all = 0xFFFFFF04; //“0”发送;“1”接收；---0,1,3,4,邮箱发送，2接收

	   //邮箱信息控制器-MSGCTRL（仅在配置为发送邮箱或邮箱被禁止时可写入）
	    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;//数据长度DLC=8个字节  （向主控发送-DLC=8）
	    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;//数据长度DLC=2个字节  （接收主控数据）
	    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;//数据长度DLC=2个字节   （）
	    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;//数据长度DLC=2个字节   （）
	    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;//数据长度DLC=2个字节   （）
	    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 2;//数据长度DLC=2个字节   （）

	    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;//无远程帧
	    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;//无远程帧
	    ECanaMboxes.MBOX2.MSGCTRL.bit.RTR = 0;//无远程帧
	    ECanaMboxes.MBOX3.MSGCTRL.bit.RTR = 0;//无远程帧
	    ECanaMboxes.MBOX4.MSGCTRL.bit.RTR = 0;//无远程帧
	    ECanaMboxes.MBOX5.MSGCTRL.bit.RTR = 0;//无远程帧

	    ECanaMboxes.MBOX1.MSGCTRL.bit.TPL = 31;//TPL越大则发送优先级最高

	    //邮箱使能设置
	   // ECanaRegs.CANME.all=0xFFFFFFFF;//使能所有邮箱
	      ECanaRegs.CANME.all=0x0000003F;//使能0,1,2,3,4,5邮箱

	    //邮箱局部接收滤波器设置
	   // ECanaLAMRegs.LAM1.all = 0x80000000;//接收邮箱1,ID必需完成匹配才能接收(ID0x001)
	   // ECanaLAMRegs.LAM1.all = 0x1FFFFFFF;//邮箱1接收所有ID(自测试时使用）

	    ECanaLAMRegs.LAM2.all = 0x1FFFFFFF;//邮箱2接收所有ID(接收PN291，M98）

	    ECanaRegs.CANOPC.all = 0x0;  //"0"旧消息会被新消息写覆盖;"1"存储在该邮箱中的旧消息被保护不被新消息写覆盖。

	    //把待发送的数据写入发送邮箱
	    ECanaMboxes.MBOX0.MDL.all = 0x0;
	    ECanaMboxes.MBOX0.MDH.all = 0x0;
	    ECanaMboxes.MBOX1.MDL.all = 0x0;
	    ECanaMboxes.MBOX1.MDH.all = 0x0;
	    ECanaMboxes.MBOX2.MDL.all = 0x0;
	    ECanaMboxes.MBOX2.MDH.all = 0x0;
	    ECanaMboxes.MBOX3.MDL.all = 0x0;
	    ECanaMboxes.MBOX3.MDH.all = 0x0;
	    ECanaMboxes.MBOX4.MDL.all = 0x0;
	    ECanaMboxes.MBOX4.MDH.all = 0x0;
	    ECanaMboxes.MBOX5.MDL.all = 0x0;
	    ECanaMboxes.MBOX5.MDH.all = 0x0;

	 //*************step4: eCAN工作模式设置***********************
	    //在自测试模式时，若无可接收邮箱，则数据会送往邮箱0
	    EALLOW;
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	    ECanaShadow.CANMC.bit.STM = 0; //"0"正常模式；"1"自测试模式
	    ECanaShadow.CANMC.bit.SCB = 1; //"1" 选择eCAN mode
	    ECanaShadow.CANMC.bit.DBO = 0; //"0"数据存放顺序与Kavaser一致
	    //"0"首先接收或发送数据的最高有效字节D0(CANMDL=D0 D1 D2 D3)  (CANMDL=D4 D5 D6 D7)
	    //"1"首先接收或发送数据的最低有效字节D0(CANMDL= D3 D2 D1 D0) CANMDH=D7 D6 D5 D4

	    ECanaShadow.CANMC.bit.ABO = 1; //总线自动开启位
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	    EDIS;

	 //*************step5: eCAN中断设置***********************
	    EALLOW;
		//全局中断屏蔽寄存器
		ECanaRegs.CANGIM.bit.I0EN = 1;//使能ECAN0INT中断

		//邮箱中断屏蔽寄存器
		ECanaRegs.CANMIM.bit.MIM2 = 1;//使能邮箱2接收中断（接收主控数据）

		//邮箱中断优先级寄存器（决定相应邮箱中断产生在ECAN0INT或ECAN1INT）
		ECanaRegs.CANMIL.all = 0x0;//邮箱中断产生在ECAN0INT
	    EDIS;

	//*************step6: DSP-eCAN中断设置***********************
	    //设置中断向量表
	    EALLOW;
	    PieVectTable.ECAN0INTA = &ECAN0INT_ISR;//eCAN接收中断
	    EDIS;
	    PieCtrlRegs.PIEIER9.all |= M_INT5;   //对应PIE9.5--ECAN0_INT
	    IER |= M_INT9;   // Enable CPU INT9 for eCAN_INT(INT9.5)
}
#endif // endif DSP28_ECANA

//---------------------------------------------------------------------------
// Example: InitECanGpio:
//---------------------------------------------------------------------------
// This function initializes GPIO pins to function as eCAN pins
//
// Each GPIO pin can be configured as a GPIO pin or up to 3 different
// peripheral functional pins. By default all pins come up as GPIO
// inputs after reset.
//
// Caution:
// Only one GPIO pin should be enabled for CANTXA operation.
// Only one GPIO pin shoudl be enabled for CANRXA operation.
// Comment out other unwanted lines.

#if DSP28_ECANA
void InitECanGpio(void)
{
   InitECanaGpio();
}
#endif

#if DSP28_ECANA
void InitECanaGpio(void)
{
   EALLOW;

/* Enable internal pull-up for the selected CAN pins */
// Pull-ups can be enabled or disabled by the user.
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;     // Enable pull-up for GPIO30 (CANRXA)
    GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;     // Enable pull-up for GPIO31 (CANTXA)

/* Set qualification for selected CAN pins to asynch only */
// Inputs are synchronized to SYSCLKOUT by default.
// This will select asynch (no qualification) for the selected pins.

    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)

/* Configure eCAN-A pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be eCAN functional pins.

    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;    // Configure GPIO30 for CANRXA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;    // Configure GPIO31 for CANTXA operation

    EDIS;
}
#endif // endif DSP28_ECANA

/* Note: Bit timing parameters must be chosen based on the network parameters such as
   the sampling point desired and the propagation delay of the network. The propagation
   delay is a function of length of the cable, delay introduced by the
   transceivers and opto/galvanic-isolators (if any).

   The parameters used in this file must be changed taking into account the above mentioned
   factors in order to arrive at the bit-timing parameters suitable for a network.
*/

//===========================================================================
// End of file.
//===========================================================================



