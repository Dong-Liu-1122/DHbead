/* =========================================================================
File name:  Ctrl_Para.H
Originator:	Digital Control Systems Group Texas Instruments
Description: 控制器参数头文件
============================================================================
 History: 2017.03.18	Version 1.0
==========================================================================  */
#ifndef Ctrl_Para_H
#define Ctrl_Para_H

//------Step1: 电机运行状态定义---------//

//Idle--;Run--正常运行;Err--故障停机
enum Motor_State {ST_Idle=0,ST_Run=1,ST_Err_Y=2,ST_Err_B=3,ST_RDY=4,wait=5};

//Init_Par--初始化参数 ;Find_Z--找编码器零位(用于初始安装);
//Run_Test--测试模式(允许在线升级);Run_Closeloop--闭环工作(出厂默认模式)
enum RunMode {Init_Par=0,Find_Z=1,Run_Test=2,Run_Closeloop=3,Idle=4};


//------Step2:电机控制参数结构体(MC)---------//
typedef struct
{
    //控制状态标志
    Uint16 EnableFlag;  //使能标志（1表示使能）
    enum RunMode RunMode;   //控制模式(DIP1);“0”只找零位; "1"正常换色;
                            //“2”用于初始化参数--以及测试模式
    Uint16 Run_Test_Ena;  //自测试使能
    Uint16 Run_Test_Flag;  //自测试标志
    enum Motor_State State; //电机当前运行状态
    Uint16 Self_Run_Ena; //电机自转使能（用于老化测试)
    Uint16 Run_time; //程序运行时间
    Uint16 RunMotor;    //电机运行使能

    Uint16 S_ERR;       //故障标志（1表示有故障）
    Uint16 Err_Value;   //故障信息
    Uint16 Err_Wri_Ena;  //故障信息写入EEPROM使能
    Uint16 Err_Clear_Ena;  //输入量：清历史故障指令
    Uint16 Par_Reset_Ena;  //参数复位使能指令
    Uint16 Stop_bit;       //“1”对应复位停机
    Uint16 Work_Stop_Flag;//"1"工作；“0”停机
    Uint16 Gain_Switch_Ena;//"1"执行增益切换

    int32  P_Out_Err;       //位置环跟踪偏差（位置过超保护用）
    Uint16 P_Err_Clear;
    int32  Motor_pos_set;   //位置指令脉冲个数
    Uint16 Pulse_set_Ena;   //位置指令脉冲数写入使能
    Uint16 Open_Motor_Dir;//脉冲曲线选择位

    _iq     SpeedRef_Find_Z;  //电机找零位时，电机转速IQ(-0.02)对应48rpm
    _iq     Speedref_Limit;   //最高转速指令限幅(1200rpm)
    Uint16  Zero_Find_Ena;    //电机找零位使能(上电后使能找零位）
    Uint16  Close_Ena;   //电机找零位成功标志，"1"代表找零位成功

    //控制环参数
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

    //电流限幅参数
    _iq  Iq_ref_Limit_Work;  //IQ(0.6)-电流峰值4A,电流指令限幅(由PA参数设定)
    _iq  Iq_ref_Limit_Stop;
    _iq  Iq_overload_Limit;  //过载保护电流限幅值

    //d轴定位参数
    _iq   Align_Idref;      //d轴定位电流(由PA参数设定)
    _iq   Align_ElecTheta;  //定位电角度
    int32 Align_Time;       //初始d轴定位时间=40000*25us=1s
    int32 Align_Ticker;     //定位时间计数
    int32 Pos_Rst_Ticker;   //电机位置回位定时计数
    Uint16 pulse_finish;

    Uint16 Zero_Find_Flag;
    Uint16 Pulse_curve_select;

    Uint16 Servo_OFF;//松锁轴标志

    //Uint16 RunTime;
   // Uint16 RiseTime;
   // Uint16 DownTime;

}MOTOR_CTL;  //控制参数结构体

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

extern MOTOR_CTL MC;//电机控制参数结构体


//------Step3:保存控制PA参数结构体(PA)---------//


typedef struct
{
	Uint16 F1;           //机头地址
	Uint16 F2;           //装置类型: 0
    Uint16 F3;           //A珠下珠速度
    Uint16 F4;           //软件版本: 01
    Uint16 F5;           //B珠下珠速度

    Uint16 Motor_Dir;
    Uint16 Address;

    Uint16 Err_num1;             //历史故障码1:
    Uint16 Err_num2;             //历史故障码2:
    Uint16 Err_num3;             //历史故障码3:
    Uint16 Err_num4;             //历史故障码4:

    Uint16 Hardware_Code;        //
    Uint16 Update_Flag;
    Uint16 F1_Value;
    Uint16 F2_Value;

}TRANSFER_PARA;  //控制参数结构体

#define PA_DEFAULTS {5,5,5,5,5,1,0,\
                     0,0,0,0,\
                     0x32,0,0,0}
extern TRANSFER_PARA PA;


//------Step4:CAN参数结构体定义(CAN)---------//
typedef struct
{
    Uint16 Rec[8];    //接收数据
    Uint16 Trs[8];    //发送数据
    Uint16 ID;        //本次接收数据ID
    Uint16 ID_last;   //上次接收数据ID
    Uint16 DLC;       //接收数据字节长度DLC
    Uint16 Rec_Flag;  //接收标志

    Uint16 Rx_Ticker;//CAN接收计数
    Uint16 Tx_Ticker;//CAN发送计数

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
//-----------SCI参数结构体定义--------------//
typedef struct
{
    Uint16 Tx[4];    //发送数据
    Uint16 Rx[4];    //接收数据
    Uint16 Tx_Ticker;//SCI发送计数
    Uint16 Rx_Ticker;//SCI接收计数
    Uint16 Rx_Err_Ticker;//SCI接收计数
    Uint16 New_sci_cmd; //新收到SCI命令标志

    Uint16 Tx_Ena; //SCI发送使能
    Uint16 Tx_Delay; //SCI发送延时

} SCI_Para;
#define SCI_DEFAULTS {{0},{0},\
                      0,0,0,0,\
                      0,0}
extern SCI_Para SCI;
extern void SCI_Handle(void);  //SCI通讯处理函数
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
	Uint16 AddressValue;    //机头地址常用值
    Uint16 StopValue;       //故障停车命令
    Uint16 AddressValueSend;//用于CAN发送时使用
    Uint16 StartCheck ; //开始检测
}HeaderAddress_PARA;

#define HeaderAddress_PARA_DEFAULTE {0,0,0,0,0,0,0,0,0,0,0,0}

//1:1.4:1传动比
//2:2:1传动比
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
    Uint16 All_START;//全头换珠指令
    Uint16 Address;
    int16 DeltaTime;//拨叉回退时机调整（0-60）
    Uint16 ParaEnable;//参数修改使能
    Uint16 ParaFlag;
    int16 Step1H;
    int16 Step1L;
    int16 DeltaTimeH;
    int16 DeltaTimeL;


    Uint16 All_START_State;//全头换珠状态
    Uint16 TestPauseEnable;
    Uint16 All_START_Delay;//全头换珠延时
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
    Uint16 Delay;//上电延时一段时间后根据EEPROM中的颜色状态进行纠正
    Uint16 ErrorStartFlag;
    Uint16 ErrCnt;
    Uint16 InitPosData;//换色上电找完原点后的编码器值
    Uint16 QEP_HisData;//上一次的换色电机编码器值

    int16 OFFPosData;//松轴前的换色位置
    int16 ONPosData; //锁轴前的换色位置
    int16 DeltaOFFON;//松锁轴过程偏差值
}HuanSe_PARA;
#define HuanSe_PARA_DEFAULTE {0,0,0,0,0,0,1,0,\
                              {-200,-160,-120,-80,-40,0,40,80,120,160},0,40000,0,0,0,0,\
                              0,0,0}
extern HuanSe_PARA HS;

#endif // end of Ctrl_Para_H
