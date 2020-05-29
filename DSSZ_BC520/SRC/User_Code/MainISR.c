//=================================================================
//  Project Number: step_controller
//  File Name :  MainISR.c
//  Description: �������ʸ������(���ಽ���������)
//  Version    Date     Author     Description
//  1.0    2016.06.30   wangdayu        use
//-----------------------------------------------------------------
// (1)2016.12.03:������Z�ź�--�ߵ�ƽ��Ч,�����ʱ--ZΪ�ߵ�ƽ,
//=================================================================

//1����ͷ�ļ�
#include "F2806x_Device.h"  //DSP28030���趨��ͷ�ļ�
#include "Stepper-Settings.h"
#include "Pos_Regulator.h"   //λ�û�������ͷ�ļ�
#include "GzTest.h"

//2�����жϺ�������
interrupt void PWM1ISR(void); //PWM1�����ж�--40kHz
interrupt void sciaRxFifoIsr(void);
//3������������
static inline void Run_Step_MotorY(void);
static inline void Run_Step_MotorB(void);
static inline void Run_Step_MotorC(void);//����������1
static inline void Run_Step_MotorD(void);//����������2
static inline void Run_Step_MotorE(void);//����������3
//static void Pulse_select(void);//��������ѡ����
static inline void Gain_Switch_Handle_Y(void); //���������л�����
static inline void Gain_Switch_Handle_B(void); //���������л�����

 static inline void Zero_Find_Handle_A(void);//ͨ�õ������λ������Doctor��
 static inline void Zero_Find_Handle_B(void);//ͨ�õ������λ������Doctor��

//4����ȫ�ֱ�������
//volatile �ؼ��ֿ��Ա�֤ÿ�η��ʸñ���ʱ��Ӧ���ڴ��ж�ȡ
const float32 T = 0.001/(ISR_FREQUENCY);//����PWM˫����ISR_FREQUENCY
Uint16 SpeedLoopCnt1 = 0;//�������״̬: 0--��λ�׶Σ�����ջ����ƽ׶�(1,2,3,4)
Uint16 SpeedCalCnt1 = 0;//ת�ټ������
Uint16 SpeedLoopCnt2 = 0;//�������״̬: 0--��λ�׶Σ�����ջ����ƽ׶�(1,2,3,4)
Uint16 SpeedCalCnt2 = 0;//ת�ټ������
//Uint16 SW_Status = 0;//����״̬0-ͣ����1-����
//ʾ������ʾ
Uint16 Dlog_en = 0;
_iq speed_ref[100] = {0};
_iq speed_bck[100] = {0};
_iq ref_id = _IQ(1);
_iq ref_id_a = _IQ(0.8);
int32 bck_old = 0;
Uint16 curve_run_flag[5] = {0,0,0,0,0};
int16 Angle_Par=0;
//5�����ṹ���������
T_POS_REG     tPosReg_x = tPosReg_default;
T_POS_FORWARD tPosFor_x = tPosFor_default;  //λ�û�ǰ��
T_POS_PROP    tPosPro_x = tPosPro_default;  //λ�û�����Ctl_Status

T_POS_REG     tPosReg_y = tPosReg_default;
T_POS_FORWARD tPosFor_y = tPosFor_default;  //λ�û�ǰ��
T_POS_PROP    tPosPro_y = tPosPro_default;  //λ�û�����Ctl_Status

ADConvter adc1 = ADC_DEFAULTS; //AD�����ṹ��
ADConvter adc2 = ADC_DEFAULTS; //AD�����ṹ��
PWMGEN pwm1 = PWMGEN_DEFAULTS; //PWM����ṹ��
PWMGEN pwm2 = PWMGEN_DEFAULTS; //PWM����ṹ��
QEP qep1 = QEP_DEFAULTS;       //QEP����ṹ��
QEP qep2 = QEP_DEFAULTS;       //QEP����ṹ��
SPEED_MEAS_QEP speed1 = SPEED_MEAS_QEP_DEFAULTS; //���ת�ټ���ṹ��
SPEED_MEAS_QEP speed2 = SPEED_MEAS_QEP_DEFAULTS; //���ת�ټ���ṹ��

//#if(CURVEGENER==TRAEZOIDGEN)
CURVEGEN curve1 = CURVEGEN_DEFAULTE;//�������߷��������ջ����
CURVEGEN curve2 = CURVEGEN_DEFAULTE;//�������߷��������ջ����
//#endif
TRANSFER_PARA PA = PA_DEFAULTS; //���Ʋ����ṹ��PA��ʼ��
MOTOR_CTL MC = MOTOR_CTL_DEFAULTS;//ѹ�ŵ�����Ʋ����ṹ��
MOTOR_CTL MB = MOTOR_CTL_DEFAULTS;//�ڸ˵�����Ʋ����ṹ��
//foc������ؽṹ��
FOC foc1 = FOC_DEFAULTS; //foc������ؽṹ��
FOC foc2 = FOC_DEFAULTS; //foc������ؽṹ��

PI_CONTROLLER pi_id  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_iq  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_spd = PI_CONTROLLER_DEFAULTS;

PI_CONTROLLER pi_id2  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_iq2  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_spd2 = PI_CONTROLLER_DEFAULTS;
//һ�׵�ͨ�˲�
LOWPASS_FILTER Spdref_filter1 = LOWPASS_FILTER_DEFAULTS;//ת��ָ���ͨ�˲�
LOWPASS_FILTER Spdref_filter2 = LOWPASS_FILTER_DEFAULTS;//ת��ָ���ͨ�˲�
/********************************************************/

TuiZhu_PARA TZ = TuiZhu_PARA_DEFAULTE;
HuanSe_PARA HS = HuanSe_PARA_DEFAULTE;
extern long cnt;
extern long cnt1;

extern Uint16 B_Mor_Ena;
extern Uint16 Y_Mor_Ena;
//extern Uint16 Motor_Ctl_En;
extern _iq BM_Open_Id;
extern void Open_Current_Loop(Uint16 Dir,Uint16 Motor_Index);
extern void BG_Control(void);
void Motor_Ctrl_Init(void)
{
#if(IIC_Ena)
    InitI2C_NOINT(); //I2C��ʼ��--DSP2803x_I2C.c
    if((DIP1==0)&&(DIP2==0)) //��ʼ������
    {
    	Wri_Rst_Par();  //д���ʼ���Ʋ�����־
    }
    Motor_Par_Init();  //�ϵ��,��EEPROM�ж�����Ʋ���
    PA.F1_Value = PA.F1;
    PA.F2_Value = PA.F2;
#endif
// step1: Initialize ADC module
    InitAdc();         //ADC������λ��׼У��(TI-OTP)
    ADC_INIT_MACRO()   //ADCģ���ʼ��

	//��β������ܿ��ϵ�ADC�������ȶ���״̬
	ADC_OFFSET_Reset_MACRO(adc1)
	ADC_OFFSET_Reset_MACRO(adc1)
	ADC_OFFSET_Reset_MACRO(adc1)

	ADC_OFFSET_Reset_MACRO(adc2)
	ADC_OFFSET_Reset_MACRO(adc2)
	ADC_OFFSET_Reset_MACRO(adc2)

	//---------ADC������׼���ϼ��------------//
	adc1.ADC_Index=1;
	ADC_OFFSET_INIT_MACRO(adc1)     //ADC������λ��׼У��,AD��ƫ
	ADC_OFFSET_CHEKE_MACRO(adc1)    //ADC������׼���ϼ��

	adc2.ADC_Index=2;
	ADC_OFFSET_INIT_MACRO(adc2)     //ADC������λ��׼У��,AD��ƫ
    ADC_OFFSET_CHEKE_MACRO(adc2)    //ADC������׼���ϼ��
    /*
	if(MC.S_ERR==0)
	{
		if((adc1.ADC_OFFSETA_ERR==1)&&(adc1.ADC_OFFSETB_ERR==1))
		{
			MC.Err_Value = 10;//������--f10(A,B��ADC������׼����)
			MC.S_ERR=1;//�ù��ϱ�־
			MC.Err_Wri_Ena=1;
		}
		else if(adc1.ADC_OFFSETA_ERR==1)
		{
			MC.Err_Value = 8;//������--f08(A��ADC������׼����)
			MC.S_ERR=1;//�ù��ϱ�־
			MC.Err_Wri_Ena=1;
		}
		else if(adc1.ADC_OFFSETB_ERR==1)
		{
			MC.Err_Value = 9;//������--f09(B��ADC������׼����)
			MC.S_ERR=1;//�ù��ϱ�־
			MC.Err_Wri_Ena=1;
		}

		if((adc2.ADC_OFFSETA_ERR==1)&&(adc2.ADC_OFFSETB_ERR==1))
		{
			MB.Err_Value = 10;//������--f10(A,B��ADC������׼����)
			MC.S_ERR=2;//�ù��ϱ�־
			MB.Err_Wri_Ena=1;
		}
		else if(adc2.ADC_OFFSETA_ERR==1)
		{
			MB.Err_Value = 8;//������--f08(A��ADC������׼����)
			MC.S_ERR=2;//�ù��ϱ�־
			MB.Err_Wri_Ena=1;
		}
		else if(adc2.ADC_OFFSETB_ERR==1)
		{
			MB.Err_Value = 9;//������--f09(B��ADC������׼����)
			MC.S_ERR=2;//�ù��ϱ�־
			MB.Err_Wri_Ena=1;
		}
	}
*/
//step2: Initialize PWM module
	pwm1.PeriodMax = SYSTEM_FREQUENCY*1000000*T/2; //PWM�������ڣ�1500--20kHz��
    PWM_INIT_MACRO(pwm1) //ePWM��ʼ��
    PWM1_OFF_MACRO()      //�ر�mosfet

    pwm2.PeriodMax = SYSTEM_FREQUENCY*1000000*T/2; //PWM�������ڣ�1500--20kHz��
	PWM2_INIT_MACRO(pwm2) //ePWM��ʼ��
	PWM2_OFF_MACRO()      //�ر�mosfet

    //LineControlInit();//drv8880����оƬPWM�ܽ�����

    //�����ж�������
    EALLOW;
    PieVectTable.EPWM1_INT = &PWM1ISR;//(������PWM1 40K�����жϣ��㹻���ˣ��Ͳ��������жϣ�����ʹ��˫����)
    //PieVectTable.EPWM6_INT = &PWM6CAL;//����ֻʹ����PWM�Ķ�ʱ����LineControlInit
    EDIS;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;//��ӦPIE3.1--EPWM1_INT
    PieCtrlRegs.PIEIER3.bit.INTx6 = 1;//��ӦPIE3.6--EPWM6_INT
    IER |= M_INT3; // Enable CPU INT3 for EPWM1_INT

//step3: Initialize QEP module
    qep1.LineEncoder = EncoderLine;//����������=1000
    qep1.MechScaler = _IQ30(0.25/qep1.LineEncoder);
    qep1.PolePairs  = Pole_Pairs;
    qep1.CalibratedAngle = 0;
    QEP_INIT_MACRO(qep1)

    qep2.LineEncoder = EncoderLine;//����������=1000
	qep2.MechScaler = _IQ30(0.25/qep1.LineEncoder);
	qep2.PolePairs  = Pole_Pairs;
	qep2.CalibratedAngle = 0;
	QEP_INIT_MACRO(qep2)

	Angle_Par= POS_SET_MAX/2/200;//512��-> Angle_Par=128/25

//step4: Initialize the Speed module for QEP based speed calculation
    speed1.K1 = _IQ21( 1/(BASE_FREQ*(T*SpeedLoopPrescaler*SpeedCalPrescaler)));//���õ�Ƕȼ���
    speed1.K2 = _IQ(1/(1+(SpeedLoopPrescaler*T*SpeedCalPrescaler)*(2*PI)*Speedfdb_filter_Freq));
    speed1.K3 = _IQ(1)-speed1.K2;
    speed1.BaseRpm = 60*(BASE_FREQ/Pole_Pairs);
    speed1.RpmScaler = _IQ(1.0/speed1.BaseRpm);//ת�ٱ任ϵ��--1/2400

    speed2.K1 = _IQ21( 1/(BASE_FREQ*(T*SpeedLoopPrescaler*SpeedCalPrescaler)));//���õ�Ƕȼ���
	speed2.K2 = _IQ(1/(1+(SpeedLoopPrescaler*T*SpeedCalPrescaler)*(2*PI)*Speedfdb_filter_Freq));
	speed2.K3 = _IQ(1)-speed2.K2;
	speed2.BaseRpm = 60*(BASE_FREQ/Pole_Pairs);
	speed2.RpmScaler = _IQ(1.0/speed2.BaseRpm);//ת�ٱ任ϵ��--1/2400

//  step5: Initialize CAP1 module
#if(ECAP_Ena)
    InitECap(); //���ڲ�������AB����
#endif

//step6: ͨѶ�������ʼ����CAN,SCI,SPI��
#if(CAN_Ena)
    InitECana();
#endif

//step5: �����ź��˲�������

    //ת��ָ���˲�---��ת�ٻ�ִ��
    Spdref_filter1.K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Speedref_filter_Freq));
    Spdref_filter1.K3 = _IQ(1)-Spdref_filter1.K2;

    Spdref_filter2.K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Speedref_filter_Freq));
    Spdref_filter2.K3 = _IQ(1)-Spdref_filter2.K2;

    //����ָ���˲�---��ת�ٻ�ִ��
    foc1.Iqref_filter_K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Iqref_filter_Freq));  //�˲���ֹƵ��1000Hz
    foc1.Iqref_filter_K3 = _IQ(1)-foc1.Iqref_filter_K2;

    foc2.Iqref_filter_K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Iqref_filter_Freq));  //�˲���ֹƵ��1000Hz
    foc2.Iqref_filter_K3 = _IQ(1)-foc2.Iqref_filter_K2;

    //���������˲�---�ڵ�������ִ��
    foc1.Idqfdb_filter_K2 = _IQ(1/(1+(T)*(2*PI)*Idqfdb_filter_Freq));  //�˲���ֹƵ��1000Hz
    foc1.Idqfdb_filter_K3 = _IQ(1)-foc1.Idqfdb_filter_K2;

    foc2.Idqfdb_filter_K2 = _IQ(1/(1+(T)*(2*PI)*Idqfdb_filter_Freq));  //�˲���ֹƵ��1000Hz
    foc2.Idqfdb_filter_K3 = _IQ(1)-foc2.Idqfdb_filter_K2;

//step8: ���Ʋ�����ʼ��

    //������PI������#57 ����1ŷ�����  1.2mH��
    //--------��Ƶ������8PWM-----------//
    MC.Cur_Kp_Work = _IQ(2);//2
    MC.Cur_Ki_Work =_IQ(0.02);// _IQ(T*800);//2000
    MC.Cur_Kp_Stop = _IQ(1);//2
    MC.Cur_Ki_Stop = _IQ(0.02);//_IQ(T*800);//400

    MB.Cur_Kp_Work = _IQ(2);//2
    MB.Cur_Ki_Work = _IQ(0.02);//2000
    MB.Cur_Kp_Stop = _IQ(2);//2
    MB.Cur_Ki_Stop = _IQ(0.02);//400

    pi_id.Kp = MC.Cur_Kp_Stop;
    pi_id.Ki = MC.Cur_Ki_Stop;
    pi_id.Umax = _IQ(0.707); //�����ѹ�޷�ֵ_IQ(0.707)
    pi_id.Umin = _IQ(-0.707);//�����ѹ�޷�ֵ_IQ(-0.707)

    pi_iq.Kp = pi_id.Kp;
    pi_iq.Ki = pi_id.Ki;
    pi_iq.Umax = _IQ(0.707); //�����ѹ�޷�ֵ0.707Bai_Gan
    pi_iq.Umin = _IQ(-0.707);//�����ѹ�޷�ֵ

    pi_id2.Kp = MB.Cur_Kp_Stop;
    pi_id2.Ki = MB.Cur_Ki_Stop;
    pi_id2.Umax = _IQ(0.7); //�����ѹ�޷�ֵ_IQ(0.707)
    pi_id2.Umin = _IQ(-0.7);//�����ѹ�޷�ֵ_IQ(-0.707)

    pi_iq2.Kp = pi_id2.Kp;
    pi_iq2.Ki = pi_id2.Ki;
    pi_iq2.Umax = _IQ(0.7); //�����ѹ�޷�ֵ0.707Bai_Gan
    pi_iq2.Umin = _IQ(-0.7);//�����ѹ�޷�ֵ

    //ת�ٻ�PI����
    MC.Spd_Kp_Stop = _IQ(1.6);//1.6
    MC.Spd_Ki_Stop =  _IQ(0.02);//_IQ(SpeedLoopPrescaler*T*50);//60
    //MC.Spd_Kp_Work = _IQ(2.8);//1
    MC.Spd_Kp_Work = _IQ(2.0);//1
    MC.Spd_Ki_Work = _IQ(0.02);//_IQ(SpeedLoopPrescaler*T*50);//60

    MB.Spd_Kp_Stop = _IQ(3);//0.8
    MB.Spd_Ki_Stop = _IQ(0.03);//60
    //MB.Spd_Kp_Stop = _IQ(4);//0.8
    //MB.Spd_Ki_Stop = _IQ(0.04);//60
    MB.Spd_Kp_Work = _IQ(2.0);//1
    MB.Spd_Ki_Work = _IQ(0.02);//60

    pi_spd.Kp = MC.Spd_Kp_Stop;
    pi_spd.Ki = MC.Spd_Ki_Stop;

    pi_spd2.Kp = MB.Spd_Kp_Work;
    pi_spd2.Ki = MB.Spd_Ki_Work;

    //λ�û�����ͨ����������
    MC.Pos_Kp_Stop = 25; //ͣ��25
    MC.Pos_Kp_Work = 25; //����25
    tPosReg_x.iEncoderLine = EncoderLine * 4;
    tPosPro_x.uKp_Pos = MC.Pos_Kp_Stop; //λ�û���������PA3=24(0~100) PA.Pos_Kp_Par=25(��ʤ)
    tPosPro_x.uEnum = 1;
    tPosPro_x.uEden = 1;
    tPosPro_x.iPosCount_Err = 0; //λ�ü�����0xFFFFFFFF
    tPosPro_x.tErrCounter.lErrCount_Out = 0; //λ�ø���ƫ��

    MB.Pos_Kp_Stop = 10; //ͣ��25
    //MB.Pos_Kp_Stop = 70; //ͣ���������ԭ��������ͬ 2019.04.19
    MB.Pos_Kp_Work = 25; //����25
    tPosReg_y.iEncoderLine = EncoderLine * 4;
    tPosPro_y.uKp_Pos = MB.Pos_Kp_Work; //λ�û���������PA3=24(0~100) PA.Pos_Kp_Par=25(��ʤ)
    tPosPro_y.uEnum = 1;
    tPosPro_y.uEden = 1;
    tPosPro_y.iPosCount_Err = 0; //λ�ü�����0xFFFFFFFF
    tPosPro_y.tErrCounter.lErrCount_Out = 0; //λ�ø���ƫ��

    //�����޷�ֵ����--_IQ(0.1)��Ӧ�������ֵ0.9A   0.75  0.4
    MC.Iq_ref_Limit_Work = _IQ(0.8); //����ʱ�����޷� _IQ(0.3)--��ֵ2.7A
    MC.Iq_ref_Limit_Stop = _IQ(0.8); //ֹͣʱ�����޷� _IQ(0.1)--��ֵ0.9A
    MC.Iq_overload_Limit = _IQmpy(_IQ(0.8),MC.Iq_ref_Limit_Stop);//���������޷�

    MB.Iq_ref_Limit_Work = _IQ(0.8); //����ʱ�����޷� _IQ(0.3)--��ֵ2.7A
    MB.Iq_ref_Limit_Stop = _IQ(0.8); //ֹͣʱ�����޷� _IQ(0.1)--��ֵ0.9A
    MB.Iq_overload_Limit = _IQmpy(_IQ(0.9),MB.Iq_ref_Limit_Stop);//���������޷�
    // BC520--IQ(1)--3A
    MC.Align_Idref = _IQ(0.7); //��Ӧ�������ֵ2.1A
    pi_spd.Umax =  MC.Iq_ref_Limit_Stop; //_IQ(0.4)��Ӧ�������ֵ2A
    pi_spd.Umin = -MC.Iq_ref_Limit_Stop;

    MB.Align_Idref = _IQ(0.7); //��Ӧ�������ֵ2.1A
    pi_spd2.Umax =  MB.Iq_ref_Limit_Stop; //_IQ(0.4)��Ӧ�������ֵ2A
    pi_spd2.Umin = -MB.Iq_ref_Limit_Stop;

    //�ϵ綨λ��������
    MC.Align_Time = Ds_Align_Time;  //d�ᶨλʱ��2s
    MC.Align_Ticker = 0;
    MC.Speedref_Limit = _IQ(0.5);//���ת���޷�(2400*0.08=192rpm)0.4
    //MC.SpeedRef_Find_Z = _IQ(0.01);//����λʱ���ת��(�Բ���ģʽ����ԭ��ת��)96rpm
    MC.SpeedRef_Find_Z = _IQ(0.05);
    MC.Work_Stop_Flag = 0;   //��ʼΪͣ��״̬
    MC.Gain_Switch_Ena = 1;  //"1"ִ�������л�
    //MC.Pulse_curve_select=0;
    MC.P_Err_Clear=0;
    MC.Pos_Rst_Ticker=0;

    MB.Align_Time = Ds_Align_Time;  //d�ᶨλʱ��2s
    MB.Align_Ticker = 0;
    MB.Speedref_Limit = _IQ(0.4);//���ת���޷�(2400*0.08=192rpm)
    //MB.SpeedRef_Find_Z = _IQ(0.015);//����λʱ���ת��(�Բ���ģʽ����ԭ��ת��)96rpm
    MB.SpeedRef_Find_Z = _IQ(0.05);
    MB.Work_Stop_Flag = 0;   //��ʼΪͣ��״̬
    MB.Gain_Switch_Ena = 0;  //"1"ִ�������л�
   // MB.Pulse_curve_select=0;
    MB.P_Err_Clear=0;
    MB.Pos_Rst_Ticker=0;


    //YJ.Work_Pul_Out=YJ.Work_High*5/2;
    //MC.Motor_pos_set=YJ.Work_High*5/2;
   //BG.Work_Pul_Out=BG.Pos_Set*16;

    //��������


    curve1.IsrFrequency = ISR_FREQUENCY/SpeedLoopPrescaler;
    curve1.Kt = _IQ30(0.001/curve1.IsrFrequency);
    /*curve1.RunTime  = 100;
    curve1.RiseTime = 20;
    curve1.DownTime = 50; */
    curve1.RunTime  = 100;
    curve1.RiseTime = 20;
    curve1.DownTime = 30;
    curve1.GenFlg = 2;
    CurveInit(&curve1);//���߷�������ʼ��
    if( (curve1.ErrFlg == 1)&&(MC.S_ERR==0) )
    {
    	MC.S_ERR=1;//�ù��ϱ�����־
    	MC.Err_Value = 12;// ������
    }
#if(Run_Mode_MotorB==Close_Loop)
    curve2.IsrFrequency = ISR_FREQUENCY/SpeedLoopPrescaler;
#else
    curve2.IsrFrequency = ISR_FREQUENCY/SpeedLoopPrescaler;
#endif
    curve2.Kt = _IQ30(0.001/curve2.IsrFrequency);
    curve2.RunTime  = 50;
    curve2.RiseTime = 10;
    curve2.DownTime = 30;
    curve2.GenFlg = 2;
    CurveInit(&curve2);//���߷�������ʼ��
    if( (curve2.ErrFlg == 1)&&(MC.S_ERR==0) )
    {
	  MB.S_ERR=1;//�ù��ϱ�����־
	  MB.Err_Value = 12;// ������
    }


//step10: ���ⲿDIP1,2�趨����ģʽ

    #if(Run_Mode==Open_Self_Run)
    {
        MC.RunMode = Init_Par;  //"2"--����ģʽ(������������)
        MC.Zero_Find_Ena = 0; //��ʹ������λ
        MC.EnableFlag = 1;    //���ϻ���װ����EnableFlag=1�����������ܹ���
    }
    #endif
    if(HS.loopmode == 0)
    {
        MC.RunMode = Init_Par;  //"2"--����ģʽ(������������)
        MC.Zero_Find_Ena = 0; //��ʹ������λ
        MC.EnableFlag = 1;    //���ϻ���װ����EnableFlag=1�����������ܹ���
    }
    if(TZ.loopmode == 0)
    {
        MB.RunMode = Init_Par;  //"2"--����ģʽ(������������)
        MB.Zero_Find_Ena = 0; //��ʹ������λ
        MB.EnableFlag = 1;    //���ϻ���װ����EnableFlag=1�����������ܹ���
    }

    if((DIP1==1)&&(DIP2==1))////DIP1=OFF DIP2=OFF
    {
        MC.RunMode = Run_Closeloop;//������λ
        MB.RunMode = Run_Closeloop;//������λ

        if(HS.loopmode == 1)
        {
            MC.Zero_Find_Ena = 1;//�ϵ�����
        }

        MC.EnableFlag = 1; //���ʹ��
        MB.EnableFlag = 1; //���ʹ��

        TZ.Address = 1;//��װ��
    }
    if((DIP1==1)&&(DIP2==0))//DIP1=OFF DIP2=ON
    {
        MC.RunMode = Run_Test;//�ջ��Բ�
        MB.RunMode = Run_Test;//�ջ��Բ�

        MC.Zero_Find_Ena = 1;//�ϵ�����
        MB.Zero_Find_Ena = 1;//�ϵ�����

        MC.EnableFlag = 1; //���ʹ��
        MB.EnableFlag = 1; //���ʹ��
    }
    if((DIP1==0)&&(DIP2==1))//DIP1=ON DIP2=OFF
    {
        /*
        MC.RunMode = Find_Z; //�ϵ�����λ(���ڵ����װ)
        MB.RunMode = Find_Z; //�ϵ�����λ(���ڵ����װ)

        MC.Zero_Find_Ena = 1;//�ϵ�����
	    MB.Zero_Find_Ena = 1;//�ϵ�����

	    MC.EnableFlag = 1; //���ʹ��
	    MB.EnableFlag = 1; //���ʹ��
         */

        MC.RunMode = Run_Closeloop;//������λ
        MB.RunMode = Run_Closeloop;//������λ

        if(HS.loopmode == 1)
        {
            MC.Zero_Find_Ena = 1;//�ϵ�����
        }

        MC.EnableFlag = 1; //���ʹ��
        MB.EnableFlag = 1; //���ʹ��

        TZ.Address = 2;//��װ��
    }
    if((DIP1==0)&&(DIP2==0))//DIP1=ON DIP2=OFF
    {
        MC.RunMode = Init_Par;//��ʼ�����Ʋ���
        MB.RunMode = Init_Par;//��ʼ�����Ʋ���

	    MC.EnableFlag = 0; //���ʹ��
	    MB.EnableFlag = 0; //���ʹ��
    }
} //end of Motor_Ctrl_Init


//---PWM1�ж�(40K�����ж�)---//
#pragma CODE_SECTION(PWM1ISR, "ramfuncs");
interrupt void PWM1ISR(void)
{
	if(MC.EnableFlag==1)//
    {
	   Run_Step_MotorY();//�����������
    }
    if(MB.EnableFlag==1)//
    {
       Run_Step_MotorB();//�����������
    }
    if(motorC.EnableFlag == 1)
    {
        Run_Step_MotorC();//����������1
    }
    if(motorD.EnableFlag == 1)
    {
        Run_Step_MotorD();//����������2
    }
    if(motorE.EnableFlag == 1)
    {
        Run_Step_MotorE();//���濪�����
    }

    //PWM1�жϴ���
    EPwm1Regs.ETCLR.bit.INT = 1;//���жϱ�־
    PieCtrlRegs.PIEACK.all  = PIEACK_GROUP3;//���ж�Ӧ���־
} //end of Main_ISR

/****************************************************
      ����⻷���Ƽ��㺯��
****************************************************/
#pragma CODE_SECTION(Run_Step_MotorY, "ramfuncs");
static inline void Run_Step_MotorY(void)
{
    if(HS.loopmode == 1)
    {
        //------------------------ѹ��-------------------------------------
        if(MC.Zero_Find_Ena==1)
        {
            Zero_Find_Handle_A();
        }

        //------�⻷��������-------//
        if(MC.Align_Ticker < MC.Align_Time)//1s
        {   //�����ʼ��λ�׶�
            MC.Align_Ticker++;
            SpeedLoopCnt1=0;//�⻷�������̼���ֵ����
        }
        else
        {   //����ջ����ƽ׶�(1,2,3,4)
            if(SpeedLoopCnt1>=SpeedLoopPrescaler)
                SpeedLoopCnt1=1;
            else
                SpeedLoopCnt1++;  //�⻷�������̼���ֵ
        }

        //"0":��ʼ��λ; ��1~2��:ʸ���ջ�����
        switch(SpeedLoopCnt1)
        {
        //--------------------------------------------//
        case 0://��ʼ��λ�л�

            //d�����ָ��
            if(pi_id.Ref <= MC.Align_Idref)
            {
                pi_id.Ref += _IQ(0.00004);//��ʼ��λʱ��������б������
                if(pi_id.Ref >= MC.Align_Idref)
                    pi_id.Ref = MC.Align_Idref;
            }

            pi_iq.Ref = 0; //q�����ָ��

            qep1.CalibratedAngle = (EQep1Regs.QPOSCNT > qep1.LineEncoder*2)? \
                    (EQep1Regs.QPOSCNT - qep1.LineEncoder*4):EQep1Regs.QPOSCNT;
            //------------------------------------------------------------------------------------

            //�ϵ��������ζ�λ���ȶ�λ��-90�ȣ��ٶ�λ��0��
            if(MC.Align_Ticker<= (MC.Align_Time>>1))
            {
                MC.Align_ElecTheta = _IQ(-0.25);//d�ᶨλʱ,�趨��Ƕ�Ϊ-90
            }
            else if( MC.Align_Ticker >(MC.Align_Time>>1) && (MC.Align_Ticker < MC.Align_Time) )
            {
                MC.Align_ElecTheta += _IQ(0.00001);
                if( MC.Align_ElecTheta>=0)
                    MC.Align_ElecTheta = 0;    //�ٶ�λ��0��
                qep1.ElecTheta = 0;
            }

            if(MC.Align_Ticker == MC.Align_Time)//����ȷ������;
            {
                qep1.IndexSyncFlag = 0x00;
                //Ϊ�˷�ֹDSP��λ����λ��־��ʼֵ����ȷ��������ǿ��������λ��־λIEL
                EQep1Regs.QCLR.bit.IEL = 1; //Clear interrupt flag
            }
            else
            {
                qep1.IndexSyncFlag = 0x0F;
            }//��λ�����У���ֹ����ԭ��λ�ã�����Ϊ�ҵ�ԭ��

            break;
            //-----------------------------------------------------------//
        case 1:
            if(curve1.Cur_mode==0)
            {
                if(curve1.GenFlg==2)
                {
                    if(MC.Pulse_set_Ena==1) //idle״̬�²�����д���µ�����ָ��
                    {
                        curve1.DeltaDistance = MC.Motor_pos_set;
                        MC.Pulse_set_Ena=0;
                        Dlog_en = 1;
                    }
                }
                CurveGen(&curve1);

            }
            else
            {
                curve1.OutputPulseErr = curve1.PositionPulse - curve1.PositionPulseOld;
                curve1.PositionPulseOld = curve1.PositionPulse;

            }

            tPosPro_x.iPosCount_Err += curve1.OutputPulseErr;//��������

            //��ȡ����������ֵ
            tPosReg_x.lFeedBack = EQep1Regs.QPOSCNT;

            if(MC.Servo_OFF == 1)
            {
                tPosPro_x.uCLR = 1; //λ�ü���������
            }
            else
            {
                tPosPro_x.uCLR = MC.P_Err_Clear; //λ�ü���������
            }

            //λ�û�����1--����ʱ�䣨66*16.7ns=1.1us)
            POS_PD_X1Macro()  //λ�û�����1(����ƫ�����)

            //��ȡλ�ø���ƫ�λ�ó����ʹ�ã�
            MC.P_Out_Err = tPosPro_x.tErrCounter.lErrCount_Out;

            //���ݹ���״̬���л�����ָ���޷�ֵ����������
            Gain_Switch_Handle_Y();
            break;
            //-------------------------------------------------------//
        case 2:
            //λ�û�����2--����ʱ�䣨107*16.7ns=1.78us)
            POS_PD_X2Macro()   //λ�û�����2--/*����������(������3us)*/
            break;
            //--------------------------------------------------------//
        case 3:  //ת�ٷ�������--����ʱ�䣨109*16.7=2.01us��
            POS_PD_X3Macro()   //λ�û�����3--/*λ�û�FAL����*/

            //���ٶ�ָ������޷�
            if(MC.Pos_Rst_Ticker<10)//���Ƶ���ϵ��λ���̵ĵ��ת��2400*0.01=24rpm
            {
                MC.Pos_Rst_Ticker++;
                Spdref_filter1.input = _IQsat( ((int32)tPosReg_x.lSpd_Ref),\
                                               _IQ(0.1),-_IQ(0.1));
            }
            else
            {
                Spdref_filter1.input = _IQsat( ((int32)tPosReg_x.lSpd_Ref),\
                                               MC.Speedref_Limit,-MC.Speedref_Limit);
            }
            //���ٶ�ָ���ͨ�˲�
            LOWPASS_FILTER_MACRO(Spdref_filter1)

            //ת�ٻ�ָ��ѡ��
            if(MC.Zero_Find_Ena==0)
            {
                pi_spd.Ref = Spdref_filter1.output;
            } //�˲����ת��ָ��
            else if(MC.Zero_Find_Ena==1)//�ҵ����λת��ָ��=60rpm
            {
                if(PA.Motor_Dir == 0)
                {
                    pi_spd.Ref = -MC.SpeedRef_Find_Z;
                }
                else
                {
                    pi_spd.Ref = MC.SpeedRef_Find_Z;
                }
            }
            //pi_spd.Ref = MC.SpeedRef_Find_Z;
            break;
            //-------------------------------------------------------//
        case 4://�ٶȻ�����--����ʱ��(73~130*16.7ns=2.54us)

            //(1)ת�ټ������=60*ת�ټ���Ƶ��/(����������*4)
            //(2)ת�ټ������=60*1000/(1000*4)=15rpm (1000�߱�������

            //���ת�ټ���
            SpeedCalCnt1++;
            if(SpeedCalCnt1>=SpeedCalPrescaler) //���ת�ټ���
            {
                SpeedCalCnt1 = 0;
                speed1.ElecTheta = qep1.ElecTheta;//��ȡ��Ƕȼ���ת��
                SPEED_FR_MACRO(speed1)  //ת�ټ���(M��)
                pi_spd.Fdb = speed1.Speed;//ת�ٷ���
            }

            //ת�ٻ�PI����
            PI_MACRO(pi_spd)

            //q�����ָ��--ת�ٻ�����ĵ���ָ���˲�
            pi_iq.Ref = _IQmpy(foc1.Iqref_filter_K2,pi_iq.Ref)+\
            _IQmpy(foc1.Iqref_filter_K3,pi_spd.Out);

            //d�����ָ��
            /*
                if(_IQabs(pi_spd.Ref)>_IQ(0.2))
                {
                    pi_id.Ref=-_IQdiv(_IQabs(pi_spd.Ref)-_IQ(0.2),ref_id);

                }
                else

                if(pi_spd.Ref>_IQ(0.2))
                {
                    pi_id.Ref=-_IQdiv(_IQabs(pi_spd.Ref)-_IQ(0.2),ref_id_a);

                }
                else if(pi_spd.Ref<_IQ(-0.2))
                {
                    pi_id.Ref=-_IQdiv(_IQabs(pi_spd.Ref)-_IQ(0.2),ref_id);

                }
                else*/
            pi_id.Ref = 0;//; //��������-_IQ(0.05)

            if(MC.Servo_OFF == 1)
            {
                pi_spd.ui=0;
                pi_spd.up=0;
                pi_spd.i1=0;
                pi_spd.v1=0;
                pi_spd.Out=0;

                pi_iq.Ref=0;
                pi_id.Ref=0;
            }
            break;
            //--------------------------------------------------//
        default: break;
        }//end of �⻷����
    }


        //----����������----//
        //step1: Park�任�Ƕȼ���(����ʱ��=103*16.667ns=1.716us)
    if(HS.loopmode == 1)
    {
        //���㷴���������Ƕ�(55*16.7=0.92us)
        QEP1_MACRO(qep1)  //���ü��������ֵ��λeQEP������,����Ƿ��I����S
                       if(SpeedLoopCnt1==0)//��Ӧd�ᶨλ״̬
                       {foc1.Angle = MC.Align_ElecTheta;} //��λ��Ƕ�
                       else
                       {foc1.Angle = qep1.ElecTheta;} //(���ñ�����������Ƕȣ�
    }
    else
    {
        foc1.Angle -= _IQ(0.0005);
        if(foc1.Angle > _IQ(1))
           foc1.Angle  = foc1.Angle - _IQ(1);
        if(foc1.Angle < _IQ(-1))
           foc1.Angle  = foc1.Angle + _IQ(1);
        pi_id.Ref += _IQ(0.005);
        if(pi_id.Ref > _IQ(0.5))
        {
            pi_id.Ref = _IQ(0.5);
        }
        pi_iq.Ref = 0;
    }


        //}
        //����任��sin,cos����
        foc1.Sine   = _IQsinPU(foc1.Angle);
        foc1.Cosine = _IQcosPU(foc1.Angle);

        //step2: ADC��������(����ʱ��=40*16.667ns=0.667us)
        //�������adc1.Ia,adc1.Ib,adc1.Udc
        ADC1_READ_MACRO(adc1)
        //**********�ɼ��۲�����************//
        /*if(Dlog_en == 1)
        {
            curve_run_flag[1]++;
            if(curve_run_flag[1]>=40)//40--1ms
            {
                curve_run_flag[1]=0;

                //if(curve_run_flag[2]>=100)
                    //curve_run_flag[2]=0;

                if(curve_run_flag[2]<100)//100������
                {
                    speed_ref[curve_run_flag[2]]=curve1.PositionPulse;//pi_spd.Fdb;//pi_spd.Ref//MC.P_Out_Err//motor1.Pulse_Out//curve1.PositionPulse
                    if(EQep1Regs.QPOSCNT>2000) speed_bck[curve_run_flag[2]]=EQep1Regs.QPOSCNT-4000;
                    else speed_bck[curve_run_flag[2]]=EQep1Regs.QPOSCNT;//pi_spd.Fdb//EQep1Regs.QPOSCNT
                    //speed_bck[curve_run_flag[2]]=pi_spd.Ref;
                    curve_run_flag[2]++;
                }
                else
                {
                    Dlog_en = 0;

                }
            }
        }
        else
        {
            curve_run_flag[1] = 0;
            curve_run_flag[2] = 0;
        }*/


        //**********�ɼ��۲�����end************//

        //**********����������������(������)************//
        static Uint16 OC_Delay_Ia=0;
        static Uint16 OC_Delay_Ib=0;

        //Ia�������
        if(_IQ15abs(adc1.Ia)>=Over_Current_Limit)
        {
            OC_Delay_Ia++;
            if(OC_Delay_Ia>OC_Delay_Set)
            {
                PWM1_OFF_MACRO()
                                OC_Delay_Ia=OC_Delay_Set;
                if(MC.S_ERR == 0)
                {
                    MC.Err_Value = 2;//������--f02(A�����)
                    MC.S_ERR=1;//�ù��ϱ�־
                    MC.Err_Wri_Ena=1; //ʹ�ܹ�����д��EEPROM
                }
            }
        }
        else
        {
            OC_Delay_Ia=0;
        }

        //Ib�������
        if(_IQ15abs(adc1.Ib)>=Over_Current_Limit) //������������
        {
            OC_Delay_Ib++;
            if(OC_Delay_Ib>OC_Delay_Set)
            {
                PWM1_OFF_MACRO()
                                OC_Delay_Ib=OC_Delay_Set;
                if(MC.S_ERR == 0)
                {
                    MC.Err_Value = 2;//������--f02(B�����)
                    MC.S_ERR=1;//�ù��ϱ�־
                    MC.Err_Wri_Ena=1; //ʹ�ܹ�����д��EEPROM
                }
            }
        }
        else
        {
            OC_Delay_Ib=0;
        }

        //step3: PARK�任(����ʱ��=36*16.667ns=0.6us)
        foc1.I_Alpha = _IQ15toIQ(adc1.Ia);//Ia_IQ24;
        foc1.I_Beta  = _IQ15toIQ(adc1.Ib);//Ib_IQ24;
        PARK_MACRO(foc1) //park�任(���ಽ���������ʱʹ�ã�

        //step4: d������PI����(����ʱ��=49*16.667ns=0.817us)
        pi_id.Fdb = _IQmpy(foc1.Idqfdb_filter_K2,pi_id.Fdb)+\
        _IQmpy(foc1.Idqfdb_filter_K3,foc1.I_Ds);

        PI_MACRO(pi_id) //d��PI����

        /*********Uq_Limit����*******/
        register _iq Uq_Limit;
        Uq_Limit = _IQsqrt( _IQ(0.95) - _IQmpy(pi_id.Out,pi_id.Out) );
        pi_iq.Umax =  Uq_Limit;
        pi_iq.Umin = -Uq_Limit;


        //step5: q������PI����(����ʱ��=49*16.667ns=0.817us)
        pi_iq.Fdb = _IQmpy(foc1.Idqfdb_filter_K2,pi_iq.Fdb)+\
                _IQmpy(foc1.Idqfdb_filter_K3,foc1.I_Qs);

        PI_MACRO(pi_iq) //q��PI����

        if(MC.Servo_OFF == 1)
        {
            pi_id.ui=0;
            pi_id.up=0;
            pi_id.i1=0;
            pi_id.v1=0;
            pi_id.Out=0;

            pi_iq.ui=0;
            pi_iq.up=0;
            pi_iq.i1=0;
            pi_iq.v1=0;
            pi_iq.Out=0;
        }

        //step6: IPARK�任(����ʱ��=40*16.667ns=0.667us)
        foc1.U_Ds = pi_id.Out;//Ud_ref
        foc1.U_Qs = pi_iq.Out;//Uq_ref
        IPARK_MACRO(foc1)

        //step7: SVPWM and PWMռ�ձȼ���
        pwm1.Ualpha = foc1.U_Alpha;
        pwm1.Ubeta  = foc1.U_Beta;
        PWM1_H8_MACRO(pwm1)//8PWMռ�ձȼ���(ռ�ձ��������ƽ����

        //step8: ���ϱ���
        if(MC.S_ERR==1)
        {PWM1_OFF_MACRO()}
}

//---------------------------------------------------------------------------------

#pragma CODE_SECTION(Run_Step_MotorB, "ramfuncs");
static inline void Run_Step_MotorB(void)
{
    if(TZ.loopmode == 1)
    {
#if(Run_Mode_MotorB==Close_Loop)
        //static Sta_Zero=0;
        //static Zero_Find_Ticker=0;
        //------------------------�ڸ�-------------------------------------
        if(MB.Zero_Find_Ena==1)//����λ
        {
            Zero_Find_Handle_B();
        }

        //------�⻷��������-------//
        if(MB.Align_Ticker < MB.Align_Time)//1s
        {   //�����ʼ��λ�׶�
            MB.Align_Ticker++;
            SpeedLoopCnt2=0;//�⻷�������̼���ֵ����
        }
        else
        {   //����ջ����ƽ׶�(1,2,3,4)
            if(SpeedLoopCnt2>=SpeedLoopPrescaler)
                SpeedLoopCnt2=1;
            else
                SpeedLoopCnt2++;  //�⻷�������̼���ֵ
        }

        //"0":��ʼ��λ; ��1~4��:ʸ���ջ�����
        switch(SpeedLoopCnt2)
        {
        //--------------------------------------------//
        case 0://��ʼ��λ�л�
            //------------------------------------------------------------------------------------
            //�ڸ� d�����ָ��
            if(pi_id2.Ref <= MB.Align_Idref)
            {
                pi_id2.Ref += _IQ(0.00004);//��ʼ��λʱ��������б������
                if(pi_id2.Ref >= MB.Align_Idref)
                    pi_id2.Ref = MB.Align_Idref;
            }

            pi_iq2.Ref = 0; //q�����ָ��

            qep2.CalibratedAngle = (EQep2Regs.QPOSCNT > qep2.LineEncoder*2)? \
                    (EQep2Regs.QPOSCNT - qep2.LineEncoder*4):EQep2Regs.QPOSCNT;

            //�ϵ��������ζ�λ���ȶ�λ��-90�ȣ��ٶ�λ��0��
            if(MB.Align_Ticker<= (MB.Align_Time>>1))
            {
                MB.Align_ElecTheta = _IQ(-0.25);//d�ᶨλʱ,�趨��Ƕ�Ϊ-90
            }
            else if( MB.Align_Ticker >(MB.Align_Time>>1) && (MB.Align_Ticker < MB.Align_Time) )
            {
                MB.Align_ElecTheta += _IQ(0.00001);
                if( MB.Align_ElecTheta>=0)
                    MB.Align_ElecTheta = 0;    //�ٶ�λ��0��
                qep2.ElecTheta = 0;
            }

            if(MB.Align_Ticker == MB.Align_Time)//����ȷ������;
            {
                qep2.IndexSyncFlag = 0x00;
                //Ϊ�˷�ֹDSP��λ����λ��־��ʼֵ����ȷ��������ǿ��������λ��־λIEL
                EQep2Regs.QCLR.bit.IEL = 1; //Clear interrupt flag
            }
            else
            {
                qep2.IndexSyncFlag = 0x0F;
            }//��λ�����У���ֹ����ԭ��λ�ã�����Ϊ�ҵ�ԭ��

            break;
            //-----------------------------------------------------------//
        case 1:
            if(curve2.Cur_mode==0)
            {
                if(curve2.GenFlg==2)
                {
                    if(MB.Pulse_set_Ena==1) //idle״̬�²�����д���µ�����ָ��
                    {
                        if(TZ.Address == 1)
                        {
                            curve2.DeltaDistance = MB.Motor_pos_set;
                        }
                        else if(TZ.Address == 2)
                        {
                            curve2.DeltaDistance = -MB.Motor_pos_set;
                        }
                        MB.Pulse_set_Ena=0;
                        Dlog_en = 1;
                    }
                }
                CurveGen(&curve2);
            }
            else
            {
                curve2.OutputPulseErr = curve2.PositionPulse - curve2.PositionPulseOld;
                curve2.PositionPulseOld = curve2.PositionPulse;
            }

            tPosPro_y.iPosCount_Err += curve2.OutputPulseErr;//��������

            //��ȡ����������ֵ
            tPosReg_y.lFeedBack = EQep2Regs.QPOSCNT;

            tPosPro_y.uCLR = MB.P_Err_Clear; //λ�ü���������
            //λ�û�����1--����ʱ�䣨66*16.7ns=1.1us)
            POS_PD_Y1Macro()  //λ�û�����1(����ƫ�����)

            //��ȡλ�ø���ƫ�λ�ó����ʹ�ã�
            MB.P_Out_Err = tPosPro_y.tErrCounter.lErrCount_Out;

            //���ݹ���״̬���л�����ָ���޷�ֵ����������
            Gain_Switch_Handle_B();
            break;
            //-------------------------------------------------------//
        case 2:
            //λ�û�����2--����ʱ�䣨107*16.7ns=1.78us)
            POS_PD_Y2Macro()   //λ�û�����2--/*����������(������3us)*/
            break;
            //--------------------------------------------------------//
        case 3:  //ת�ٷ�������--����ʱ�䣨109*16.7=2.01us��
            //case 2:
            POS_PD_Y3Macro()   //λ�û�����3--/*λ�û�FAL����*/

            //���ٶ�ָ������޷�
            if(MB.Pos_Rst_Ticker<10)//���Ƶ���ϵ��λ���̵ĵ��ת��2400*0.01=24rpm
            {
                MB.Pos_Rst_Ticker++;
                Spdref_filter2.input = _IQsat( ((int32)tPosReg_y.lSpd_Ref),\
                                               _IQ(0.1),-_IQ(0.1));
            }
            else
            {
                Spdref_filter2.input = _IQsat( ((int32)tPosReg_y.lSpd_Ref),\
                                               MB.Speedref_Limit,-MB.Speedref_Limit);
            }

            //���ٶ�ָ���ͨ�˲�
            LOWPASS_FILTER_MACRO(Spdref_filter2)

            //ת�ٻ�ָ��ѡ��
            if(MB.Zero_Find_Ena==0)
            { pi_spd2.Ref = Spdref_filter2.output;} //�˲����ת��ָ��
            else if(MB.Zero_Find_Ena==1)//�ҵ����λת��ָ��=60rpm
            {
                if(TZ.Address == 1)
                {
                    pi_spd2.Ref = MB.SpeedRef_Find_Z;
                }
                else if(TZ.Address == 2)
                {
                    pi_spd2.Ref = -MB.SpeedRef_Find_Z;
                }
            }
            //pi_spd.Ref = MC.SpeedRef_Find_Z;
            break;
            //-------------------------------------------------------//
        case 4://�ٶȻ�����--����ʱ��(73~130*16.7ns=2.54us)

            //(1)ת�ټ������=60*ת�ټ���Ƶ��/(����������*4)
            //(2)ת�ټ������=60*1000/(1000*4)=15rpm (1000�߱�������

            //���ת�ټ���
            SpeedCalCnt2++;
            if(SpeedCalCnt2>=SpeedCalPrescaler) //���ת�ټ���
            {
                SpeedCalCnt2= 0;
                speed2.ElecTheta = qep2.ElecTheta;//��ȡ��Ƕȼ���ת��
                SPEED_FR_MACRO(speed2)  //ת�ټ���(M��)
                pi_spd2.Fdb = speed2.Speed;//ת�ٷ���
            }

            //ת�ٻ�PI����
            PI_MACRO(pi_spd2)

            //q�����ָ��--ת�ٻ�����ĵ���ָ���˲�
            pi_iq2.Ref = _IQmpy(foc2.Iqref_filter_K2,pi_iq2.Ref)+\
            _IQmpy(foc2.Iqref_filter_K3,pi_spd2.Out);

            pi_id2.Ref = 0;//; //��������-_IQ(0.05)

            break;
            //--------------------------------------------------//
        default: break;
        }//end of �⻷����
    }

    if(TZ.loopmode == 1)
    {
        //----����������----//
        //step1: Park�任�Ƕȼ���(����ʱ��=103*16.667ns=1.716us)

        //���㷴���������Ƕ�(55*16.7=0.92us)
        QEP2_MACRO(qep2)  //���ü��������ֵ��λeQEP������,����Ƿ��I����S
                if(SpeedLoopCnt2==0)//��Ӧd�ᶨλ״̬
                {foc2.Angle = MB.Align_ElecTheta;} //��λ��Ƕ�
                else
                {foc2.Angle = qep2.ElecTheta;} //(���ñ�����������Ƕȣ�
    }
    else
    {
        if(TZ.Address == 1)
        {
            if(TZ.Direction == 0)
            {
                //foc2.Angle -= _IQ(0.001);//�ӿ�
                foc2.Angle -= _IQ(0.0001);//����ǰ������
            }
            else
            {
                //foc2.Angle += _IQ(0.001);//�ӿ�
                foc2.Angle += _IQ(0.0001);//������˷���
            }
        }
        else if(TZ.Address == 2)
        {
            if(TZ.Direction == 0)
            { foc2.Angle += _IQ(0.0001);}//����ǰ������
            else
            { foc2.Angle -= _IQ(0.0001);}//������˷���
        }

        if(foc2.Angle > _IQ(1))
            foc2.Angle  = foc2.Angle - _IQ(1);
        if(foc2.Angle < _IQ(-1))
            foc2.Angle  = foc2.Angle + _IQ(1);
        pi_id2.Ref += _IQ(0.005);
        if(pi_id2.Ref > _IQ(0.6))
        {
            pi_id2.Ref = _IQ(0.6);
        }
        pi_iq2.Ref = 0;
    }
#endif
    //����任��sin,cos����
    foc2.Sine   = _IQsinPU(foc2.Angle);
    foc2.Cosine = _IQcosPU(foc2.Angle);

    //step2: ADC��������(����ʱ��=40*16.667ns=0.667us)
    //�������adc1.Ia,adc1.Ib,adc1.Udc
    ADC2_READ_MACRO(adc2)
    //**********�ɼ��۲�����************//

    if(Dlog_en == 1)
    {
        curve_run_flag[1]++;
        if(curve_run_flag[1]>=20)//20--1ms
        {
            curve_run_flag[1]=0;

            //if(curve_run_flag[2]>=100)
            //   curve_run_flag[2]=0;

            if(curve_run_flag[2]<100)//100������
            {
                speed_ref[curve_run_flag[2]]=MB.P_Out_Err;//curve2.PositionPulse;//pi_spd.Fdb;////pi_spd.Ref//MC.P_Out_Err//motor1.Pulse_Out//curve1.PositionPulse
                if(EQep2Regs.QPOSCNT>2000) speed_bck[curve_run_flag[2]]=EQep2Regs.QPOSCNT-4000;
                else speed_bck[curve_run_flag[2]]=EQep2Regs.QPOSCNT;//pi_spd.Fdb//EQep1Regs.QPOSCNT
                //speed_bck[curve_run_flag[2]]=pi_spd.Ref;
                curve_run_flag[2]++;
            }
            else
            {
                Dlog_en = 0;

            }
        }
    }
    else
    {
        curve_run_flag[1] = 0;
        curve_run_flag[2] = 0;
    }
    //*/
    //**********�ɼ��۲�����end************//

    //**********����������������(������)************//
    static Uint16 OC_Delay_Ia=0;
    static Uint16 OC_Delay_Ib=0;

    //Ia�������
    if( _IQ15abs(adc2.Ia)>=Over_Current_Limit)
    {
        OC_Delay_Ia++;
        if(OC_Delay_Ia>OC_Delay_Set)
        {
            PWM2_OFF_MACRO()
        	                OC_Delay_Ia=OC_Delay_Set;
            if(MB.S_ERR == 0)
            {
                MB.Err_Value = 2;//������--f02(A�����)
                MB.S_ERR=1;//�ù��ϱ�־
                MB.Err_Wri_Ena=1; //ʹ�ܹ�����д��EEPROM
            }
        }
    }
    else
    {
        OC_Delay_Ia=0;
    }

    //Ib�������
    if( _IQ15abs(adc2.Ib)>=Over_Current_Limit) //������������
    {
        OC_Delay_Ib++;
        if(OC_Delay_Ib>OC_Delay_Set)
        {
            PWM2_OFF_MACRO()
                            OC_Delay_Ib=OC_Delay_Set;
            if(MB.S_ERR == 0)
            {
                MB.Err_Value = 2;//������--f02(B�����)
                MB.S_ERR=1;//�ù��ϱ�־
                MB.Err_Wri_Ena=1; //ʹ�ܹ�����д��EEPROM
            }
        }
    }
    else
    {
        OC_Delay_Ib=0;
    }

    //step3: PARK�任(����ʱ��=36*16.667ns=0.6us)
    foc2.I_Alpha = _IQ15toIQ(adc2.Ia);//Ia_IQ24;
    foc2.I_Beta  = _IQ15toIQ(adc2.Ib);//Ib_IQ24;
    PARK_MACRO(foc2) //park�任(���ಽ���������ʱʹ�ã�

    //step4: d������PI����(����ʱ��=49*16.667ns=0.817us)
    pi_id2.Fdb = _IQmpy(foc2.Idqfdb_filter_K2,pi_id2.Fdb)+\
    _IQmpy(foc2.Idqfdb_filter_K3,foc2.I_Ds);

    PI_MACRO(pi_id2) //d��PI����

    /*********Uq_Limit����*******/
    register _iq Uq_Limit;
    Uq_Limit = _IQsqrt( _IQ(0.95) - _IQmpy(pi_id2.Out,pi_id2.Out) );
    pi_iq2.Umax =  Uq_Limit;
    pi_iq2.Umin = -Uq_Limit;

    //step5: q������PI����(����ʱ��=49*16.667ns=0.817us)
    pi_iq2.Fdb = _IQmpy(foc2.Idqfdb_filter_K2,pi_iq2.Fdb)+\
            _IQmpy(foc2.Idqfdb_filter_K3,foc2.I_Qs);

    PI_MACRO(pi_iq2) //q��PI����

    //step6: IPARK�任(����ʱ��=40*16.667ns=0.667us)
    foc2.U_Ds = pi_id2.Out;//Ud_ref
    foc2.U_Qs = pi_iq2.Out;//Uq_ref
    IPARK_MACRO(foc2)

    //step7: SVPWM and PWMռ�ձȼ���
    pwm2.Ualpha = foc2.U_Alpha;
    pwm2.Ubeta  = foc2.U_Beta;
    PWM2_H8_MACRO(pwm2)//8PWMռ�ձȼ���(ռ�ձ��������ƽ����

    //step8: ���ϱ���
    if(MB.S_ERR==1)
    {PWM2_OFF_MACRO()}
}
/**************************************************************
//���������л���
**************************************************************/
#pragma CODE_SECTION(Gain_Switch_Handle_Y, "ramfuncs");
static inline void Gain_Switch_Handle_Y(void)//���������л�����
{
	//step1:  �����λ�ж�
	#define Pos_Limit_Set     3    //�����λƫ���ж�ֵ
	#define Motor_Ready_Time_Set 15  //�ж�ʱ��=1ms*10=10ms
	static  Uint16 Motor_Ready_Delay=0;  //(motor1.step_state==0)

	if((MC.S_ERR==0)&&(curve1.GenFlg==2)\
			&&(abs(MC.P_Out_Err)<=Pos_Limit_Set) )
	{
		Motor_Ready_Delay++;
		if(Motor_Ready_Delay>=Motor_Ready_Time_Set)
		{
			Motor_Ready_Delay = Motor_Ready_Time_Set;
			MC.Work_Stop_Flag = 0;//����ѵ�λ
		}
	}
	else
	{
		Motor_Ready_Delay = 0;
		MC.Work_Stop_Flag = 1;//���û�е�λ
	}

	//step2: �����л�
	static Uint16 Work_Stop_Flag_Last =0;
	if(Work_Stop_Flag_Last!=MC.Work_Stop_Flag)
	{
		MC.Gain_Switch_Ena=1;
		Work_Stop_Flag_Last=MC.Work_Stop_Flag;
	}

	if(MC.Gain_Switch_Ena==1)
	{
		MC.Gain_Switch_Ena=0;

		if(MC.Work_Stop_Flag==1)
		{
			/*******����״̬*******/
			//λ�û�����
			tPosPro_x.uKp_Pos = MC.Pos_Kp_Work; //λ�û�����PA.Pos_Kp=35--IQ(35/32)=IQ(1.09)

			//ת�ٻ�����������
			pi_spd.Kp = MC.Spd_Kp_Work;
			pi_spd.Ki = MC.Spd_Ki_Work;

			//����ָ���޷�ֵ
			pi_id.Kp = MC.Cur_Kp_Work;
			pi_id.Ki = MC.Cur_Ki_Work;
			pi_iq.Kp = pi_id.Kp;
			pi_iq.Ki = pi_id.Ki;

			MC.Iq_overload_Limit = _IQmpy(_IQ(0.9),MC.Iq_ref_Limit_Work);//�������������޷�
			pi_spd.Umax =  MC.Iq_ref_Limit_Work;
			pi_spd.Umin = -MC.Iq_ref_Limit_Work;
		}
		else
		{
			/*****ͣ��״̬******/
			//λ�û�����
			tPosPro_x.uKp_Pos = MC.Pos_Kp_Stop; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

			//ת�ٻ�����������
			pi_spd.Kp = MC.Spd_Kp_Stop;
			pi_spd.Ki = MC.Spd_Ki_Stop;

			//����������������
			pi_id.Kp = MC.Cur_Kp_Stop;
			pi_id.Ki = MC.Cur_Ki_Stop;
			pi_iq.Kp = pi_id.Kp;
			pi_iq.Ki = pi_id.Ki;

			//������ѹ�޷�
			MC.Iq_overload_Limit = _IQmpy(_IQ(0.6),MC.Iq_ref_Limit_Stop);//ͣ�����������޷�
			pi_spd.Umax =  MC.Iq_ref_Limit_Stop;
			pi_spd.Umin = -MC.Iq_ref_Limit_Stop;
		}
	}
}

/*************************************************************/
#pragma CODE_SECTION(Gain_Switch_Handle_B, "ramfuncs");
static inline void Gain_Switch_Handle_B(void)//���������л�����
{
	//step1:  �����λ�ж�
	#define Pos_Limit_Set     3    //�����λƫ���ж�ֵ
	#define Motor_Ready_Time_Set 15  //�ж�ʱ��=1ms*10=10ms
	static  Uint16 Motor_Ready_Delay=0;  //(motor1.step_state==0)

	if((MB.S_ERR==0)&&(curve2.GenFlg==2)\
			&&(abs(MB.P_Out_Err)<=Pos_Limit_Set) )
	{
		Motor_Ready_Delay++;
		if(Motor_Ready_Delay>=Motor_Ready_Time_Set)
		{
			Motor_Ready_Delay = Motor_Ready_Time_Set;
			MB.Work_Stop_Flag = 0;//����ѵ�λ
		}
	}
	else
	{
		Motor_Ready_Delay = 0;
		MB.Work_Stop_Flag = 1;//���û�е�λ
	}

	//step2: �����л�
	static Uint16 Work_Stop_Flag_Last =0;
	if(Work_Stop_Flag_Last!=MB.Work_Stop_Flag)
	{
		MB.Gain_Switch_Ena=0;
		Work_Stop_Flag_Last=MB.Work_Stop_Flag;
	}

	if(MB.Gain_Switch_Ena==1)
	{
		MB.Gain_Switch_Ena=0;

		if(MB.Work_Stop_Flag==1)
		{
			/*******����״̬*******/
			//λ�û�����
			tPosPro_y.uKp_Pos = MB.Pos_Kp_Work; //λ�û�����PA.Pos_Kp=35--IQ(35/32)=IQ(1.09)

			//ת�ٻ�����������
			pi_spd2.Kp = MB.Spd_Kp_Work;
			pi_spd2.Ki = MB.Spd_Ki_Work;

			//����ָ���޷�ֵ
			pi_id2.Kp = MB.Cur_Kp_Work;
			pi_id2.Ki = MB.Cur_Ki_Work;
			pi_iq2.Kp = pi_id2.Kp;
			pi_iq2.Ki = pi_id2.Ki;

			//MB.Iq_overload_Limit = _IQmpy(_IQ(0.9),MB.Iq_ref_Limit_Work);//�������������޷�
			MB.Iq_overload_Limit = _IQmpy(_IQ(0.8),MB.Iq_ref_Limit_Work);//�������������޷�
			pi_spd2.Umax =  MB.Iq_ref_Limit_Work;
			pi_spd2.Umin = -MB.Iq_ref_Limit_Work;
		}
		else
		{
			/*****ͣ��״̬******/
			//λ�û�����
		    //if(TZ.Position_state != 1)
		    //{
		        tPosPro_y.uKp_Pos = MB.Pos_Kp_Stop; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

		        //ת�ٻ�����������
		        pi_spd2.Kp = MB.Spd_Kp_Stop;
		        pi_spd2.Ki = MB.Spd_Ki_Stop;
		    //}
		   /* else
		    {
		        tPosPro_y.uKp_Pos = 50; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

		        //ת�ٻ�����������
		        pi_spd2.Kp = _IQ(5.0);
		        pi_spd2.Ki = _IQ(0.01);
		    }*/

			//����������������
			pi_id2.Kp = MB.Cur_Kp_Stop;
			pi_id2.Ki = MB.Cur_Ki_Stop;
			pi_iq2.Kp = pi_id2.Kp;
			pi_iq2.Ki = pi_id2.Ki;

			//������ѹ�޷�
			//MB.Iq_overload_Limit = _IQmpy(_IQ(0.6),MB.Iq_ref_Limit_Stop);//ͣ�����������޷�
			MB.Iq_overload_Limit = _IQmpy(_IQ(0.8),MB.Iq_ref_Limit_Stop);//ͣ�����������޷�
			pi_spd2.Umax =  MB.Iq_ref_Limit_Stop;
			pi_spd2.Umin = -MB.Iq_ref_Limit_Stop;
		}
	}
}

//�����ת���ƺ���
#pragma CODE_SECTION(Self_Run_Test, "ramfuncs");
void Self_Run_Test(void) //for �ϻ�����
{
     static Uint16  Self_Run_Ticker=0;
     static Uint16  Sec1_Ticker=0;
     static Uint16  Y_End=0;

       Sec1_Ticker++;
       if(Sec1_Ticker>5000)
       {
          Sec1_Ticker = 0;
          //STEPTOGGLE = 1;
          Self_Run_Ticker++;
       }

               if(SpeedLoopCnt1!=0)
               {
                  //������ת����
                  ///Self_Run_Ticker++;
                 //-------------------------------------
                   if(Self_Run_Ticker==0)
                   {
                	   curve1.RunTime  = 830;
					  curve1.RiseTime = 50;
					  curve1.DownTime = 50;
					  curve1.ChangeFlg = 1;
					//curve1.tCounter=0;
					  curve1.Cur_mode=0;
					  MC.Motor_pos_set = -4000; //��ת3Ȧ
					  MC.Pulse_set_Ena = 1;
                   }
                 //-------------------------------------
                    if(Self_Run_Ticker==1&&Y_End==0)
                   {
                    	 curve1.RunTime  = 830;
						  curve1.RiseTime = 50;
						  curve1.DownTime = 50;
						  curve1.ChangeFlg = 1;
						//curve1.tCounter=0;
						  curve1.Cur_mode=0;
						  MC.Motor_pos_set = 4000; //��ת3Ȧ
						  MC.Pulse_set_Ena = 1;
						  Y_End=1;
                   }
                  //-------------------------------------
                   if(Self_Run_Ticker>=2)
                   {
                      Self_Run_Ticker = 0;
                      Sec1_Ticker=0;
                      Y_Mor_Ena=0;
                      Y_End=0;
                   }
               }

       //}// end of 1s
}// end of Self_Run_Test  PWM6CAL
#pragma CODE_SECTION(Zero_Find_Handle_A, "ramfuncs");
static inline void Zero_Find_Handle_A(void)
{
    //����������ɫʱ������λ������ʱ���
#define Zero_Find_OverTime_Set 120000 //��Z��ʱʱ��=0.025ms*120000=3s

    static Uint32 Zero_Find_Delay=0;

    if((SpeedLoopCnt1!=0)&&(MC.S_ERR==0))
    {
        Zero_Find_Delay++;
        if(Zero_Find_Delay>=Zero_Find_OverTime_Set)
        {
            Zero_Find_Delay=0;
            MC.S_ERR=1;//�ù��ϱ�����־
            MC.Err_Value = 11;// ������-f11(����λ��ʱ)
            MC.Err_Wri_Ena =1;//������Ϣд��EEPROMʹ��
        }
    }

    MC.P_Err_Clear = 1;//λ�ü���������
    if(qep1.IndexSyncFlag == 0x00F0)//�ҵ������λZ
    {
        qep1.IndexSyncFlag = 0x0F;//0x0F��־���������λ

        Zero_Find_Delay=0;

        MC.Zero_Find_Ena  = 0; //������λʹ�ܱ�־
        MC.Zero_Find_Flag = 1; //���ҵ�Z�źű�־
        MC.P_Err_Clear = 0; //λ�ü������ָ�������
        //Ϊ��У��������Z�ź�ƫ����ת��λ��ƫ��Ƕ�
#if(Znc_Z_offset_Ena)
        //����ֱ�ӷ�ָ������ķ�ʽ������Z�Ƕ�ƫ��(����OK)
        //������Z�źſ��4*4������ֵΪZ�źſ�ȵ�һ��
#define Enc_Z_offset_Set1  8//-2    //���ֱ�4:1

        //��������λʱ���ת������ȷ������Z�Ƕ�ƫ��
        if( MC.SpeedRef_Find_Z > 0)
        {
            MC.Motor_pos_set = Enc_Z_offset_Set1;//
        }
        else
        {
            MC.Motor_pos_set = -Enc_Z_offset_Set1;
        }
        MC.Pulse_curve_select=0;
        MC.Pulse_set_Ena = 1;
#endif
    }
}
#pragma CODE_SECTION(Zero_Find_Handle_B, "ramfuncs");
static inline void Zero_Find_Handle_B(void)
{
    //����������ɫʱ������λ������ʱ���
#define Zero_Find_OverTime_Set_B 120000 //��Z��ʱʱ��=0.025ms*120000=3s

    static Uint32 Zero_Find_Delay_B=0;

    if((SpeedLoopCnt2!=0)&&(MB.S_ERR==0))
    {
        Zero_Find_Delay_B++;
        if(Zero_Find_Delay_B>=Zero_Find_OverTime_Set_B)
        {
            Zero_Find_Delay_B=0;
            MB.S_ERR=1;//�ù��ϱ�����־
            MB.Err_Value = 11;// ������-f11(����λ��ʱ)
            MB.Err_Wri_Ena =1;//������Ϣд��EEPROMʹ��
        }
    }

    MB.P_Err_Clear = 1;//λ�ü���������
    if(qep2.IndexSyncFlag == 0x00F0)//�ҵ������λZ
    {
        qep2.IndexSyncFlag = 0x0F;//0x0F��־���������λ

        Zero_Find_Delay_B=0;

        MB.Zero_Find_Ena  = 0; //������λʹ�ܱ�־
        MB.Zero_Find_Flag = 1; //���ҵ�Z�źű�־
        MB.P_Err_Clear = 0; //λ�ü������ָ�������
        //Ϊ��У��������Z�ź�ƫ����ת��λ��ƫ��Ƕ�
#if(Znc_Z_offset_Ena)
        //����ֱ�ӷ�ָ������ķ�ʽ������Z�Ƕ�ƫ��(����OK)
        //������Z�źſ��4*4������ֵΪZ�źſ�ȵ�һ��
#define Enc_Z_offset_Set2  15//-2    //���ֱ�4:1

        //��������λʱ���ת������ȷ������Z�Ƕ�ƫ��
        if( MB.SpeedRef_Find_Z > 0)
        {
            MB.Motor_pos_set = Enc_Z_offset_Set2;//
        }
        else
        {
            MB.Motor_pos_set = -Enc_Z_offset_Set2;
        }
        MB.Pulse_curve_select=0;
        MB.Pulse_set_Ena = 1;
#endif
    }
}
#pragma CODE_SECTION(Run_Step_MotorC, "ramfuncs");
static inline void Run_Step_MotorC(void)
{
    if(motorC.step_state!=0)
    {
        IRQHandler_SPTA(&motorC);//����������ٶȺ�һֱ������壬ֱ����⵽��λ�ź�
        if(motorC.SPTA_PULSE >= 1)
        {
            M1_CLK = 1;//��ɫ���
            motorC.SPTA_PULSE = 0;
        }
    }
    else
    {
        if(motorC.DisableFlag == 0)
        {
            M1_DIS = 1;
            motorC.DisableFlag = 1;
        }
        if(motorC.PulseEnable == 1)
        {
            motorC.DisableFlag = 0;
            motorC.PulseEnable = 0;
            switch(PA.F3)
            {
            case 0:
                motorC.step_spmax=15000;
                motorC.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=200000;
                break;
            case 1:
                motorC.step_spmax=17000;
                motorC.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=250000;
                break;
            case 2:
                motorC.step_spmax=19000;
                motorC.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=250000;
                break;
            case 3:
                motorC.step_spmax=21000;
                motorC.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 4:
                motorC.step_spmax=23000;
                motorC.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 5:
                motorC.step_spmax=25000;
                motorC.step_accel_up=350000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 6:
                motorC.step_spmax=29000;
                motorC.step_accel_up=350000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 7:
                motorC.step_spmax=33000;
                motorC.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 8:
                motorC.step_spmax=37000;
                motorC.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            case 9:
                motorC.step_spmax=40000;
                motorC.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorC.step_accel_down=300000;
                break;
            }
/*
            //motorC.step_spmax=30000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorC.step_accel_up=800000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorC.step_accel_down=200000;
            //motorC.speedenbale = 1;
*/
            Start_Motor_SPTA(0,motorC.Degree,&motorC);
            M1_EN = 1;
        }
    }
}
#pragma CODE_SECTION(Run_Step_MotorD, "ramfuncs");
static inline void Run_Step_MotorD(void)
{
    if(motorD.step_state!=0)
    {
        IRQHandler_SPTA(&motorD);//����������ٶȺ�һֱ������壬ֱ����⵽��λ�ź�
        if(motorD.SPTA_PULSE >= 1)
        {
            M2_CLK = 1;//��ɫ���
            motorD.SPTA_PULSE = 0;
        }
    }
    else
    {
        if(motorD.DisableFlag == 0)
        {
            M2_DIS = 1;
            motorD.DisableFlag = 1;
        }
        if(motorD.PulseEnable == 1)
        {
            motorD.DisableFlag = 0;
            motorD.PulseEnable = 0;
            //switch(PA.F3)
            switch(PA.F5)
            {
            case 0:
                motorD.step_spmax=15000;
                motorD.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=200000;
                break;
            case 1:
                motorD.step_spmax=17000;
                motorD.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=250000;
                break;
            case 2:
                motorD.step_spmax=19000;
                motorD.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=250000;
                break;
            case 3:
                motorD.step_spmax=21000;
                motorD.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 4:
                motorD.step_spmax=23000;
                motorD.step_accel_up=300000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 5:
                motorD.step_spmax=25000;
                motorD.step_accel_up=350000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 6:
                motorD.step_spmax=29000;
                motorD.step_accel_up=350000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 7:
                motorD.step_spmax=33000;
                motorD.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 8:
                motorD.step_spmax=37000;
                motorD.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
            case 9:
                motorD.step_spmax=40000;
                motorD.step_accel_up=400000;//����칤�������Ч�ʵ��޸ģ��������
                motorD.step_accel_down=300000;
                break;
                /*            case 0:
                motorD.step_spmax=10000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 1:
                motorD.step_spmax=10000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 2:
                motorD.step_spmax=15000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 3:
                motorD.step_spmax=20000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 4:
                motorD.step_spmax=25000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 5:
                motorD.step_spmax=30000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 6:
                motorD.step_spmax=35000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 7:
                motorD.step_spmax=40000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 8:
                motorD.step_spmax=40000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;
            case 9:
                motorD.step_spmax=40000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
                break;*/
            }
/*
            //motorD.step_spmax=30000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorD.step_accel_up=800000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorD.step_accel_down=200000;
            //motorD.speedenbale = 1;
*/
            Start_Motor_SPTA(0,motorD.Degree,&motorD);
            M2_EN = 1;
        }
    }
}
Uint16 clkout_over = 0;
#pragma CODE_SECTION(Run_Step_MotorE, "ramfuncs");
static inline void Run_Step_MotorE(void)
{
    if(clkout_over == 1)
    {
        M3_CLK_LO = 1;
        clkout_over = 0;
    }
    if(motorE.step_state!=0)
    {
        IRQHandler_SPTA(&motorE);//����������ٶȺ�һֱ������壬ֱ����⵽��λ�ź�
        if(motorE.SPTA_PULSE >= 1)
        {
            //M3_CLK = 1;//��ɫ���
            M3_CLK_HI = 1;
            motorE.SPTA_PULSE = 0;
            clkout_over = 1;
        }
    }
    else
    {
        if(motorE.DisableFlag == 0)
        {
            //M3_DIS = 1;
            motorE.DisableFlag = 1;
        }
        if(motorE.PulseEnable == 1)
        {
            motorE.DisableFlag = 0;
            motorE.PulseEnable = 0;
            //motorE.speedenbale = 1;
            Start_Motor_SPTA(0,motorE.Degree,&motorE);
            M3_EN = 1;
        }
    }
}
#define TZ_Distance_step1 -790
#define TZ_Distance_step2 -100
#define TZ_Distance_step3 890

//3������������
#pragma CODE_SECTION(Push_Pearl, "ramfuncs");
#if 0
Uint16 Push_Pearl(int Dir)
{
    static Uint16 Pearl_Position_State = 0;
    Pearl_Position_State++;
    if(Pearl_Position_State == 1)
    {
/*        curve2.RunTime  = 80;//���ܸ�Ϊ100��������ϻ�������
        curve2.RiseTime = 20;
        curve2.DownTime = 20;*/
        //curve2.RunTime  = 60;//���ܸ�Ϊ100��������ϻ�������
        curve2.RunTime  = 70;//�����칤������4.0���Ӳ������̫�죬��Ҫ��ϸ����� 2019.05.20
        curve2.RiseTime = 5;
        //curve2.DownTime = 10;
        curve2.DownTime = 5;//�����칤������4.0���Ӳ�����������Լ��٣���Ҫ��ϼӿ죬 2019.05.26
        curve2.GenFlg = 2;
        CurveInit(&curve2);//���߷�������ʼ��
        //tPosPro_y.uKp_Pos = 60;
       //tPosPro_y.uKp_Pos = 75;//ͨ�ð棺�̶�λ��
        //tPosPro_y.uKp_Pos = 63;//T�棺����ͣģʽ
        //pi_spd2.Kp = _IQ(4);
        //pi_spd2.Ki = _IQ(0.04);

        tPosPro_y.uKp_Pos = 75;
        pi_spd2.Kp = _IQ(4.0);
        pi_spd2.Ki = _IQ(0.04);
        pi_id2.Kp = _IQ(5.0);
        pi_id2.Ki = _IQ(0.02);
        pi_iq2.Kp = _IQ(5.0);
        pi_iq2.Ki = _IQ(0.02);

        MB.Motor_pos_set = TZ.Step1 + (-TZ.DeltaStep1[PA.F1_Value]);
        MB.Pulse_set_Ena = 1;
    }
    else if(Pearl_Position_State == 2)
    {
        curve2.RunTime  = 10;
        curve2.RiseTime = 5;
        curve2.DownTime = 5;
        curve2.GenFlg = 2;
        CurveInit(&curve2);//���߷�������ʼ��
        //tPosPro_y.uKp_Pos = 25;
       //tPosPro_y.uKp_Pos = 30;
        //pi_spd2.Kp = _IQ(5);
        //pi_spd2.Ki = _IQ(0.025);

        tPosPro_y.uKp_Pos = 25;
        pi_spd2.Kp = _IQ(4.0);
        pi_spd2.Ki = _IQ(0.04);
        pi_id2.Kp = _IQ(5.0);
        pi_id2.Ki = _IQ(0.02);
        pi_iq2.Kp = _IQ(5.0);
        pi_iq2.Ki = _IQ(0.02);
        MB.Motor_pos_set = TZ.Step2 + (-TZ.DeltaStep2[PA.F2_Value]);
        MB.Pulse_set_Ena = 1;
    }
    else if(Pearl_Position_State == 3)
    {
        //ͨ�ð�
        //curve2.RunTime  = 70;
        //curve2.RiseTime = 20;
        //curve2.DownTime = 10;
        //T��
/*        curve2.RunTime  = 80;//���ܸ�Ϊ100��������ϻ�������
        curve2.RiseTime = 20;
        curve2.DownTime = 20;*/
        //curve2.RunTime  = 60;//������ 2019.04.28
        curve2.RunTime  = 70;//�����칤������4.0���Ӳ������̫�죬��Ҫ��ϸ����� 2019.05.20
        curve2.RiseTime = 5;
        //curve2.DownTime = 10;//������ 2019.04.28
        curve2.DownTime = 5;//�����칤������4.0���Ӳ�����������Լ��٣���Ҫ��ϼӿ죬 2019.05.26
        curve2.GenFlg = 2;
        CurveInit(&curve2);//���߷�������ʼ��
        tPosPro_y.uKp_Pos = 30;
       //tPosPro_y.uKp_Pos = 70;//ͨ�ð棺�̶�λ��
        //tPosPro_y.uKp_Pos = 65;//T�棺����ͣģʽ
        pi_spd2.Kp = _IQ(4.0);
        pi_spd2.Ki = _IQ(0.04);

       /* tPosPro_y.uKp_Pos = 35;
        pi_spd2.Kp = _IQ(4.0);
        pi_spd2.Ki = _IQ(0.01);
        pi_id2.Kp = _IQ(5.0);
        pi_id2.Ki = _IQ(0.02);
        pi_iq2.Kp = _IQ(5.0);
        pi_iq2.Ki = _IQ(0.02);*/
        MB.Motor_pos_set = TZ.Step3 + TZ.DeltaStep1[PA.F1_Value]+TZ.DeltaStep2[PA.F2_Value];
        MB.Pulse_set_Ena = 1;

        //��¼�κ�ʱ���������ֶ���������ɫ
        TZ.color = HS.Color_state;
    }
    if(Pearl_Position_State >= 3)
        Pearl_Position_State = 0;
    return Pearl_Position_State;
}
#else
#define TZCurrent TRQ100
#define TRQ100 1
#define TRQ75  2
#define TRQ50  3
#define TRQ25  4
Uint16 Pearl_Position_State = 0;
Uint16 Push_Pearl(Uint16 Dir)
{
    TZ.HandTZDelay = 0;//�����С�����Ķ�ʱ����
    TZ.HandTZFlag = 0;
#if(TZCurrent == TRQ100)
    //100% -> 2.1A
    HC595Regs.bit.M3_TQ0 = 0;
    HC595Regs.bit.M3_TQ1 = 0;
    HC595SendData(HC595Regs.all);
#elif(TZCurrent == TRQ75)
    //75% -> 1.6A
    HC595Regs.bit.M3_TQ0 = 1;
    HC595Regs.bit.M3_TQ1 = 0;
    HC595SendData(HC595Regs.all);
#elif(TZCurrent == TRQ50)
    //50% -> 1.2A
    HC595Regs.bit.M3_TQ0 = 0;
    HC595Regs.bit.M3_TQ1 = 1;
    HC595SendData(HC595Regs.all);
#elif(TZCurrent == TRQ25)
    //25% -> 0.7A
    HC595Regs.bit.M3_TQ0 = 1;
    HC595Regs.bit.M3_TQ1 = 1;
    HC595SendData(HC595Regs.all);
#endif
    switch(Dir)
    {
    case 1:
        if((Embroider.Speed > 85)&&(Embroider.Speed <= 105))
        {
            //1000rpm��Ʋ��� 50ms
            motorE.step_spmax=50000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorE.step_accel_up=20000000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorE.step_accel_down=20000000;
        }
        else if(Embroider.Speed <= 85)
        {
            //��Ʋ��� 60ms
            motorE.step_spmax=40000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorE.step_accel_up=15000000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorE.step_accel_down=15000000;
        }
        //motorE.Degree = 80;
        motorE.Degree = abs(TZ.Step2 + (-TZ.DeltaStep2[PA.F2_Value]));
        if(TZ.Address == 1)
        {M3_CCW = 1;}
        else if(TZ.Address == 2)
        {M3_CW = 1;}
        //M3_CCW = 1;
        motorE.EnableFlag = 1;
        motorE.speedenbale = 0;
        motorE.PulseEnable = 1;

        motorE.TempTest = 0;

        Pearl_Position_State = 1;

        break;
    case 2:
        if((Embroider.Speed > 85)&&(Embroider.Speed <= 105))
        {
            //1000rpm��Ʋ��� 50ms
            motorE.step_spmax=60000;//50000
            motorE.step_accel_up=25000000;//20000000
            motorE.step_accel_down=25000000;//20000000
        }
        else if(Embroider.Speed <= 85)
        {
            //��Ʋ��� 60ms
            motorE.step_spmax=30000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorE.step_accel_up=10000000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorE.step_accel_down=10000000;
        }
        //motorE.Degree = 630;
        motorE.Degree = abs(TZ.Step3 + TZ.DeltaStep1[PA.F1_Value]+TZ.DeltaStep2[PA.F2_Value]);
        if(TZ.Address == 1)
        {M3_CW = 1;}
        else if(TZ.Address == 2)
        {M3_CCW = 1;}
        //M3_CW = 1;
        motorE.EnableFlag = 1;
        motorE.speedenbale = 1;
        motorE.PulseEnable = 1;

        motorE.TempTest = 1;

        //��¼�κ�ʱ���������ֶ���������ɫ
        TZ.color = HS.Color_state;

        Pearl_Position_State = 2;

        break;
    case 3:
        if((Embroider.Speed > 85)&&(Embroider.Speed <= 105))
        {
            //1000rpm��Ʋ��� 50ms
            motorE.step_spmax=60000;//50000
            motorE.step_accel_up=25000000;//20000000
            motorE.step_accel_down=25000000;//20000000
        }
        else if(Embroider.Speed <= 85)
        {
            //��Ʋ��� 60ms
            motorE.step_spmax=40000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
            motorE.step_accel_up=15000000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
            motorE.step_accel_down=15000000;
        }
        //motorE.Degree = 580;
        motorE.Degree = abs(TZ.Step1 + (-TZ.DeltaStep1[PA.F1_Value]));
        if(TZ.Address == 1)
        {M3_CCW = 1;}
        else if(TZ.Address == 2)
        {M3_CW = 1;}
        //M3_CCW = 1;
        motorE.EnableFlag = 1;
        motorE.speedenbale = 0;
        motorE.PulseEnable = 1;

        motorE.TempTest = 0;

        Pearl_Position_State = 3;

        break;
    case 4:
        HC595Regs.bit.M3_TQ0 = 1;
        HC595Regs.bit.M3_TQ1 = 0;
        HC595SendData(HC595Regs.all);
        //��Ʋ��� 60ms
        motorE.step_spmax=10000;//�ʺϸ�ΰ������ͷ�� 2017.06.15
        motorE.step_accel_up=1500000;//���ԼӴ���ٶȣ���ɫʱ������С 2017.02.16
        motorE.step_accel_down=1500000;
        motorE.Degree = 260;
        //motorE.Degree = abs(TZ.Step2 + (-TZ.DeltaStep2[PA.F2_Value]));
        if(TZ.Address == 1)
        {M3_CCW = 1;}
        else if(TZ.Address == 2)
        {M3_CW = 1;}
        motorE.EnableFlag = 1;
        motorE.speedenbale = 0;
        motorE.PulseEnable = 1;

        motorE.TempTest = 0;

        Pearl_Position_State = 1;
        break;
    default:
        break;
    }

    return Pearl_Position_State;
}
#endif
//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED
//======================================================================================
