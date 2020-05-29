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
#include "Stepper-Settings.h"    //��������ͷ�ļ�


interrupt void ECAN0INT_ISR(void); //eCAN�����ж�

#define TXaddr  0x50f
#define RXaddr  0x500


//CANλʱ������
#define TQ8    0
#define TQ10   1
#define TQ15   2
#define TQ20   3
#define TQ_setup  TQ20   //������ 62.5%��for MD801)
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
	    //eCAN�Ŀ��ƺ�״̬�Ĵ���ֻ����32λ����

	    //eCANʱ�ӱ���ʹ�ܣ�for DSP2803x CAN_CLK=SYSCLKOUT/2=60MHZ/2=30MHz

	    //*************step1: eCAN�ܽ�����***********************
	    EALLOW;

	      /* ʹ��eCANʱ�� */
	    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;  // eCAN-A(ʹ�ã�1-SYSCLKOUT/2

	     /* ʹ�ܹܽ��ڲ����� */
		GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	 //"0" Enable pull-up for GPIO30 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	 //"1" Disable pull-up for GPIO31 (CANTXA)

		// GpioCtrlRegs.GPAPUD.bit.GPIO31 = 1;  //"1": Disable the internal pullup on GPIO31(CANTXA)
		                                         //(���������CANTX��оƬ�ڲ���������)

	     /* ������������ʱ�� */
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3; // �첽 for GPIO30 (CANRXA)

	    /* ����eCAN��ӦIO�ܽ� */
		GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1; // Configure GPIO30 for CANRXA
		GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1; // Configure GPIO31 for CANTXA
	    EDIS;

	    /* ����eCAN IO���ƼĴ��� */
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

		ECanaRegs.CANES.all = 0xFFFFFFFF;   /* Clear all �����־*/

		//�жϱ�־λCANGIFx����ͨ����CANTA��CANRMPд1�������CANGIFx��ֻ����

	//*************step2: eCANģ���ʼ��***********************

	    //���ؼĴ�����CANMC������EALLOW��������֧��32λ����
	     //bit.CCR = 1;//Ҫ������ı�λʱ�����üĴ�����CANBTC��
	    EALLOW;
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	    ECanaShadow.CANMC.bit.CCR = 1; //Ҫ������ı����üĴ���
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	    EDIS;

	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 1);
	    //  CCE=1����ʾ���Զ�λʱ�����üĴ�����CANBTC�����в���

	//--------------------------------------------------------
	    //λʱ�����üĴ���(CANBTC),��EALLOW����,����ֻ���ڳ�ʼ��ģʽд��
	    //CANʱ��Ƶ��=CPUʱ��60MHz/2=30MHz

	    EALLOW;
	    ECanaShadow.CANBTC.all= ECanaShadow.CANBTC.all;

	    #if(TQ_setup==TQ8)  //8TQ--62.5%
	   //ԭ250kbit/sλʱ������
	    //ͨѶ������=30MHz/(((14+1)*(1+(3+1)+(2+1))=250 kbit/s(8λʱ�����ã�
	    //����λʱ��Ϊ(TSEG1REG +1)+1/8*TQ=5/8=66.7%��(8λʱ�����ã�
	    //ECanaShadow.CANBTC.bit.BRPREG = 14;//��Ӧ������250kbit/s(�����ʹ��,�ܹ�10TQ��
	   // ECanaShadow.CANBTC.bit.BRPREG = 5;//��Ӧ������500kbit/s
	    ECanaShadow.CANBTC.bit.BRPREG = 2;//��Ӧ������1Mbit/s�������ã�
	    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //
	    ECanaShadow.CANBTC.bit.TSEG2REG = 2;  //Bit time=10TQ
	    ECanaShadow.CANBTC.bit.SAM = 0;      //CAN�������ݲ���3��(��BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910Ҫ��ͬ����Ϊ1TQ��ͬ����ת���=SJW+1��TQ����SJW<=min(TSEG2,4TQ))
	    #endif

	    #if(TQ_setup==TQ10)  //10TQ--62.5%
	    //ԭ250kbit/sλʱ������
	    //ͨѶ������=30MHz/(((14+1)*(1+(4+1)+(3+1))=250 kbit/s(8λʱ�����ã�
	    //����λʱ��Ϊ(TSEG1REG +1)+1/8*TQ=6/10=60%��(8λʱ�����ã�
	    ECanaShadow.CANBTC.bit.BRPREG = 11;//��Ӧ������250kbit/s(�����ʹ��,�ܹ�10TQ��
	    // ECanaShadow.CANBTC.bit.BRPREG = 5;//��Ӧ������500kbit/s
	    // ECanaShadow.CANBTC.bit.BRPREG = 2;//��Ӧ������1Mbit/s�������ã�
	    ECanaShadow.CANBTC.bit.TSEG1REG = 4;   //
	    ECanaShadow.CANBTC.bit.TSEG2REG = 3;  //Bit time=10TQ
	    ECanaShadow.CANBTC.bit.SAM = 1;      //CAN�������ݲ���3��(��BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 0;   //DH910Ҫ��ͬ����Ϊ1TQ��ͬ����ת���=SJW+1��TQ����SJW<=min(TSEG2,4TQ))
	    #endif

	    #if(TQ_setup==TQ15)  //15TQ
	     //ԭ250kbit/sλʱ������,����λʱ��Ϊ(TSEG1REG +1)+1/15*TQ=66.7%��(15λʱ�����ã�
	      //ͨѶ������=30MHz(((7+1)*(1+(8+1)+(4+1))=250 kbit/s(15λʱ�����ã�
	     // ECanaShadow.CANBTC.bit.BRPREG = 7;//��Ӧ������250kbit/s(�����ʹ��,�ܹ�10TQ��
	     // ECanaShadow.CANBTC.bit.BRPREG = 3;//��Ӧ������500kbit/s
	      ECanaShadow.CANBTC.bit.BRPREG = 1;//��Ӧ������1Mbit/s�������ã�
	      ECanaShadow.CANBTC.bit.TSEG1REG = 8;   //8,������=15-5/15=66.7%    7,������=15-6/15=60%
	      ECanaShadow.CANBTC.bit.TSEG2REG = 4;  //Bit time=15TQ
	      ECanaShadow.CANBTC.bit.SAM = 0;      //CAN�������ݲ���3��(��BPR>4)
	      ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910Ҫ��ͬ����Ϊ1TQ��ͬ����ת���=SJW+1��TQ����SJW<=min(TSEG2,4TQ))
	    #endif

	    // MD801B--����ISO1050--CAN�����ʿɴ�500kbit/s
	    #if(TQ_setup==TQ20)  //20TQ   DH910(8TQ,������5/8=62.5%��
	    //��250kbit/sλʱ������--DH910�Ƽ�����λʱ��Ϊ(TSEG1REG +1)+1/(20*TQ)=13/20=65%��
	    //ͨѶ������=30MHz(((5+1)*(1+(11+1)+(6+1))=250 kbit/s(20λʱ�����ã�
	    //ECanaShadow.CANBTC.bit.BRPREG = 2;    //��Ӧ������500kbit/s(�ܹ�20TQ��
	    #if(0)
	    ECanaShadow.CANBTC.bit.BRPREG = 5;    //��Ӧ������250kbit/s(�����ʹ��,�ܹ�20TQ��
	    ECanaShadow.CANBTC.bit.TSEG1REG = 11; //10(������=60%),11(������=65%),12(������=70%)
	    ECanaShadow.CANBTC.bit.TSEG2REG = 6;  //7,6,5  Bit time=20TQ
	    ECanaShadow.CANBTC.bit.SAM = 0;      //CAN�������ݲ���3��(��BPR>4)
	    ECanaShadow.CANBTC.bit.SJWREG = 1;   //DH910Ҫ��ͬ����Ϊ1TQ��ͬ����ת���=SJWreg+1��TQ����SJW<=min(TSEG2,4TQ))
	    #else
	    //Bootloader
	    ECanaShadow.CANBTC.bit.BRPREG = 8;    //��Ӧ������250kbit/s(�����ʹ��,�ܹ�20TQ��
	    ECanaShadow.CANBTC.bit.TSEG1REG = 10; //10(������=60%),11(������=65%),12(������=70%)
	    ECanaShadow.CANBTC.bit.TSEG2REG = 7;  //7,6,5  Bit time=20TQ
	    ECanaShadow.CANBTC.bit.SAM = 1;      //CAN�������ݲ���3��(��BPR>4)
	    #endif

	    #endif

	    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

	    //CPU������������
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;//�������ƼĴ���CANMC��ֵ
	    ECanaShadow.CANMC.bit.CCR = 0; // Set CCR = 0��
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	    EDIS;
	//--------------------------------------------------------

	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 0);  // Wait for CCE bit to be cleared.
		//eCANģ���ʼ�����

	//*************step3: eCAN��������***********************
	    //�޸���������ǰ�����Ƚ�ֹ��Ӧ���䣬�޸����ú�������ʹ����Ӧ����

		 /*дMSGIDs֮ǰ�����ֹ�������� */
	 	ECanaRegs.CANME.all = 0x0; //��λ��������

	 	//��������ز��ñ�׼ID��11λ��

	 	//����0--ID����(��������--_Kvaser)������ID��ԽС���������ȼ�Խ�ߣ�
	 	ECanaMboxes.MBOX0.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	 	ECanaMboxes.MBOX0.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	 	ECanaMboxes.MBOX0.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
		ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = TXaddr;

		//����1--ID���ã���������--�����ط��������ã�
	    ECanaMboxes.MBOX1.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	    ECanaMboxes.MBOX1.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	    ECanaMboxes.MBOX1.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
	    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = TXaddr;

	    //------����0--���ͣ�����1--���գ�(Bootloader)

	    //����2--ID���ã���������--PN291�����أ�
	    ECanaMboxes.MBOX2.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	    ECanaMboxes.MBOX2.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	    ECanaMboxes.MBOX2.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
	    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = RXaddr;

	   //����3--ID--��Ӧ����3����������--��PN291����������)������ID��ԽС���������ȼ�Խ�ߣ�
	    ECanaMboxes.MBOX3.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	    ECanaMboxes.MBOX3.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	    ECanaMboxes.MBOX3.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
	    ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = TXaddr;

	    //����4--ID--��Ӧ����4����������--��PN291���������ã�������ID��ԽС���������ȼ�Խ�ߣ�
	     ECanaMboxes.MBOX4.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	     ECanaMboxes.MBOX4.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	     ECanaMboxes.MBOX4.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
	     ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = TXaddr;

	     ECanaMboxes.MBOX5.MSGID.bit.IDE = 0;  //"0":���ñ�׼ID
	     ECanaMboxes.MBOX5.MSGID.bit.AME = 1;  //�Խ���������Ч��"1"ʹ����Ӧ�Ľ�������
	     ECanaMboxes.MBOX5.MSGID.bit.AAM = 0;  //�Է���������Ч��"0"��ֹ�Զ�Ӧ��ģʽλ
	     ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = TXaddr;

	    // ���䷽�����ã�0�����ͣ�1�����գ�
	    ECanaRegs.CANMD.all = 0xFFFFFF04; //��0������;��1�����գ�---0,1,3,4,���䷢�ͣ�2����

	   //������Ϣ������-MSGCTRL����������Ϊ������������䱻��ֹʱ��д�룩
	    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;//���ݳ���DLC=8���ֽ�  �������ط���-DLC=8��
	    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;//���ݳ���DLC=2���ֽ�  �������������ݣ�
	    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;//���ݳ���DLC=2���ֽ�   ����
	    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;//���ݳ���DLC=2���ֽ�   ����
	    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;//���ݳ���DLC=2���ֽ�   ����
	    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 2;//���ݳ���DLC=2���ֽ�   ����

	    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;//��Զ��֡
	    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;//��Զ��֡
	    ECanaMboxes.MBOX2.MSGCTRL.bit.RTR = 0;//��Զ��֡
	    ECanaMboxes.MBOX3.MSGCTRL.bit.RTR = 0;//��Զ��֡
	    ECanaMboxes.MBOX4.MSGCTRL.bit.RTR = 0;//��Զ��֡
	    ECanaMboxes.MBOX5.MSGCTRL.bit.RTR = 0;//��Զ��֡

	    ECanaMboxes.MBOX1.MSGCTRL.bit.TPL = 31;//TPLԽ���������ȼ����

	    //����ʹ������
	   // ECanaRegs.CANME.all=0xFFFFFFFF;//ʹ����������
	      ECanaRegs.CANME.all=0x0000003F;//ʹ��0,1,2,3,4,5����

	    //����ֲ������˲�������
	   // ECanaLAMRegs.LAM1.all = 0x80000000;//��������1,ID�������ƥ����ܽ���(ID0x001)
	   // ECanaLAMRegs.LAM1.all = 0x1FFFFFFF;//����1��������ID(�Բ���ʱʹ�ã�

	    ECanaLAMRegs.LAM2.all = 0x1FFFFFFF;//����2��������ID(����PN291��M98��

	    ECanaRegs.CANOPC.all = 0x0;  //"0"����Ϣ�ᱻ����Ϣд����;"1"�洢�ڸ������еľ���Ϣ��������������Ϣд���ǡ�

	    //�Ѵ����͵�����д�뷢������
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

	 //*************step4: eCAN����ģʽ����***********************
	    //���Բ���ģʽʱ�����޿ɽ������䣬�����ݻ���������0
	    EALLOW;
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	    ECanaShadow.CANMC.bit.STM = 0; //"0"����ģʽ��"1"�Բ���ģʽ
	    ECanaShadow.CANMC.bit.SCB = 1; //"1" ѡ��eCAN mode
	    ECanaShadow.CANMC.bit.DBO = 0; //"0"���ݴ��˳����Kavaserһ��
	    //"0"���Ƚ��ջ������ݵ������Ч�ֽ�D0(CANMDL=D0 D1 D2 D3)  (CANMDL=D4 D5 D6 D7)
	    //"1"���Ƚ��ջ������ݵ������Ч�ֽ�D0(CANMDL= D3 D2 D1 D0) CANMDH=D7 D6 D5 D4

	    ECanaShadow.CANMC.bit.ABO = 1; //�����Զ�����λ
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	    EDIS;

	 //*************step5: eCAN�ж�����***********************
	    EALLOW;
		//ȫ���ж����μĴ���
		ECanaRegs.CANGIM.bit.I0EN = 1;//ʹ��ECAN0INT�ж�

		//�����ж����μĴ���
		ECanaRegs.CANMIM.bit.MIM2 = 1;//ʹ������2�����жϣ������������ݣ�

		//�����ж����ȼ��Ĵ�����������Ӧ�����жϲ�����ECAN0INT��ECAN1INT��
		ECanaRegs.CANMIL.all = 0x0;//�����жϲ�����ECAN0INT
	    EDIS;

	//*************step6: DSP-eCAN�ж�����***********************
	    //�����ж�������
	    EALLOW;
	    PieVectTable.ECAN0INTA = &ECAN0INT_ISR;//eCAN�����ж�
	    EDIS;
	    PieCtrlRegs.PIEIER9.all |= M_INT5;   //��ӦPIE9.5--ECAN0_INT
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



