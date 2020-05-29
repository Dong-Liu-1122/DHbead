/* =======================================================================
File name:  Stepper-Settings.H
Originator:	Digital Control Systems Group Texas Instruments
Description: ��������ͷ�ļ�
==========================================================================
 History: 2017.04.18	Version 1.0
==========================================================================*/
#ifndef PROJ_SETTINGS_H

//------Step1: Select the driver------------//
#define SY200     1
#define Driver   SY200

//------Step2: Select control mode----------//


//------Step3.1: �Զ���ͷ�ļ�����----------//
#include "F2806x_Examples.h"
#include "IQmathLib.h"
#include "Foc.h"       //�����㷨�ṹ��
#include "Ctrl_Para.h" //���Ʋ����ṹ��
//#include "Pos_Regulator.h"   //λ�û�������ͷ�ļ�

//------Step3.2:DSP��������ͷ�ļ����� ---------//
#include "f2803xqep_Stepper.h"
#include "f2803xpwm_Drv8412.h"   //8PWM(��Ƶ������PWM)
#include "f2803xileg_Hall_Drv8412.h"
#include "YaJiao.h"
#include "Baigan.h"
#include "HC595.h"
#include "SPTA.h"
#include "HandKey.h"
#include "DecryptHex.h"

//------Step4: ���ƹ���ѡ��---------//
//������ƹ���
#define Open_Self_Run        0    // "0":������ת
#define Close_Loop           1    // "1":�ջ�
#define Open_Loop           0    // "1":�ջ�
#define Run_Mode       Close_Loop // �������ģʽ
#define Run_Mode_MotorB       Close_Loop // �������ģʽ

#define Debug_Test           0    // "1"��Ӧ����״̬,"0"��Ӧ��������״̬
#define Znc_Z_offset_Ena     1    // "1"��Ӧʹ�ܱ�����ZУ��

//------Step5: DSP������������---------//
#define DSP_Flash_Update  (void (*)(void))(0x3F7FF6) //Boot��������������ڵ�ַ(оƬ�̶���ַ)
#define DSP_Soft_Reset    (void (*)(void))(0x3F5FF6) //�û�����c_int0��ڵ�ַ(for 28031)

//��������DLC=8
#define RX_D0    0xFC //��������D0
#define RX_D1    0xB0 //��������D1

//�����ط���DLC=8
#define TX_D0    0xF5 //�����ط���D0
#define TX_D1    0x07 //�����ط���D1

#define MachineControl GE
#define Ax8 1
#define GE  2

#define DSP_Update_Ena   1   // "1"��ӦDSP�������ʹ��
#define Origa_Ena 1

#if(MachineControl == GE)
#define HardwareNumber  0x32 //Ӳ��ʶ����
#define SoftWareVersion 0x01 //����汾��
#elif(MachineControl == Ax8)
#define HardwareNumber  0x33 //Ӳ��ʶ����
#define SoftWareVersion 0x01 //����汾��
#endif

#define MachineType Double
#define Double 1
#define Three 2

//------Step6:FOC���Ʋ�������---------//
#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

#define HS_Distance 2080

#define PI 3.14159265358979
#define SYSTEM_FREQUENCY    90    //DSP28030 DSP_CLK=60MHz
#define Pole_Pairs          50    //�������������=50
#define BASE_FREQ           2000  //speed_Base=60*2000/50=2400rpm

#define EncoderLine         1000   //#42���,����������(1000��/ת)1000

#define POS_SET_MAX         4000   //1600
#define POS_SET_MAX_HALF    2000   //1600

#define ISR_FREQUENCY       20     //PWM�ж�Ƶ��=20kHz(Drv8412)
#define SpeedLoopPrescaler   4     //ת�ٻ���Ƶϵ��=20kHz/2=10kHz
#define SpeedCalPrescaler    2     //ת�ټ����Ƶϵ��--ת�ټ���Ƶ��=20kHz/(2*10)=1kHz
//�����������
#define Over_Current_Limit   _IQ15(1.1)  //�������������ֵ--IQ(0.1)--0.54A
#define OC_Delay_Set    100         //��ʱʱ��=25us*100=2.5ms
#define Ds_Align_Time   40000L      //d�ᶨλʱ��2s��20000--1s��
//ת�ٻ��˲�
#define Speedref_filter_Freq  800  //ת��ָ���ͨ�˲�Ƶ��
#define Speedfdb_filter_Freq  200  //ת�ٷ�����ͨ�˲�Ƶ��
//�������˲�
#define Iqref_filter_Freq     1000 //����ָ���ͨ�˲�Ƶ��
#define Idqfdb_filter_Freq    1000 //����������ͨ�˲�Ƶ��

//---------Hardware Setup---------------//

//------Step1: DSP����ѡ��---------//
#define IIC_Ena        1  // IIC_Enaʹ��ѡ��
#define CAN_Ena        1
#define ECAP_Ena       1
#define SCI_Ena       0
//------Step2: GPIO�ܽŶ���---------//
//EEPROM��ַ
#define eprom_addr_Error        0x30   //������IIC��ַ
#define eprom_addr_Reset        0xF0   //д���ʼ��������IIC��ַ
#define eprom_addr_Motor_Dir    0x01   //IIC��ַ(0x1)

//����IO��
//#define DIP1           GpioDataRegs.AIODAT.bit.AIO12   //DIP_on��Ӧ��0���͵�ƽ
//#define DIP2           GpioDataRegs.AIODAT.bit.AIO14   //DIP_on��Ӧ��0���͵�ƽ
//SY200--DIPӲ��ֱ������GND
//#define DIP1           GpioDataRegs.GPADAT.bit.GPIO12
//#define DIP2           GpioDataRegs.GPADAT.bit.GPIO13
//#define Enc_Z_IN       GpioDataRegs.GPADAT.bit.GPIO23    //������Z�źš�IO���ߵ�ƽʱ�������

//#define DIP1  1
//#define DIP2  1
#define DIP1  INPUT_HC597Regs.bit.DIP1
#define DIP2  INPUT_HC597Regs.bit.DIP2



//Drv8412��ؿ����ź�
//#define Drv8412_FLT    GpioDataRegs.GPADAT.bit.GPIO5   //Drv8412�����ź�,����Ч
//#define Drv8412_OTW    GpioDataRegs.GPADAT.bit.GPIO3   //Drv8412�����ź�,����Ч

//#define Drv8412_RESET  GpioDataRegs.GPACLEAR.bit.GPIO7=1 //��λDrv8412,����Ч
//#define Drv8412_ENA    GpioDataRegs.GPASET.bit.GPIO7=1   //ʹ��Drv8412
#define Drv8412_RESET  GpioDataRegs.GPBCLEAR.bit.GPIO43=1 //��λDrv8412,����Ч
#define Drv8412_ENA    GpioDataRegs.GPBSET.bit.GPIO43=1   //ʹ��Drv8412

//#define Drv8412_FLT_B    GpioDataRegs.GPBDAT.bit.GPIO39   //Drv8412�����ź�,����Ч
//#define Drv8412_OTW_B    GpioDataRegs.GPADAT.bit.GPIO11   //Drv8412�����ź�,����Ч

//#define Drv8412_RESET_B  GpioDataRegs.GPBCLEAR.bit.GPIO41=1 //��λDrv8412,����Ч
//#define Drv8412_ENA_B    GpioDataRegs.GPBSET.bit.GPIO41=1   //ʹ��Drv8412
#define Drv8412_RESET_B  GpioDataRegs.GPACLEAR.bit.GPIO25=1 //��λDrv8412,����Ч
#define Drv8412_ENA_B    GpioDataRegs.GPASET.bit.GPIO25=1   //ʹ��Drv8412


//Led
#define LED_FLT_OUT    GpioDataRegs.GPBCLEAR.bit.GPIO34=1   //������ϵ�
//#define LED_KEY_GRN_OFF  GpioDataRegs.GPBSET.bit.GPIO43=1  //����״ָ̬ʾ��
#define LED_KEY_RED_OFF  GpioDataRegs.GPBSET.bit.GPIO44=1  //����״ָ̬ʾ��
//#define LED_KEY_GRN_ON  GpioDataRegs.GPBCLEAR.bit.GPIO43=1  //����״ָ̬ʾ��
#define LED_KEY_RED_ON  GpioDataRegs.GPBCLEAR.bit.GPIO44=1  //����״ָ̬ʾ��
//#define LED_RED_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO34=1  //����״ָ̬ʾ��
#define LED_RED_BLINK   GpioDataRegs.AIOTOGGLE.bit.AIO12=1  //����״ָ̬ʾ��

//#define LED_KEY_GRN_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO43=1  //����״ָ̬ʾ��
#define LED_KEY_RED_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO44=1  //����״ָ̬ʾ��

#define  State_MA_ERR   100  //����״̬
#define  State_MA_STOP  110  //ͣ��״̬
#define  State_MA_IDLE  10   //�ȴ�״̬
#define  State_MA_HAND  11   //����״̬
#define  State_MA_REACH 12
#define  State_MA_FINDZ  0
#define  State_MA_INIT    1   //�ȴ�״̬
#define  State_MA_Rdy     2   //״̬
#define  State_MA_WORK    3   //״̬
#define  State_MA_WORKNCR 4
#define  State_MA_WORKCR  5
#define  State_MA_Wait    6

#define KEY_MOTOR_Y         GpioDataRegs.GPBDAT.bit.GPIO50 //Key1
#define KEY_MOTOR_B         GpioDataRegs.GPBDAT.bit.GPIO51 //Key2
#define KEY_ON            1
#define KEY_OFF           0


#define GREEN 2
#define RED   3
#define OFF   1
//���IO��(onewireʹ��)
//#define OneWire_IO  GpioDataRegs.GPADAT.bit.GPIO22

//------Step6: ȫ�ֺ�������---------//
//#define CPU_CLOCK_SPEED      16.667L  //16.667L  for a 60MHz CPU clock speed
//#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_CLOCK_SPEED) - 9.0L) / 5.0L)
extern void DSP28x_usDelay(unsigned long Count);

//DSP�����ʼ����������
extern void InitFlash(void);
extern void InitI2C_NOINT(void); //IIC��ʼ������(��ѯ��)
extern void InitAdc(void);
extern void Motor_Ctrl_Init(void);//������Ƴ�ʼ��
extern void Motor_Fault_Check(void); //���ϼ�⺯��
extern void Motor_Par_Init(void);//EEPROM������ʼ��
extern void Motor_Ctrl_FSM(void);
extern void Wri_Rst_Par(void);//д�������λ��־,�´��ϵ��ִ�в�����λ
extern void Eeprom_Err_Clear(void);  //������Ϣ����
extern void Eeprom_Err_Wri(void);  //������Ϣд��EEPROM
extern void CAN_Handle(void);
extern void SCI_CMD_Handle(void);
//�Բ�����غ���
extern void Self_Run_Test(void); //����ջ���ת���ƺ���
//------Step7: �ⲿ�ṹ���������---------//
extern Uint16 RamfuncsLoadStart,RamfuncsLoadEnd,RamfuncsRunStart;
extern Uint16 IQmathLoadStart,IQmathLoadEnd,IQmathRunStart;

//��������ṹ��
extern ADConvter adc1,adc2;  //AD�����ṹ��
extern PWMGEN pwm1,pwm2; //PWM����ṹ��
extern QEP qep1,qep2;
extern SPEED_MEAS_QEP speed1,speed2;
//���Ʋ����ṹ��
extern SPTA motor1;
extern MOTOR_CTL MC,MB;//������Ʋ����ṹ��
extern TRANSFER_PARA PA; //���Ʋ����ṹ��PA��ʼ��
//foc������ؽṹ��
extern FOC foc1,foc2; //foc������ؽṹ��
extern PI_CONTROLLER pi_id,pi_id2;
extern PI_CONTROLLER pi_iq,pi_iq2;
extern PI_CONTROLLER pi_spd,pi_spd2;
extern CURVEGEN curve1,curve2;

extern Uint16 Push_Pearl(Uint16 Dir);
extern Uint16 Change_Pearl_Color(void);
extern void LoopMode_Read(void);
extern void LoopMode_Write(Uint16 Data);
extern void Par_Key_Read(void);
extern void Par_Key_Write(void);
#endif // end of PROJ_SETTINGS_H
