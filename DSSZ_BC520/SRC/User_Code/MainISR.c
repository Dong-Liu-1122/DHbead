//=================================================================
//  Project Number: step_controller
//  File Name :  MainISR.c
//  Description: 步进电机矢量控制(两相步进电机控制)
//  Version    Date     Author     Description
//  1.0    2016.06.30   wangdayu        use
//-----------------------------------------------------------------
// (1)2016.12.03:编码器Z信号--高电平有效,红灯亮时--Z为高电平,
//=================================================================

//1——头文件
#include "F2806x_Device.h"  //DSP28030外设定义头文件
#include "Stepper-Settings.h"
#include "Pos_Regulator.h"   //位置环控制器头文件
#include "GzTest.h"

//2——中断函数声明
interrupt void PWM1ISR(void); //PWM1下溢中断--40kHz
interrupt void sciaRxFifoIsr(void);
//3——函数声明
static inline void Run_Step_MotorY(void);
static inline void Run_Step_MotorB(void);
static inline void Run_Step_MotorC(void);//开环拌珠电机1
static inline void Run_Step_MotorD(void);//开环拌珠电机2
static inline void Run_Step_MotorE(void);//开环拌珠电机3
//static void Pulse_select(void);//脉冲曲线选择函数
static inline void Gain_Switch_Handle_Y(void); //控制增益切换函数
static inline void Gain_Switch_Handle_B(void); //控制增益切换函数

 static inline void Zero_Find_Handle_A(void);//通用电机找零位函数（Doctor）
 static inline void Zero_Find_Handle_B(void);//通用电机找零位函数（Doctor）

//4——全局变量声明
//volatile 关键字可以保证每次访问该变量时都应从内存中读取
const float32 T = 0.001/(ISR_FREQUENCY);//采用PWM双更新ISR_FREQUENCY
Uint16 SpeedLoopCnt1 = 0;//电机控制状态: 0--定位阶段；电机闭环控制阶段(1,2,3,4)
Uint16 SpeedCalCnt1 = 0;//转速计算计数
Uint16 SpeedLoopCnt2 = 0;//电机控制状态: 0--定位阶段；电机闭环控制阶段(1,2,3,4)
Uint16 SpeedCalCnt2 = 0;//转速计算计数
//Uint16 SW_Status = 0;//拉杆状态0-停车，1-启动
//示波器显示
Uint16 Dlog_en = 0;
_iq speed_ref[100] = {0};
_iq speed_bck[100] = {0};
_iq ref_id = _IQ(1);
_iq ref_id_a = _IQ(0.8);
int32 bck_old = 0;
Uint16 curve_run_flag[5] = {0,0,0,0,0};
int16 Angle_Par=0;
//5——结构体变量声明
T_POS_REG     tPosReg_x = tPosReg_default;
T_POS_FORWARD tPosFor_x = tPosFor_default;  //位置环前馈
T_POS_PROP    tPosPro_x = tPosPro_default;  //位置环比例Ctl_Status

T_POS_REG     tPosReg_y = tPosReg_default;
T_POS_FORWARD tPosFor_y = tPosFor_default;  //位置环前馈
T_POS_PROP    tPosPro_y = tPosPro_default;  //位置环比例Ctl_Status

ADConvter adc1 = ADC_DEFAULTS; //AD采样结构体
ADConvter adc2 = ADC_DEFAULTS; //AD采样结构体
PWMGEN pwm1 = PWMGEN_DEFAULTS; //PWM计算结构体
PWMGEN pwm2 = PWMGEN_DEFAULTS; //PWM计算结构体
QEP qep1 = QEP_DEFAULTS;       //QEP计算结构体
QEP qep2 = QEP_DEFAULTS;       //QEP计算结构体
SPEED_MEAS_QEP speed1 = SPEED_MEAS_QEP_DEFAULTS; //电机转速计算结构体
SPEED_MEAS_QEP speed2 = SPEED_MEAS_QEP_DEFAULTS; //电机转速计算结构体

//#if(CURVEGENER==TRAEZOIDGEN)
CURVEGEN curve1 = CURVEGEN_DEFAULTE;//梯形曲线发生器，闭环电机
CURVEGEN curve2 = CURVEGEN_DEFAULTE;//梯形曲线发生器，闭环电机
//#endif
TRANSFER_PARA PA = PA_DEFAULTS; //控制参数结构体PA初始化
MOTOR_CTL MC = MOTOR_CTL_DEFAULTS;//压脚电机控制参数结构体
MOTOR_CTL MB = MOTOR_CTL_DEFAULTS;//摆杆电机控制参数结构体
//foc控制相关结构体
FOC foc1 = FOC_DEFAULTS; //foc控制相关结构体
FOC foc2 = FOC_DEFAULTS; //foc控制相关结构体

PI_CONTROLLER pi_id  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_iq  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_spd = PI_CONTROLLER_DEFAULTS;

PI_CONTROLLER pi_id2  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_iq2  = PI_CONTROLLER_DEFAULTS;
PI_CONTROLLER pi_spd2 = PI_CONTROLLER_DEFAULTS;
//一阶低通滤波
LOWPASS_FILTER Spdref_filter1 = LOWPASS_FILTER_DEFAULTS;//转速指令低通滤波
LOWPASS_FILTER Spdref_filter2 = LOWPASS_FILTER_DEFAULTS;//转速指令低通滤波
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
    InitI2C_NOINT(); //I2C初始化--DSP2803x_I2C.c
    if((DIP1==0)&&(DIP2==0)) //初始化参数
    {
    	Wri_Rst_Par();  //写入初始控制参数标志
    }
    Motor_Par_Init();  //上电后,从EEPROM中读入控制参数
    PA.F1_Value = PA.F1;
    PA.F2_Value = PA.F2;
#endif
// step1: Initialize ADC module
    InitAdc();         //ADC采样零位基准校正(TI-OTP)
    ADC_INIT_MACRO()   //ADC模块初始化

	//多次采样，避开上电ADC采样不稳定的状态
	ADC_OFFSET_Reset_MACRO(adc1)
	ADC_OFFSET_Reset_MACRO(adc1)
	ADC_OFFSET_Reset_MACRO(adc1)

	ADC_OFFSET_Reset_MACRO(adc2)
	ADC_OFFSET_Reset_MACRO(adc2)
	ADC_OFFSET_Reset_MACRO(adc2)

	//---------ADC采样基准故障检测------------//
	adc1.ADC_Index=1;
	ADC_OFFSET_INIT_MACRO(adc1)     //ADC采样零位基准校正,AD零偏
	ADC_OFFSET_CHEKE_MACRO(adc1)    //ADC采样基准故障检测

	adc2.ADC_Index=2;
	ADC_OFFSET_INIT_MACRO(adc2)     //ADC采样零位基准校正,AD零偏
    ADC_OFFSET_CHEKE_MACRO(adc2)    //ADC采样基准故障检测
    /*
	if(MC.S_ERR==0)
	{
		if((adc1.ADC_OFFSETA_ERR==1)&&(adc1.ADC_OFFSETB_ERR==1))
		{
			MC.Err_Value = 10;//故障码--f10(A,B相ADC采样基准故障)
			MC.S_ERR=1;//置故障标志
			MC.Err_Wri_Ena=1;
		}
		else if(adc1.ADC_OFFSETA_ERR==1)
		{
			MC.Err_Value = 8;//故障码--f08(A相ADC采样基准故障)
			MC.S_ERR=1;//置故障标志
			MC.Err_Wri_Ena=1;
		}
		else if(adc1.ADC_OFFSETB_ERR==1)
		{
			MC.Err_Value = 9;//故障码--f09(B相ADC采样基准故障)
			MC.S_ERR=1;//置故障标志
			MC.Err_Wri_Ena=1;
		}

		if((adc2.ADC_OFFSETA_ERR==1)&&(adc2.ADC_OFFSETB_ERR==1))
		{
			MB.Err_Value = 10;//故障码--f10(A,B相ADC采样基准故障)
			MC.S_ERR=2;//置故障标志
			MB.Err_Wri_Ena=1;
		}
		else if(adc2.ADC_OFFSETA_ERR==1)
		{
			MB.Err_Value = 8;//故障码--f08(A相ADC采样基准故障)
			MC.S_ERR=2;//置故障标志
			MB.Err_Wri_Ena=1;
		}
		else if(adc2.ADC_OFFSETB_ERR==1)
		{
			MB.Err_Value = 9;//故障码--f09(B相ADC采样基准故障)
			MC.S_ERR=2;//置故障标志
			MB.Err_Wri_Ena=1;
		}
	}
*/
//step2: Initialize PWM module
	pwm1.PeriodMax = SYSTEM_FREQUENCY*1000000*T/2; //PWM开关周期（1500--20kHz）
    PWM_INIT_MACRO(pwm1) //ePWM初始化
    PWM1_OFF_MACRO()      //关闭mosfet

    pwm2.PeriodMax = SYSTEM_FREQUENCY*1000000*T/2; //PWM开关周期（1500--20kHz）
	PWM2_INIT_MACRO(pwm2) //ePWM初始化
	PWM2_OFF_MACRO()      //关闭mosfet

    //LineControlInit();//drv8880驱动芯片PWM管脚配置

    //设置中断向量表
    EALLOW;
    PieVectTable.EPWM1_INT = &PWM1ISR;//(这里是PWM1 40K下溢中断，足够快了，就不用周期中断，即不使用双更新)
    //PieVectTable.EPWM6_INT = &PWM6CAL;//这里只使用了PWM的定时功能LineControlInit
    EDIS;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;//对应PIE3.1--EPWM1_INT
    PieCtrlRegs.PIEIER3.bit.INTx6 = 1;//对应PIE3.6--EPWM6_INT
    IER |= M_INT3; // Enable CPU INT3 for EPWM1_INT

//step3: Initialize QEP module
    qep1.LineEncoder = EncoderLine;//编码器线数=1000
    qep1.MechScaler = _IQ30(0.25/qep1.LineEncoder);
    qep1.PolePairs  = Pole_Pairs;
    qep1.CalibratedAngle = 0;
    QEP_INIT_MACRO(qep1)

    qep2.LineEncoder = EncoderLine;//编码器线数=1000
	qep2.MechScaler = _IQ30(0.25/qep1.LineEncoder);
	qep2.PolePairs  = Pole_Pairs;
	qep2.CalibratedAngle = 0;
	QEP_INIT_MACRO(qep2)

	Angle_Par= POS_SET_MAX/2/200;//512线-> Angle_Par=128/25

//step4: Initialize the Speed module for QEP based speed calculation
    speed1.K1 = _IQ21( 1/(BASE_FREQ*(T*SpeedLoopPrescaler*SpeedCalPrescaler)));//采用电角度计算
    speed1.K2 = _IQ(1/(1+(SpeedLoopPrescaler*T*SpeedCalPrescaler)*(2*PI)*Speedfdb_filter_Freq));
    speed1.K3 = _IQ(1)-speed1.K2;
    speed1.BaseRpm = 60*(BASE_FREQ/Pole_Pairs);
    speed1.RpmScaler = _IQ(1.0/speed1.BaseRpm);//转速变换系数--1/2400

    speed2.K1 = _IQ21( 1/(BASE_FREQ*(T*SpeedLoopPrescaler*SpeedCalPrescaler)));//采用电角度计算
	speed2.K2 = _IQ(1/(1+(SpeedLoopPrescaler*T*SpeedCalPrescaler)*(2*PI)*Speedfdb_filter_Freq));
	speed2.K3 = _IQ(1)-speed2.K2;
	speed2.BaseRpm = 60*(BASE_FREQ/Pole_Pairs);
	speed2.RpmScaler = _IQ(1.0/speed2.BaseRpm);//转速变换系数--1/2400

//  step5: Initialize CAP1 module
#if(ECAP_Ena)
    InitECap(); //用于捕获主轴AB脉冲
#endif

//step6: 通讯类外设初始化（CAN,SCI,SPI）
#if(CAN_Ena)
    InitECana();
#endif

//step5: 采样信号滤波器设置

    //转速指令滤波---在转速环执行
    Spdref_filter1.K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Speedref_filter_Freq));
    Spdref_filter1.K3 = _IQ(1)-Spdref_filter1.K2;

    Spdref_filter2.K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Speedref_filter_Freq));
    Spdref_filter2.K3 = _IQ(1)-Spdref_filter2.K2;

    //电流指令滤波---在转速环执行
    foc1.Iqref_filter_K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Iqref_filter_Freq));  //滤波截止频率1000Hz
    foc1.Iqref_filter_K3 = _IQ(1)-foc1.Iqref_filter_K2;

    foc2.Iqref_filter_K2 = _IQ(1/( 1+(SpeedLoopPrescaler*T)*(2*PI)*Iqref_filter_Freq));  //滤波截止频率1000Hz
    foc2.Iqref_filter_K3 = _IQ(1)-foc2.Iqref_filter_K2;

    //电流反馈滤波---在电流环中执行
    foc1.Idqfdb_filter_K2 = _IQ(1/(1+(T)*(2*PI)*Idqfdb_filter_Freq));  //滤波截止频率1000Hz
    foc1.Idqfdb_filter_K3 = _IQ(1)-foc1.Idqfdb_filter_K2;

    foc2.Idqfdb_filter_K2 = _IQ(1/(1+(T)*(2*PI)*Idqfdb_filter_Freq));  //滤波截止频率1000Hz
    foc2.Idqfdb_filter_K3 = _IQ(1)-foc2.Idqfdb_filter_K2;

//step8: 控制参数初始化

    //电流环PI参数（#57 电阻1欧，电感  1.2mH）
    //--------倍频单极性8PWM-----------//
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
    pi_id.Umax = _IQ(0.707); //输出电压限幅值_IQ(0.707)
    pi_id.Umin = _IQ(-0.707);//输出电压限幅值_IQ(-0.707)

    pi_iq.Kp = pi_id.Kp;
    pi_iq.Ki = pi_id.Ki;
    pi_iq.Umax = _IQ(0.707); //输出电压限幅值0.707Bai_Gan
    pi_iq.Umin = _IQ(-0.707);//输出电压限幅值

    pi_id2.Kp = MB.Cur_Kp_Stop;
    pi_id2.Ki = MB.Cur_Ki_Stop;
    pi_id2.Umax = _IQ(0.7); //输出电压限幅值_IQ(0.707)
    pi_id2.Umin = _IQ(-0.7);//输出电压限幅值_IQ(-0.707)

    pi_iq2.Kp = pi_id2.Kp;
    pi_iq2.Ki = pi_id2.Ki;
    pi_iq2.Umax = _IQ(0.7); //输出电压限幅值0.707Bai_Gan
    pi_iq2.Umin = _IQ(-0.7);//输出电压限幅值

    //转速环PI参数
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

    //位置环比例通道参数配置
    MC.Pos_Kp_Stop = 25; //停机25
    MC.Pos_Kp_Work = 25; //工作25
    tPosReg_x.iEncoderLine = EncoderLine * 4;
    tPosPro_x.uKp_Pos = MC.Pos_Kp_Stop; //位置环比例增益PA3=24(0~100) PA.Pos_Kp_Par=25(信胜)
    tPosPro_x.uEnum = 1;
    tPosPro_x.uEden = 1;
    tPosPro_x.iPosCount_Err = 0; //位置计数器0xFFFFFFFF
    tPosPro_x.tErrCounter.lErrCount_Out = 0; //位置跟踪偏差

    MB.Pos_Kp_Stop = 10; //停机25
    //MB.Pos_Kp_Stop = 70; //停机保持与回原点增益相同 2019.04.19
    MB.Pos_Kp_Work = 25; //工作25
    tPosReg_y.iEncoderLine = EncoderLine * 4;
    tPosPro_y.uKp_Pos = MB.Pos_Kp_Work; //位置环比例增益PA3=24(0~100) PA.Pos_Kp_Par=25(信胜)
    tPosPro_y.uEnum = 1;
    tPosPro_y.uEden = 1;
    tPosPro_y.iPosCount_Err = 0; //位置计数器0xFFFFFFFF
    tPosPro_y.tErrCounter.lErrCount_Out = 0; //位置跟踪偏差

    //电流限幅值设置--_IQ(0.1)对应相电流峰值0.9A   0.75  0.4
    MC.Iq_ref_Limit_Work = _IQ(0.8); //工作时电流限幅 _IQ(0.3)--峰值2.7A
    MC.Iq_ref_Limit_Stop = _IQ(0.8); //停止时电流限幅 _IQ(0.1)--峰值0.9A
    MC.Iq_overload_Limit = _IQmpy(_IQ(0.8),MC.Iq_ref_Limit_Stop);//电流过载限幅

    MB.Iq_ref_Limit_Work = _IQ(0.8); //工作时电流限幅 _IQ(0.3)--峰值2.7A
    MB.Iq_ref_Limit_Stop = _IQ(0.8); //停止时电流限幅 _IQ(0.1)--峰值0.9A
    MB.Iq_overload_Limit = _IQmpy(_IQ(0.9),MB.Iq_ref_Limit_Stop);//电流过载限幅
    // BC520--IQ(1)--3A
    MC.Align_Idref = _IQ(0.7); //对应相电流峰值2.1A
    pi_spd.Umax =  MC.Iq_ref_Limit_Stop; //_IQ(0.4)对应相电流峰值2A
    pi_spd.Umin = -MC.Iq_ref_Limit_Stop;

    MB.Align_Idref = _IQ(0.7); //对应相电流峰值2.1A
    pi_spd2.Umax =  MB.Iq_ref_Limit_Stop; //_IQ(0.4)对应相电流峰值2A
    pi_spd2.Umin = -MB.Iq_ref_Limit_Stop;

    //上电定位参数设置
    MC.Align_Time = Ds_Align_Time;  //d轴定位时间2s
    MC.Align_Ticker = 0;
    MC.Speedref_Limit = _IQ(0.5);//电机转速限幅(2400*0.08=192rpm)0.4
    //MC.SpeedRef_Find_Z = _IQ(0.01);//找零位时电机转速(自测试模式下找原点转速)96rpm
    MC.SpeedRef_Find_Z = _IQ(0.05);
    MC.Work_Stop_Flag = 0;   //初始为停机状态
    MC.Gain_Switch_Ena = 1;  //"1"执行增益切换
    //MC.Pulse_curve_select=0;
    MC.P_Err_Clear=0;
    MC.Pos_Rst_Ticker=0;

    MB.Align_Time = Ds_Align_Time;  //d轴定位时间2s
    MB.Align_Ticker = 0;
    MB.Speedref_Limit = _IQ(0.4);//电机转速限幅(2400*0.08=192rpm)
    //MB.SpeedRef_Find_Z = _IQ(0.015);//找零位时电机转速(自测试模式下找原点转速)96rpm
    MB.SpeedRef_Find_Z = _IQ(0.05);
    MB.Work_Stop_Flag = 0;   //初始为停机状态
    MB.Gain_Switch_Ena = 0;  //"1"执行增益切换
   // MB.Pulse_curve_select=0;
    MB.P_Err_Clear=0;
    MB.Pos_Rst_Ticker=0;


    //YJ.Work_Pul_Out=YJ.Work_High*5/2;
    //MC.Motor_pos_set=YJ.Work_High*5/2;
   //BG.Work_Pul_Out=BG.Pos_Set*16;

    //梯形曲线


    curve1.IsrFrequency = ISR_FREQUENCY/SpeedLoopPrescaler;
    curve1.Kt = _IQ30(0.001/curve1.IsrFrequency);
    /*curve1.RunTime  = 100;
    curve1.RiseTime = 20;
    curve1.DownTime = 50; */
    curve1.RunTime  = 100;
    curve1.RiseTime = 20;
    curve1.DownTime = 30;
    curve1.GenFlg = 2;
    CurveInit(&curve1);//曲线发生器初始化
    if( (curve1.ErrFlg == 1)&&(MC.S_ERR==0) )
    {
    	MC.S_ERR=1;//置故障保护标志
    	MC.Err_Value = 12;// 故障码
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
    CurveInit(&curve2);//曲线发生器初始化
    if( (curve2.ErrFlg == 1)&&(MC.S_ERR==0) )
    {
	  MB.S_ERR=1;//置故障保护标志
	  MB.Err_Value = 12;// 故障码
    }


//step10: 由外部DIP1,2设定工作模式

    #if(Run_Mode==Open_Self_Run)
    {
        MC.RunMode = Init_Par;  //"2"--测试模式(允许在线升级)
        MC.Zero_Find_Ena = 0; //不使能找零位
        MC.EnableFlag = 1;    //由老化工装发送EnableFlag=1后，驱动器才能工作
    }
    #endif
    if(HS.loopmode == 0)
    {
        MC.RunMode = Init_Par;  //"2"--测试模式(允许在线升级)
        MC.Zero_Find_Ena = 0; //不使能找零位
        MC.EnableFlag = 1;    //由老化工装发送EnableFlag=1后，驱动器才能工作
    }
    if(TZ.loopmode == 0)
    {
        MB.RunMode = Init_Par;  //"2"--测试模式(允许在线升级)
        MB.Zero_Find_Ena = 0; //不使能找零位
        MB.EnableFlag = 1;    //由老化工装发送EnableFlag=1后，驱动器才能工作
    }

    if((DIP1==1)&&(DIP2==1))////DIP1=OFF DIP2=OFF
    {
        MC.RunMode = Run_Closeloop;//正常走位
        MB.RunMode = Run_Closeloop;//正常走位

        if(HS.loopmode == 1)
        {
            MC.Zero_Find_Ena = 1;//上电找零
        }

        MC.EnableFlag = 1; //电机使能
        MB.EnableFlag = 1; //电机使能

        TZ.Address = 1;//左装置
    }
    if((DIP1==1)&&(DIP2==0))//DIP1=OFF DIP2=ON
    {
        MC.RunMode = Run_Test;//闭环自测
        MB.RunMode = Run_Test;//闭环自测

        MC.Zero_Find_Ena = 1;//上电找零
        MB.Zero_Find_Ena = 1;//上电找零

        MC.EnableFlag = 1; //电机使能
        MB.EnableFlag = 1; //电机使能
    }
    if((DIP1==0)&&(DIP2==1))//DIP1=ON DIP2=OFF
    {
        /*
        MC.RunMode = Find_Z; //上电找零位(用于电机安装)
        MB.RunMode = Find_Z; //上电找零位(用于电机安装)

        MC.Zero_Find_Ena = 1;//上电找零
	    MB.Zero_Find_Ena = 1;//上电找零

	    MC.EnableFlag = 1; //电机使能
	    MB.EnableFlag = 1; //电机使能
         */

        MC.RunMode = Run_Closeloop;//正常走位
        MB.RunMode = Run_Closeloop;//正常走位

        if(HS.loopmode == 1)
        {
            MC.Zero_Find_Ena = 1;//上电找零
        }

        MC.EnableFlag = 1; //电机使能
        MB.EnableFlag = 1; //电机使能

        TZ.Address = 2;//右装置
    }
    if((DIP1==0)&&(DIP2==0))//DIP1=ON DIP2=OFF
    {
        MC.RunMode = Init_Par;//初始化控制参数
        MB.RunMode = Init_Par;//初始化控制参数

	    MC.EnableFlag = 0; //电机使能
	    MB.EnableFlag = 0; //电机使能
    }
} //end of Motor_Ctrl_Init


//---PWM1中断(40K下溢中断)---//
#pragma CODE_SECTION(PWM1ISR, "ramfuncs");
interrupt void PWM1ISR(void)
{
	if(MC.EnableFlag==1)//
    {
	   Run_Step_MotorY();//步进电机控制
    }
    if(MB.EnableFlag==1)//
    {
       Run_Step_MotorB();//步进电机控制
    }
    if(motorC.EnableFlag == 1)
    {
        Run_Step_MotorC();//开环拌珠电机1
    }
    if(motorD.EnableFlag == 1)
    {
        Run_Step_MotorD();//开环拌珠电机2
    }
    if(motorE.EnableFlag == 1)
    {
        Run_Step_MotorE();//拨叉开环电机
    }

    //PWM1中断处理
    EPwm1Regs.ETCLR.bit.INT = 1;//清中断标志
    PieCtrlRegs.PIEACK.all  = PIEACK_GROUP3;//清中断应答标志
} //end of Main_ISR

/****************************************************
      电机外环控制计算函数
****************************************************/
#pragma CODE_SECTION(Run_Step_MotorY, "ramfuncs");
static inline void Run_Step_MotorY(void)
{
    if(HS.loopmode == 1)
    {
        //------------------------压脚-------------------------------------
        if(MC.Zero_Find_Ena==1)
        {
            Zero_Find_Handle_A();
        }

        //------外环控制流程-------//
        if(MC.Align_Ticker < MC.Align_Time)//1s
        {   //电机初始定位阶段
            MC.Align_Ticker++;
            SpeedLoopCnt1=0;//外环控制流程计数值清零
        }
        else
        {   //电机闭环控制阶段(1,2,3,4)
            if(SpeedLoopCnt1>=SpeedLoopPrescaler)
                SpeedLoopCnt1=1;
            else
                SpeedLoopCnt1++;  //外环控制流程计数值
        }

        //"0":初始定位; “1~2”:矢量闭环控制
        switch(SpeedLoopCnt1)
        {
        //--------------------------------------------//
        case 0://初始定位切换

            //d轴电流指令
            if(pi_id.Ref <= MC.Align_Idref)
            {
                pi_id.Ref += _IQ(0.00004);//初始定位时，电流以斜率增加
                if(pi_id.Ref >= MC.Align_Idref)
                    pi_id.Ref = MC.Align_Idref;
            }

            pi_iq.Ref = 0; //q轴电流指令

            qep1.CalibratedAngle = (EQep1Regs.QPOSCNT > qep1.LineEncoder*2)? \
                    (EQep1Regs.QPOSCNT - qep1.LineEncoder*4):EQep1Regs.QPOSCNT;
            //------------------------------------------------------------------------------------

            //上电后采用两次定位，先定位到-90度，再定位到0度
            if(MC.Align_Ticker<= (MC.Align_Time>>1))
            {
                MC.Align_ElecTheta = _IQ(-0.25);//d轴定位时,设定电角度为-90
            }
            else if( MC.Align_Ticker >(MC.Align_Time>>1) && (MC.Align_Ticker < MC.Align_Time) )
            {
                MC.Align_ElecTheta += _IQ(0.00001);
                if( MC.Align_ElecTheta>=0)
                    MC.Align_ElecTheta = 0;    //再定位到0度
                qep1.ElecTheta = 0;
            }

            if(MC.Align_Ticker == MC.Align_Time)//需再确定其用途
            {
                qep1.IndexSyncFlag = 0x00;
                //为了防止DSP复位后，零位标志初始值不正确，程序中强制清零零位标志位IEL
                EQep1Regs.QCLR.bit.IEL = 1; //Clear interrupt flag
            }
            else
            {
                qep1.IndexSyncFlag = 0x0F;
            }//定位过程中，防止经过原点位置，误以为找到原点

            break;
            //-----------------------------------------------------------//
        case 1:
            if(curve1.Cur_mode==0)
            {
                if(curve1.GenFlg==2)
                {
                    if(MC.Pulse_set_Ena==1) //idle状态下才允许写入新的脉冲指令
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

            tPosPro_x.iPosCount_Err += curve1.OutputPulseErr;//增量给定

            //读取编码器反馈值
            tPosReg_x.lFeedBack = EQep1Regs.QPOSCNT;

            if(MC.Servo_OFF == 1)
            {
                tPosPro_x.uCLR = 1; //位置计数器清零
            }
            else
            {
                tPosPro_x.uCLR = MC.P_Err_Clear; //位置计数器清零
            }

            //位置环计算1--运行时间（66*16.7ns=1.1us)
            POS_PD_X1Macro()  //位置环计算1(脉冲偏差计算)

            //读取位置跟踪偏差（位置超差保护使用）
            MC.P_Out_Err = tPosPro_x.tErrCounter.lErrCount_Out;

            //根据工作状态，切换电流指令限幅值及控制增益
            Gain_Switch_Handle_Y();
            break;
            //-------------------------------------------------------//
        case 2:
            //位置环计算2--运行时间（107*16.7ns=1.78us)
            POS_PD_X2Macro()   //位置环计算2--/*脉冲误差计算(计算需3us)*/
            break;
            //--------------------------------------------------------//
        case 3:  //转速反馈计算--运行时间（109*16.7=2.01us）
            POS_PD_X3Macro()   //位置环计算3--/*位置环FAL计算*/

            //对速度指令进行限幅
            if(MC.Pos_Rst_Ticker<10)//限制电机上电回位过程的电机转速2400*0.01=24rpm
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
            //对速度指令低通滤波
            LOWPASS_FILTER_MACRO(Spdref_filter1)

            //转速环指令选择
            if(MC.Zero_Find_Ena==0)
            {
                pi_spd.Ref = Spdref_filter1.output;
            } //滤波后的转速指令
            else if(MC.Zero_Find_Ena==1)//找电机零位转速指令=60rpm
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
        case 4://速度环计算--运行时间(73~130*16.7ns=2.54us)

            //(1)转速计算误差=60*转速计算频率/(编码器线数*4)
            //(2)转速计算误差=60*1000/(1000*4)=15rpm (1000线编码器）

            //电机转速计算
            SpeedCalCnt1++;
            if(SpeedCalCnt1>=SpeedCalPrescaler) //电机转速计算
            {
                SpeedCalCnt1 = 0;
                speed1.ElecTheta = qep1.ElecTheta;//读取电角度计算转速
                SPEED_FR_MACRO(speed1)  //转速计算(M法)
                pi_spd.Fdb = speed1.Speed;//转速反馈
            }

            //转速环PI计算
            PI_MACRO(pi_spd)

            //q轴电流指令--转速环输出的电流指令滤波
            pi_iq.Ref = _IQmpy(foc1.Iqref_filter_K2,pi_iq.Ref)+\
            _IQmpy(foc1.Iqref_filter_K3,pi_spd.Out);

            //d轴电流指令
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
            pi_id.Ref = 0;//; //不需弱磁-_IQ(0.05)

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
        }//end of 外环控制
    }


        //----电流环控制----//
        //step1: Park变换角度计算(运行时间=103*16.667ns=1.716us)
    if(HS.loopmode == 1)
    {
        //计算反馈编码器角度(55*16.7=0.92us)
        QEP1_MACRO(qep1)  //采用计数到最大值复位eQEP计数器,检查是否过I或者S
                       if(SpeedLoopCnt1==0)//对应d轴定位状态
                       {foc1.Angle = MC.Align_ElecTheta;} //定位电角度
                       else
                       {foc1.Angle = qep1.ElecTheta;} //(采用编码器反馈电角度）
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
        //坐标变换用sin,cos计算
        foc1.Sine   = _IQsinPU(foc1.Angle);
        foc1.Cosine = _IQcosPU(foc1.Angle);

        //step2: ADC电流采样(运行时间=40*16.667ns=0.667us)
        //采样结果adc1.Ia,adc1.Ib,adc1.Udc
        ADC1_READ_MACRO(adc1)
        //**********采集观测数据************//
        /*if(Dlog_en == 1)
        {
            curve_run_flag[1]++;
            if(curve_run_flag[1]>=40)//40--1ms
            {
                curve_run_flag[1]=0;

                //if(curve_run_flag[2]>=100)
                    //curve_run_flag[2]=0;

                if(curve_run_flag[2]<100)//100个数据
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


        //**********采集观测数据end************//

        //**********软件相电流过流保护(两相电机)************//
        static Uint16 OC_Delay_Ia=0;
        static Uint16 OC_Delay_Ib=0;

        //Ia过流检测
        if(_IQ15abs(adc1.Ia)>=Over_Current_Limit)
        {
            OC_Delay_Ia++;
            if(OC_Delay_Ia>OC_Delay_Set)
            {
                PWM1_OFF_MACRO()
                                OC_Delay_Ia=OC_Delay_Set;
                if(MC.S_ERR == 0)
                {
                    MC.Err_Value = 2;//故障码--f02(A相过流)
                    MC.S_ERR=1;//置故障标志
                    MC.Err_Wri_Ena=1; //使能故障码写入EEPROM
                }
            }
        }
        else
        {
            OC_Delay_Ia=0;
        }

        //Ib过流检测
        if(_IQ15abs(adc1.Ib)>=Over_Current_Limit) //相电流过流检测
        {
            OC_Delay_Ib++;
            if(OC_Delay_Ib>OC_Delay_Set)
            {
                PWM1_OFF_MACRO()
                                OC_Delay_Ib=OC_Delay_Set;
                if(MC.S_ERR == 0)
                {
                    MC.Err_Value = 2;//故障码--f02(B相过流)
                    MC.S_ERR=1;//置故障标志
                    MC.Err_Wri_Ena=1; //使能故障码写入EEPROM
                }
            }
        }
        else
        {
            OC_Delay_Ib=0;
        }

        //step3: PARK变换(运行时间=36*16.667ns=0.6us)
        foc1.I_Alpha = _IQ15toIQ(adc1.Ia);//Ia_IQ24;
        foc1.I_Beta  = _IQ15toIQ(adc1.Ib);//Ib_IQ24;
        PARK_MACRO(foc1) //park变换(两相步进电机控制时使用）

        //step4: d电流环PI计算(运行时间=49*16.667ns=0.817us)
        pi_id.Fdb = _IQmpy(foc1.Idqfdb_filter_K2,pi_id.Fdb)+\
        _IQmpy(foc1.Idqfdb_filter_K3,foc1.I_Ds);

        PI_MACRO(pi_id) //d轴PI计算

        /*********Uq_Limit计算*******/
        register _iq Uq_Limit;
        Uq_Limit = _IQsqrt( _IQ(0.95) - _IQmpy(pi_id.Out,pi_id.Out) );
        pi_iq.Umax =  Uq_Limit;
        pi_iq.Umin = -Uq_Limit;


        //step5: q电流环PI计算(运行时间=49*16.667ns=0.817us)
        pi_iq.Fdb = _IQmpy(foc1.Idqfdb_filter_K2,pi_iq.Fdb)+\
                _IQmpy(foc1.Idqfdb_filter_K3,foc1.I_Qs);

        PI_MACRO(pi_iq) //q轴PI计算

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

        //step6: IPARK变换(运行时间=40*16.667ns=0.667us)
        foc1.U_Ds = pi_id.Out;//Ud_ref
        foc1.U_Qs = pi_iq.Out;//Uq_ref
        IPARK_MACRO(foc1)

        //step7: SVPWM and PWM占空比计算
        pwm1.Ualpha = foc1.U_Alpha;
        pwm1.Ubeta  = foc1.U_Beta;
        PWM1_H8_MACRO(pwm1)//8PWM占空比计算(占空比与输出电平反向）

        //step8: 故障保护
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
        //------------------------摆杆-------------------------------------
        if(MB.Zero_Find_Ena==1)//找零位
        {
            Zero_Find_Handle_B();
        }

        //------外环控制流程-------//
        if(MB.Align_Ticker < MB.Align_Time)//1s
        {   //电机初始定位阶段
            MB.Align_Ticker++;
            SpeedLoopCnt2=0;//外环控制流程计数值清零
        }
        else
        {   //电机闭环控制阶段(1,2,3,4)
            if(SpeedLoopCnt2>=SpeedLoopPrescaler)
                SpeedLoopCnt2=1;
            else
                SpeedLoopCnt2++;  //外环控制流程计数值
        }

        //"0":初始定位; “1~4”:矢量闭环控制
        switch(SpeedLoopCnt2)
        {
        //--------------------------------------------//
        case 0://初始定位切换
            //------------------------------------------------------------------------------------
            //摆杆 d轴电流指令
            if(pi_id2.Ref <= MB.Align_Idref)
            {
                pi_id2.Ref += _IQ(0.00004);//初始定位时，电流以斜率增加
                if(pi_id2.Ref >= MB.Align_Idref)
                    pi_id2.Ref = MB.Align_Idref;
            }

            pi_iq2.Ref = 0; //q轴电流指令

            qep2.CalibratedAngle = (EQep2Regs.QPOSCNT > qep2.LineEncoder*2)? \
                    (EQep2Regs.QPOSCNT - qep2.LineEncoder*4):EQep2Regs.QPOSCNT;

            //上电后采用两次定位，先定位到-90度，再定位到0度
            if(MB.Align_Ticker<= (MB.Align_Time>>1))
            {
                MB.Align_ElecTheta = _IQ(-0.25);//d轴定位时,设定电角度为-90
            }
            else if( MB.Align_Ticker >(MB.Align_Time>>1) && (MB.Align_Ticker < MB.Align_Time) )
            {
                MB.Align_ElecTheta += _IQ(0.00001);
                if( MB.Align_ElecTheta>=0)
                    MB.Align_ElecTheta = 0;    //再定位到0度
                qep2.ElecTheta = 0;
            }

            if(MB.Align_Ticker == MB.Align_Time)//需再确定其用途
            {
                qep2.IndexSyncFlag = 0x00;
                //为了防止DSP复位后，零位标志初始值不正确，程序中强制清零零位标志位IEL
                EQep2Regs.QCLR.bit.IEL = 1; //Clear interrupt flag
            }
            else
            {
                qep2.IndexSyncFlag = 0x0F;
            }//定位过程中，防止经过原点位置，误以为找到原点

            break;
            //-----------------------------------------------------------//
        case 1:
            if(curve2.Cur_mode==0)
            {
                if(curve2.GenFlg==2)
                {
                    if(MB.Pulse_set_Ena==1) //idle状态下才允许写入新的脉冲指令
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

            tPosPro_y.iPosCount_Err += curve2.OutputPulseErr;//增量给定

            //读取编码器反馈值
            tPosReg_y.lFeedBack = EQep2Regs.QPOSCNT;

            tPosPro_y.uCLR = MB.P_Err_Clear; //位置计数器清零
            //位置环计算1--运行时间（66*16.7ns=1.1us)
            POS_PD_Y1Macro()  //位置环计算1(脉冲偏差计算)

            //读取位置跟踪偏差（位置超差保护使用）
            MB.P_Out_Err = tPosPro_y.tErrCounter.lErrCount_Out;

            //根据工作状态，切换电流指令限幅值及控制增益
            Gain_Switch_Handle_B();
            break;
            //-------------------------------------------------------//
        case 2:
            //位置环计算2--运行时间（107*16.7ns=1.78us)
            POS_PD_Y2Macro()   //位置环计算2--/*脉冲误差计算(计算需3us)*/
            break;
            //--------------------------------------------------------//
        case 3:  //转速反馈计算--运行时间（109*16.7=2.01us）
            //case 2:
            POS_PD_Y3Macro()   //位置环计算3--/*位置环FAL计算*/

            //对速度指令进行限幅
            if(MB.Pos_Rst_Ticker<10)//限制电机上电回位过程的电机转速2400*0.01=24rpm
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

            //对速度指令低通滤波
            LOWPASS_FILTER_MACRO(Spdref_filter2)

            //转速环指令选择
            if(MB.Zero_Find_Ena==0)
            { pi_spd2.Ref = Spdref_filter2.output;} //滤波后的转速指令
            else if(MB.Zero_Find_Ena==1)//找电机零位转速指令=60rpm
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
        case 4://速度环计算--运行时间(73~130*16.7ns=2.54us)

            //(1)转速计算误差=60*转速计算频率/(编码器线数*4)
            //(2)转速计算误差=60*1000/(1000*4)=15rpm (1000线编码器）

            //电机转速计算
            SpeedCalCnt2++;
            if(SpeedCalCnt2>=SpeedCalPrescaler) //电机转速计算
            {
                SpeedCalCnt2= 0;
                speed2.ElecTheta = qep2.ElecTheta;//读取电角度计算转速
                SPEED_FR_MACRO(speed2)  //转速计算(M法)
                pi_spd2.Fdb = speed2.Speed;//转速反馈
            }

            //转速环PI计算
            PI_MACRO(pi_spd2)

            //q轴电流指令--转速环输出的电流指令滤波
            pi_iq2.Ref = _IQmpy(foc2.Iqref_filter_K2,pi_iq2.Ref)+\
            _IQmpy(foc2.Iqref_filter_K3,pi_spd2.Out);

            pi_id2.Ref = 0;//; //不需弱磁-_IQ(0.05)

            break;
            //--------------------------------------------------//
        default: break;
        }//end of 外环控制
    }

    if(TZ.loopmode == 1)
    {
        //----电流环控制----//
        //step1: Park变换角度计算(运行时间=103*16.667ns=1.716us)

        //计算反馈编码器角度(55*16.7=0.92us)
        QEP2_MACRO(qep2)  //采用计数到最大值复位eQEP计数器,检查是否过I或者S
                if(SpeedLoopCnt2==0)//对应d轴定位状态
                {foc2.Angle = MB.Align_ElecTheta;} //定位电角度
                else
                {foc2.Angle = qep2.ElecTheta;} //(采用编码器反馈电角度）
    }
    else
    {
        if(TZ.Address == 1)
        {
            if(TZ.Direction == 0)
            {
                //foc2.Angle -= _IQ(0.001);//加快
                foc2.Angle -= _IQ(0.0001);//拨叉前进方向
            }
            else
            {
                //foc2.Angle += _IQ(0.001);//加快
                foc2.Angle += _IQ(0.0001);//拨叉回退方向
            }
        }
        else if(TZ.Address == 2)
        {
            if(TZ.Direction == 0)
            { foc2.Angle += _IQ(0.0001);}//拨叉前进方向
            else
            { foc2.Angle -= _IQ(0.0001);}//拨叉回退方向
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
    //坐标变换用sin,cos计算
    foc2.Sine   = _IQsinPU(foc2.Angle);
    foc2.Cosine = _IQcosPU(foc2.Angle);

    //step2: ADC电流采样(运行时间=40*16.667ns=0.667us)
    //采样结果adc1.Ia,adc1.Ib,adc1.Udc
    ADC2_READ_MACRO(adc2)
    //**********采集观测数据************//

    if(Dlog_en == 1)
    {
        curve_run_flag[1]++;
        if(curve_run_flag[1]>=20)//20--1ms
        {
            curve_run_flag[1]=0;

            //if(curve_run_flag[2]>=100)
            //   curve_run_flag[2]=0;

            if(curve_run_flag[2]<100)//100个数据
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
    //**********采集观测数据end************//

    //**********软件相电流过流保护(两相电机)************//
    static Uint16 OC_Delay_Ia=0;
    static Uint16 OC_Delay_Ib=0;

    //Ia过流检测
    if( _IQ15abs(adc2.Ia)>=Over_Current_Limit)
    {
        OC_Delay_Ia++;
        if(OC_Delay_Ia>OC_Delay_Set)
        {
            PWM2_OFF_MACRO()
        	                OC_Delay_Ia=OC_Delay_Set;
            if(MB.S_ERR == 0)
            {
                MB.Err_Value = 2;//故障码--f02(A相过流)
                MB.S_ERR=1;//置故障标志
                MB.Err_Wri_Ena=1; //使能故障码写入EEPROM
            }
        }
    }
    else
    {
        OC_Delay_Ia=0;
    }

    //Ib过流检测
    if( _IQ15abs(adc2.Ib)>=Over_Current_Limit) //相电流过流检测
    {
        OC_Delay_Ib++;
        if(OC_Delay_Ib>OC_Delay_Set)
        {
            PWM2_OFF_MACRO()
                            OC_Delay_Ib=OC_Delay_Set;
            if(MB.S_ERR == 0)
            {
                MB.Err_Value = 2;//故障码--f02(B相过流)
                MB.S_ERR=1;//置故障标志
                MB.Err_Wri_Ena=1; //使能故障码写入EEPROM
            }
        }
    }
    else
    {
        OC_Delay_Ib=0;
    }

    //step3: PARK变换(运行时间=36*16.667ns=0.6us)
    foc2.I_Alpha = _IQ15toIQ(adc2.Ia);//Ia_IQ24;
    foc2.I_Beta  = _IQ15toIQ(adc2.Ib);//Ib_IQ24;
    PARK_MACRO(foc2) //park变换(两相步进电机控制时使用）

    //step4: d电流环PI计算(运行时间=49*16.667ns=0.817us)
    pi_id2.Fdb = _IQmpy(foc2.Idqfdb_filter_K2,pi_id2.Fdb)+\
    _IQmpy(foc2.Idqfdb_filter_K3,foc2.I_Ds);

    PI_MACRO(pi_id2) //d轴PI计算

    /*********Uq_Limit计算*******/
    register _iq Uq_Limit;
    Uq_Limit = _IQsqrt( _IQ(0.95) - _IQmpy(pi_id2.Out,pi_id2.Out) );
    pi_iq2.Umax =  Uq_Limit;
    pi_iq2.Umin = -Uq_Limit;

    //step5: q电流环PI计算(运行时间=49*16.667ns=0.817us)
    pi_iq2.Fdb = _IQmpy(foc2.Idqfdb_filter_K2,pi_iq2.Fdb)+\
            _IQmpy(foc2.Idqfdb_filter_K3,foc2.I_Qs);

    PI_MACRO(pi_iq2) //q轴PI计算

    //step6: IPARK变换(运行时间=40*16.667ns=0.667us)
    foc2.U_Ds = pi_id2.Out;//Ud_ref
    foc2.U_Qs = pi_iq2.Out;//Uq_ref
    IPARK_MACRO(foc2)

    //step7: SVPWM and PWM占空比计算
    pwm2.Ualpha = foc2.U_Alpha;
    pwm2.Ubeta  = foc2.U_Beta;
    PWM2_H8_MACRO(pwm2)//8PWM占空比计算(占空比与输出电平反向）

    //step8: 故障保护
    if(MB.S_ERR==1)
    {PWM2_OFF_MACRO()}
}
/**************************************************************
//控制增益切换函
**************************************************************/
#pragma CODE_SECTION(Gain_Switch_Handle_Y, "ramfuncs");
static inline void Gain_Switch_Handle_Y(void)//控制增益切换函数
{
	//step1:  电机到位判断
	#define Pos_Limit_Set     3    //电机到位偏差判断值
	#define Motor_Ready_Time_Set 15  //判断时间=1ms*10=10ms
	static  Uint16 Motor_Ready_Delay=0;  //(motor1.step_state==0)

	if((MC.S_ERR==0)&&(curve1.GenFlg==2)\
			&&(abs(MC.P_Out_Err)<=Pos_Limit_Set) )
	{
		Motor_Ready_Delay++;
		if(Motor_Ready_Delay>=Motor_Ready_Time_Set)
		{
			Motor_Ready_Delay = Motor_Ready_Time_Set;
			MC.Work_Stop_Flag = 0;//电机已到位
		}
	}
	else
	{
		Motor_Ready_Delay = 0;
		MC.Work_Stop_Flag = 1;//电机没有到位
	}

	//step2: 增益切换
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
			/*******工作状态*******/
			//位置环比例
			tPosPro_x.uKp_Pos = MC.Pos_Kp_Work; //位置环比例PA.Pos_Kp=35--IQ(35/32)=IQ(1.09)

			//转速环比例及积分
			pi_spd.Kp = MC.Spd_Kp_Work;
			pi_spd.Ki = MC.Spd_Ki_Work;

			//电流指令限幅值
			pi_id.Kp = MC.Cur_Kp_Work;
			pi_id.Ki = MC.Cur_Ki_Work;
			pi_iq.Kp = pi_id.Kp;
			pi_iq.Ki = pi_id.Ki;

			MC.Iq_overload_Limit = _IQmpy(_IQ(0.9),MC.Iq_ref_Limit_Work);//工作电流过载限幅
			pi_spd.Umax =  MC.Iq_ref_Limit_Work;
			pi_spd.Umin = -MC.Iq_ref_Limit_Work;
		}
		else
		{
			/*****停车状态******/
			//位置环比例
			tPosPro_x.uKp_Pos = MC.Pos_Kp_Stop; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

			//转速环比例及积分
			pi_spd.Kp = MC.Spd_Kp_Stop;
			pi_spd.Ki = MC.Spd_Ki_Stop;

			//电流环比例及积分
			pi_id.Kp = MC.Cur_Kp_Stop;
			pi_id.Ki = MC.Cur_Ki_Stop;
			pi_iq.Kp = pi_id.Kp;
			pi_iq.Ki = pi_id.Ki;

			//电流电压限幅
			MC.Iq_overload_Limit = _IQmpy(_IQ(0.6),MC.Iq_ref_Limit_Stop);//停车电流过载限幅
			pi_spd.Umax =  MC.Iq_ref_Limit_Stop;
			pi_spd.Umin = -MC.Iq_ref_Limit_Stop;
		}
	}
}

/*************************************************************/
#pragma CODE_SECTION(Gain_Switch_Handle_B, "ramfuncs");
static inline void Gain_Switch_Handle_B(void)//控制增益切换函数
{
	//step1:  电机到位判断
	#define Pos_Limit_Set     3    //电机到位偏差判断值
	#define Motor_Ready_Time_Set 15  //判断时间=1ms*10=10ms
	static  Uint16 Motor_Ready_Delay=0;  //(motor1.step_state==0)

	if((MB.S_ERR==0)&&(curve2.GenFlg==2)\
			&&(abs(MB.P_Out_Err)<=Pos_Limit_Set) )
	{
		Motor_Ready_Delay++;
		if(Motor_Ready_Delay>=Motor_Ready_Time_Set)
		{
			Motor_Ready_Delay = Motor_Ready_Time_Set;
			MB.Work_Stop_Flag = 0;//电机已到位
		}
	}
	else
	{
		Motor_Ready_Delay = 0;
		MB.Work_Stop_Flag = 1;//电机没有到位
	}

	//step2: 增益切换
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
			/*******工作状态*******/
			//位置环比例
			tPosPro_y.uKp_Pos = MB.Pos_Kp_Work; //位置环比例PA.Pos_Kp=35--IQ(35/32)=IQ(1.09)

			//转速环比例及积分
			pi_spd2.Kp = MB.Spd_Kp_Work;
			pi_spd2.Ki = MB.Spd_Ki_Work;

			//电流指令限幅值
			pi_id2.Kp = MB.Cur_Kp_Work;
			pi_id2.Ki = MB.Cur_Ki_Work;
			pi_iq2.Kp = pi_id2.Kp;
			pi_iq2.Ki = pi_id2.Ki;

			//MB.Iq_overload_Limit = _IQmpy(_IQ(0.9),MB.Iq_ref_Limit_Work);//工作电流过载限幅
			MB.Iq_overload_Limit = _IQmpy(_IQ(0.8),MB.Iq_ref_Limit_Work);//工作电流过载限幅
			pi_spd2.Umax =  MB.Iq_ref_Limit_Work;
			pi_spd2.Umin = -MB.Iq_ref_Limit_Work;
		}
		else
		{
			/*****停车状态******/
			//位置环比例
		    //if(TZ.Position_state != 1)
		    //{
		        tPosPro_y.uKp_Pos = MB.Pos_Kp_Stop; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

		        //转速环比例及积分
		        pi_spd2.Kp = MB.Spd_Kp_Stop;
		        pi_spd2.Ki = MB.Spd_Ki_Stop;
		    //}
		   /* else
		    {
		        tPosPro_y.uKp_Pos = 50; //Pos_Kp=25(0~100)--_IQ(25/32)=IQ(0.78)

		        //转速环比例及积分
		        pi_spd2.Kp = _IQ(5.0);
		        pi_spd2.Ki = _IQ(0.01);
		    }*/

			//电流环比例及积分
			pi_id2.Kp = MB.Cur_Kp_Stop;
			pi_id2.Ki = MB.Cur_Ki_Stop;
			pi_iq2.Kp = pi_id2.Kp;
			pi_iq2.Ki = pi_id2.Ki;

			//电流电压限幅
			//MB.Iq_overload_Limit = _IQmpy(_IQ(0.6),MB.Iq_ref_Limit_Stop);//停车电流过载限幅
			MB.Iq_overload_Limit = _IQmpy(_IQ(0.8),MB.Iq_ref_Limit_Stop);//停车电流过载限幅
			pi_spd2.Umax =  MB.Iq_ref_Limit_Stop;
			pi_spd2.Umin = -MB.Iq_ref_Limit_Stop;
		}
	}
}

//电机自转控制函数
#pragma CODE_SECTION(Self_Run_Test, "ramfuncs");
void Self_Run_Test(void) //for 老化测试
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
                  //正常自转运行
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
					  MC.Motor_pos_set = -4000; //正转3圈
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
						  MC.Motor_pos_set = 4000; //正转3圈
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
    //非正正常换色时，找零位动作超时检测
#define Zero_Find_OverTime_Set 120000 //找Z超时时间=0.025ms*120000=3s

    static Uint32 Zero_Find_Delay=0;

    if((SpeedLoopCnt1!=0)&&(MC.S_ERR==0))
    {
        Zero_Find_Delay++;
        if(Zero_Find_Delay>=Zero_Find_OverTime_Set)
        {
            Zero_Find_Delay=0;
            MC.S_ERR=1;//置故障保护标志
            MC.Err_Value = 11;// 故障码-f11(找零位超时)
            MC.Err_Wri_Ena =1;//故障信息写入EEPROM使能
        }
    }

    MC.P_Err_Clear = 1;//位置计数器清零
    if(qep1.IndexSyncFlag == 0x00F0)//找到电机零位Z
    {
        qep1.IndexSyncFlag = 0x0F;//0x0F标志已完成找零位

        Zero_Find_Delay=0;

        MC.Zero_Find_Ena  = 0; //清找零位使能标志
        MC.Zero_Find_Flag = 1; //置找到Z信号标志
        MC.P_Err_Clear = 0; //位置计数器恢复不清零
        //为了校正编码器Z信号偏差，电机转动位置偏差角度
#if(Znc_Z_offset_Ena)
        //采用直接发指令脉冲的方式，补偿Z角度偏差(测试OK)
        //编码器Z信号宽度4*4，补偿值为Z信号宽度的一半
#define Enc_Z_offset_Set1  8//-2    //齿轮比4:1

        //根据找零位时电机转到方向，确定补偿Z角度偏差
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
    //非正正常换色时，找零位动作超时检测
#define Zero_Find_OverTime_Set_B 120000 //找Z超时时间=0.025ms*120000=3s

    static Uint32 Zero_Find_Delay_B=0;

    if((SpeedLoopCnt2!=0)&&(MB.S_ERR==0))
    {
        Zero_Find_Delay_B++;
        if(Zero_Find_Delay_B>=Zero_Find_OverTime_Set_B)
        {
            Zero_Find_Delay_B=0;
            MB.S_ERR=1;//置故障保护标志
            MB.Err_Value = 11;// 故障码-f11(找零位超时)
            MB.Err_Wri_Ena =1;//故障信息写入EEPROM使能
        }
    }

    MB.P_Err_Clear = 1;//位置计数器清零
    if(qep2.IndexSyncFlag == 0x00F0)//找到电机零位Z
    {
        qep2.IndexSyncFlag = 0x0F;//0x0F标志已完成找零位

        Zero_Find_Delay_B=0;

        MB.Zero_Find_Ena  = 0; //清找零位使能标志
        MB.Zero_Find_Flag = 1; //置找到Z信号标志
        MB.P_Err_Clear = 0; //位置计数器恢复不清零
        //为了校正编码器Z信号偏差，电机转动位置偏差角度
#if(Znc_Z_offset_Ena)
        //采用直接发指令脉冲的方式，补偿Z角度偏差(测试OK)
        //编码器Z信号宽度4*4，补偿值为Z信号宽度的一半
#define Enc_Z_offset_Set2  15//-2    //齿轮比4:1

        //根据找零位时电机转到方向，确定补偿Z角度偏差
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
        IRQHandler_SPTA(&motorC);//运行至最大速度后一直输出脉冲，直至检测到到位信号
        if(motorC.SPTA_PULSE >= 1)
        {
            M1_CLK = 1;//换色电机
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
                motorC.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=200000;
                break;
            case 1:
                motorC.step_spmax=17000;
                motorC.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=250000;
                break;
            case 2:
                motorC.step_spmax=19000;
                motorC.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=250000;
                break;
            case 3:
                motorC.step_spmax=21000;
                motorC.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 4:
                motorC.step_spmax=23000;
                motorC.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 5:
                motorC.step_spmax=25000;
                motorC.step_accel_up=350000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 6:
                motorC.step_spmax=29000;
                motorC.step_accel_up=350000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 7:
                motorC.step_spmax=33000;
                motorC.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 8:
                motorC.step_spmax=37000;
                motorC.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            case 9:
                motorC.step_spmax=40000;
                motorC.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorC.step_accel_down=300000;
                break;
            }
/*
            //motorC.step_spmax=30000;//适合富伟开环单头机 2017.06.15
            motorC.step_accel_up=800000;//尝试加大加速度，换色时噪音减小 2017.02.16
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
        IRQHandler_SPTA(&motorD);//运行至最大速度后一直输出脉冲，直至检测到到位信号
        if(motorD.SPTA_PULSE >= 1)
        {
            M2_CLK = 1;//换色电机
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
                motorD.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=200000;
                break;
            case 1:
                motorD.step_spmax=17000;
                motorD.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=250000;
                break;
            case 2:
                motorD.step_spmax=19000;
                motorD.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=250000;
                break;
            case 3:
                motorD.step_spmax=21000;
                motorD.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 4:
                motorD.step_spmax=23000;
                motorD.step_accel_up=300000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 5:
                motorD.step_spmax=25000;
                motorD.step_accel_up=350000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 6:
                motorD.step_spmax=29000;
                motorD.step_accel_up=350000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 7:
                motorD.step_spmax=33000;
                motorD.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 8:
                motorD.step_spmax=37000;
                motorD.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
            case 9:
                motorD.step_spmax=40000;
                motorD.step_accel_up=400000;//针对朱工提出下珠效率低修改，提高升速
                motorD.step_accel_down=300000;
                break;
                /*            case 0:
                motorD.step_spmax=10000;//适合富伟开环单头机 2017.06.15
                break;
            case 1:
                motorD.step_spmax=10000;//适合富伟开环单头机 2017.06.15
                break;
            case 2:
                motorD.step_spmax=15000;//适合富伟开环单头机 2017.06.15
                break;
            case 3:
                motorD.step_spmax=20000;//适合富伟开环单头机 2017.06.15
                break;
            case 4:
                motorD.step_spmax=25000;//适合富伟开环单头机 2017.06.15
                break;
            case 5:
                motorD.step_spmax=30000;//适合富伟开环单头机 2017.06.15
                break;
            case 6:
                motorD.step_spmax=35000;//适合富伟开环单头机 2017.06.15
                break;
            case 7:
                motorD.step_spmax=40000;//适合富伟开环单头机 2017.06.15
                break;
            case 8:
                motorD.step_spmax=40000;//适合富伟开环单头机 2017.06.15
                break;
            case 9:
                motorD.step_spmax=40000;//适合富伟开环单头机 2017.06.15
                break;*/
            }
/*
            //motorD.step_spmax=30000;//适合富伟开环单头机 2017.06.15
            motorD.step_accel_up=800000;//尝试加大加速度，换色时噪音减小 2017.02.16
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
        IRQHandler_SPTA(&motorE);//运行至最大速度后一直输出脉冲，直至检测到到位信号
        if(motorE.SPTA_PULSE >= 1)
        {
            //M3_CLK = 1;//换色电机
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

//3——函数定义
#pragma CODE_SECTION(Push_Pearl, "ramfuncs");
#if 0
Uint16 Push_Pearl(int Dir)
{
    static Uint16 Pearl_Position_State = 0;
    Pearl_Position_State++;
    if(Pearl_Position_State == 1)
    {
/*        curve2.RunTime  = 80;//不能改为100，机电配合会有问题
        curve2.RiseTime = 20;
        curve2.DownTime = 20;*/
        //curve2.RunTime  = 60;//不能改为100，机电配合会有问题
        curve2.RunTime  = 70;//佳宇朱工反馈做4.0珠子拨叉回退太快，需要配合改慢， 2019.05.20
        curve2.RiseTime = 5;
        //curve2.DownTime = 10;
        curve2.DownTime = 5;//佳宇朱工反馈做4.0珠子拨叉回退有明显减速，需要配合加快， 2019.05.26
        curve2.GenFlg = 2;
        CurveInit(&curve2);//曲线发生器初始化
        //tPosPro_y.uKp_Pos = 60;
       //tPosPro_y.uKp_Pos = 75;//通用版：固定位置
        //tPosPro_y.uKp_Pos = 63;//T版：立即停模式
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
        CurveInit(&curve2);//曲线发生器初始化
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
        //通用版
        //curve2.RunTime  = 70;
        //curve2.RiseTime = 20;
        //curve2.DownTime = 10;
        //T版
/*        curve2.RunTime  = 80;//不能改为100，机电配合会有问题
        curve2.RiseTime = 20;
        curve2.DownTime = 20;*/
        //curve2.RunTime  = 60;//正常版 2019.04.28
        curve2.RunTime  = 70;//佳宇朱工反馈做4.0珠子拨叉回退太快，需要配合改慢， 2019.05.20
        curve2.RiseTime = 5;
        //curve2.DownTime = 10;//正常版 2019.04.28
        curve2.DownTime = 5;//佳宇朱工反馈做4.0珠子拨叉回退有明显减速，需要配合加快， 2019.05.26
        curve2.GenFlg = 2;
        CurveInit(&curve2);//曲线发生器初始化
        tPosPro_y.uKp_Pos = 30;
       //tPosPro_y.uKp_Pos = 70;//通用版：固定位置
        //tPosPro_y.uKp_Pos = 65;//T版：立即停模式
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

        //记录任何时候（绣作和手动）珠子颜色
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
    TZ.HandTZDelay = 0;//清除切小电流的定时变量
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
            //1000rpm设计参数 50ms
            motorE.step_spmax=50000;//适合富伟开环单头机 2017.06.15
            motorE.step_accel_up=20000000;//尝试加大加速度，换色时噪音减小 2017.02.16
            motorE.step_accel_down=20000000;
        }
        else if(Embroider.Speed <= 85)
        {
            //设计参数 60ms
            motorE.step_spmax=40000;//适合富伟开环单头机 2017.06.15
            motorE.step_accel_up=15000000;//尝试加大加速度，换色时噪音减小 2017.02.16
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
            //1000rpm设计参数 50ms
            motorE.step_spmax=60000;//50000
            motorE.step_accel_up=25000000;//20000000
            motorE.step_accel_down=25000000;//20000000
        }
        else if(Embroider.Speed <= 85)
        {
            //设计参数 60ms
            motorE.step_spmax=30000;//适合富伟开环单头机 2017.06.15
            motorE.step_accel_up=10000000;//尝试加大加速度，换色时噪音减小 2017.02.16
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

        //记录任何时候（绣作和手动）珠子颜色
        TZ.color = HS.Color_state;

        Pearl_Position_State = 2;

        break;
    case 3:
        if((Embroider.Speed > 85)&&(Embroider.Speed <= 105))
        {
            //1000rpm设计参数 50ms
            motorE.step_spmax=60000;//50000
            motorE.step_accel_up=25000000;//20000000
            motorE.step_accel_down=25000000;//20000000
        }
        else if(Embroider.Speed <= 85)
        {
            //设计参数 60ms
            motorE.step_spmax=40000;//适合富伟开环单头机 2017.06.15
            motorE.step_accel_up=15000000;//尝试加大加速度，换色时噪音减小 2017.02.16
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
        //设计参数 60ms
        motorE.step_spmax=10000;//适合富伟开环单头机 2017.06.15
        motorE.step_accel_up=1500000;//尝试加大加速度，换色时噪音减小 2017.02.16
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
