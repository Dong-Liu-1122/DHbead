/* =========================================================================
File name:  Ctrl_Para.H
Originator:	Digital Control Systems Group Texas Instruments
Description: ����������ͷ�ļ�
============================================================================
 History: 2017.03.18	Version 1.0
==========================================================================  */
#ifndef Ctrl_Para_H
#define Ctrl_Para_H

//------Step1: �������״̬����---------//

//Idle--;Run--��������;Err--����ͣ��
enum Motor_State {ST_Idle=0,ST_Run=1,ST_Err_Y=2,ST_Err_B=3,ST_RDY=4,wait=5};

//Init_Par--��ʼ������ ;Find_Z--�ұ�������λ(���ڳ�ʼ��װ);
//Run_Test--����ģʽ(������������);Run_Closeloop--�ջ�����(����Ĭ��ģʽ)
enum RunMode {Init_Par=0,Find_Z=1,Run_Test=2,Run_Closeloop=3,Idle=4};


//------Step2:������Ʋ����ṹ��(MC)---------//
typedef struct
{
    //����״̬��־
    Uint16 EnableFlag;  //ʹ�ܱ�־��1��ʾʹ�ܣ�
    enum RunMode RunMode;   //����ģʽ(DIP1);��0��ֻ����λ; "1"������ɫ;
                            //��2�����ڳ�ʼ������--�Լ�����ģʽ
    Uint16 Run_Test_Ena;  //�Բ���ʹ��
    Uint16 Run_Test_Flag;  //�Բ��Ա�־
    enum Motor_State State; //�����ǰ����״̬
    Uint16 Self_Run_Ena; //�����תʹ�ܣ������ϻ�����)
    Uint16 Run_time; //��������ʱ��
    Uint16 RunMotor;    //�������ʹ��

    Uint16 S_ERR;       //���ϱ�־��1��ʾ�й��ϣ�
    Uint16 Err_Value;   //������Ϣ
    Uint16 Err_Wri_Ena;  //������Ϣд��EEPROMʹ��
    Uint16 Err_Clear_Ena;  //������������ʷ����ָ��
    Uint16 Par_Reset_Ena;  //������λʹ��ָ��
    Uint16 Stop_bit;       //��1����Ӧ��λͣ��
    Uint16 Work_Stop_Flag;//"1"��������0��ͣ��
    Uint16 Gain_Switch_Ena;//"1"ִ�������л�

    int32  P_Out_Err;       //λ�û�����ƫ�λ�ù��������ã�
    Uint16 P_Err_Clear;
    int32  Motor_pos_set;   //λ��ָ���������
    Uint16 Pulse_set_Ena;   //λ��ָ��������д��ʹ��
    Uint16 Open_Motor_Dir;//��������ѡ��λ

    _iq     SpeedRef_Find_Z;  //�������λʱ�����ת��IQ(-0.02)��Ӧ48rpm
    _iq     Speedref_Limit;   //���ת��ָ���޷�(1200rpm)
    Uint16  Zero_Find_Ena;    //�������λʹ��(�ϵ��ʹ������λ��
    Uint16  Close_Ena;   //�������λ�ɹ���־��"1"��������λ�ɹ�

    //���ƻ�����
    _iq  Pos_Kp_Work;
    _iq  Pos_Kp_Stop;

    _iq  Spd_Kp_Work;
    _iq  Spd_Ki_Work;
    _iq  Spd_Kp_Stop;
    _iq  Spd_Ki_Stop;

    _iq  Cur_Kp_Work;
    _iq  Cur_Ki_Work;
    _iq  Cur_Kp_Stop;
    _iq  Cur_Ki_Stop;

    //�����޷�����
    _iq  Iq_ref_Limit_Work;  //IQ(0.6)-������ֵ4A,����ָ���޷�(��PA�����趨)
    _iq  Iq_ref_Limit_Stop;
    _iq  Iq_overload_Limit;  //���ر��������޷�ֵ

    //d�ᶨλ����
    _iq   Align_Idref;      //d�ᶨλ����(��PA�����趨)
    _iq   Align_ElecTheta;  //��λ��Ƕ�
    int32 Align_Time;       //��ʼd�ᶨλʱ��=40000*25us=1s
    int32 Align_Ticker;     //��λʱ�����
    int32 Pos_Rst_Ticker;   //���λ�û�λ��ʱ����
    Uint16 pulse_finish;

    Uint16 Zero_Find_Flag;
    Uint16 Pulse_curve_select;

    Uint16 Servo_OFF;//�������־

    //Uint16 RunTime;
   // Uint16 RiseTime;
   // Uint16 DownTime;

}MOTOR_CTL;  //���Ʋ����ṹ��

#define MOTOR_CTL_DEFAULTS {0,Init_Par,0,0,ST_Idle,0,0,0,0,0,\
                            0,0,0,0,0,0,0,\
                            0x0,0x0,0x0,0x0,\
                            _IQ(0.01),_IQ(0.5),0,0,\
                            0x0,0x0,\
                            0,0,0,0,\
                            0x0,0x0,0x0,0x0,\
                            _IQ(0.6),_IQ(0.2),_IQ(0.5),\
                            _IQ(0.3),0,40000,0,0,0,0,0,0\
                            }

extern MOTOR_CTL MC;//������Ʋ����ṹ��


//------Step3:�������PA�����ṹ��(PA)---------//


typedef struct
{
	Uint16 F1;           //��ͷ��ַ
	Uint16 F2;           //װ������: 0
    Uint16 F3;           //A�������ٶ�
    Uint16 F4;           //����汾: 01
    Uint16 F5;           //B�������ٶ�

    Uint16 Motor_Dir;
    Uint16 Address;

    Uint16 Err_num1;             //��ʷ������1:
    Uint16 Err_num2;             //��ʷ������2:
    Uint16 Err_num3;             //��ʷ������3:
    Uint16 Err_num4;             //��ʷ������4:

    Uint16 Hardware_Code;        //
    Uint16 Update_Flag;
    Uint16 F1_Value;
    Uint16 F2_Value;

}TRANSFER_PARA;  //���Ʋ����ṹ��

#define PA_DEFAULTS {5,5,5,5,5,1,0,\
                     0,0,0,0,\
                     0x32,0,0,0}
extern TRANSFER_PARA PA;


//------Step4:CAN�����ṹ�嶨��(CAN)---------//
typedef struct
{
    Uint16 Rec[8];    //��������
    Uint16 Trs[8];    //��������
    Uint16 ID;        //���ν�������ID
    Uint16 ID_last;   //�ϴν�������ID
    Uint16 DLC;       //���������ֽڳ���DLC
    Uint16 Rec_Flag;  //���ձ�־

    Uint16 Rx_Ticker;//CAN���ռ���
    Uint16 Tx_Ticker;//CAN���ͼ���

    Uint16 back1;
    Uint16 back2;
    Uint16 Forward1;
    Uint16 Forward2;
} CAN_Para;
#define CAN_DEFAULTS  {{0},{0},0,0,0,0,0,0,0,0,0,0}
extern CAN_Para CAN;



typedef struct {

	Uint16 HandKeyState;
	Uint16 LastState;
	Uint16 SwitchState;
	Uint16 BlinkLedCounter;
	Uint32 FastBlinkLedCounter;
	Uint16 Cut_Status;
	Uint16 Key_Off_Flag;
	Uint16 Cut_Zero_Find;
}MANUSW;

#define MANUSW_DEFAULTE {0,0,0,0,0,0,0,0}
extern MANUSW ManuSw;
//-----------SCI�����ṹ�嶨��--------------//
typedef struct
{
    Uint16 Tx[4];    //��������
    Uint16 Rx[4];    //��������
    Uint16 Tx_Ticker;//SCI���ͼ���
    Uint16 Rx_Ticker;//SCI���ռ���
    Uint16 Rx_Err_Ticker;//SCI���ռ���
    Uint16 New_sci_cmd; //���յ�SCI�����־

    Uint16 Tx_Ena; //SCI����ʹ��
    Uint16 Tx_Delay; //SCI������ʱ

} SCI_Para;
#define SCI_DEFAULTS {{0},{0},\
                      0,0,0,0,\
                      0,0}
extern SCI_Para SCI;
extern void SCI_Handle(void);  //SCIͨѶ������
//-------------------------------------//

typedef struct {
	Uint16 StartWrite;
	Uint16 Command;
	Uint16 GroupNumber;
	Uint16 Address;
	Uint16 WriteOver;
	Uint32 BlinkCounter;
	Uint32 RedOFFFlag;
	Uint32 First;
	//Uint16 ReadAddress;
	Uint16 AddressValue;    //��ͷ��ַ����ֵ
    Uint16 StopValue;       //����ͣ������
    Uint16 AddressValueSend;//����CAN����ʱʹ��
    Uint16 StartCheck ; //��ʼ���
}HeaderAddress_PARA;

#define HeaderAddress_PARA_DEFAULTE {0,0,0,0,0,0,0,0,0,0,0,0}

//1:1.4:1������
//2:2:1������
#define ChuanDongBi 1

typedef struct{
    Uint16 state;
    Uint16 cmd;
    Uint16 OriginStopFlag;
    Uint16 OriginStopCnt;
    Uint16 cmdFlag;
    Uint16 Position_state;

    int16 DeltaStep1[10];
    int16 DeltaStep2[10];

    int16 DeltaStep3;
    int16 Step1;
    int16 Step1Old;
    int16 Step2;
    int16 Step2Old;
    int16 Step3;
    Uint16 JiaZhu;
    Uint16 All_START;//ȫͷ����ָ��
    Uint16 Address;
    int16 DeltaTime;//�������ʱ��������0-60��
    Uint16 ParaEnable;//�����޸�ʹ��
    Uint16 ParaFlag;
    int16 Step1H;
    int16 Step1L;
    int16 DeltaTimeH;
    int16 DeltaTimeL;


    Uint16 All_START_State;//ȫͷ����״̬
    Uint16 TestPauseEnable;
    Uint16 All_START_Delay;//ȫͷ������ʱ
    Uint16 InitDelay;
    int16 QEPInitValue;
    int16 QEPTargetValue;
    Uint16 loopmode;
    Uint16 loopmode_Flag;
    Uint16 AB_Pulse;
    Uint16 Enable;
    Uint16 ErrorEnable;
    Uint16 color;
    Uint16 ErrCnt;
    Uint16 Direction;
    Uint16 MotorEnableFlag;
    Uint32 HandTZDelay;
    Uint16 HandTZFlag;
    Uint16 ERRTZDelay;
    Uint16 ErrFlag;
    Uint16 CPUEnable;
}TuiZhu_PARA;
#define TuiZhu_PARA_DEFAULTE {0,0,0,0,0,0,\
                              {-45,-40,-30,-20,-10,0,10,20,30,40},\
                              {-25,-20,-15,-10,-5,0,5,10,15,20},\
                              0,-580,-580,-80,-80,660,140,0,1,0,0,0,0,0,0,0,\
                              0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0}
extern TuiZhu_PARA TZ;

typedef struct{
    Uint16 state;
    Uint16 cmd;
    Uint16 cmdstate;
    Uint16 Color_state;
    Uint16 Color_state_old;
    Uint16 cmdFlag;
    Uint16 loopmode;
    Uint16 loopmode_Flag;
    int16 Delta[10];
    Uint16 ErrorEnable;
    Uint16 Delay;//�ϵ���ʱһ��ʱ������EEPROM�е���ɫ״̬���о���
    Uint16 ErrorStartFlag;
    Uint16 ErrCnt;
    Uint16 InitPosData;//��ɫ�ϵ�����ԭ���ı�����ֵ
    Uint16 QEP_HisData;//��һ�εĻ�ɫ���������ֵ

    int16 OFFPosData;//����ǰ�Ļ�ɫλ��
    int16 ONPosData; //����ǰ�Ļ�ɫλ��
    int16 DeltaOFFON;//���������ƫ��ֵ
}HuanSe_PARA;
#define HuanSe_PARA_DEFAULTE {0,0,0,0,0,0,1,0,\
                              {-200,-160,-120,-80,-40,0,40,80,120,160},0,40000,0,0,0,0,\
                              0,0,0}
extern HuanSe_PARA HS;

#endif // end of Ctrl_Para_H
