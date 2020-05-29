/* =======================================================================
File name:  Stepper-Settings.H
Originator:	Digital Control Systems Group Texas Instruments
Description: 参数定义头文件
==========================================================================
 History: 2017.04.18	Version 1.0
==========================================================================*/
#ifndef PROJ_SETTINGS_H

//------Step1: Select the driver------------//
#define SY200     1
#define Driver   SY200

//------Step2: Select control mode----------//


//------Step3.1: 自定义头文件声明----------//
#include "F2806x_Examples.h"
#include "IQmathLib.h"
#include "Foc.h"       //控制算法结构体
#include "Ctrl_Para.h" //控制参数结构体
//#include "Pos_Regulator.h"   //位置环控制器头文件

//------Step3.2:DSP控制外设头文件声明 ---------//
#include "f2803xqep_Stepper.h"
#include "f2803xpwm_Drv8412.h"   //8PWM(倍频单极性PWM)
#include "f2803xileg_Hall_Drv8412.h"
#include "YaJiao.h"
#include "Baigan.h"
#include "HC595.h"
#include "SPTA.h"
#include "HandKey.h"
#include "DecryptHex.h"

//------Step4: 控制功能选择---------//
//定义控制功能
#define Open_Self_Run        0    // "0":开环自转
#define Close_Loop           1    // "1":闭环
#define Open_Loop           0    // "1":闭环
#define Run_Mode       Close_Loop // 电机控制模式
#define Run_Mode_MotorB       Close_Loop // 电机控制模式

#define Debug_Test           0    // "1"对应测试状态,"0"对应正常运行状态
#define Znc_Z_offset_Ena     1    // "1"对应使能编码器Z校正

//------Step5: DSP在线升级设置---------//
#define DSP_Flash_Update  (void (*)(void))(0x3F7FF6) //Boot引导程序升级入口地址(芯片固定地址)
#define DSP_Soft_Reset    (void (*)(void))(0x3F5FF6) //用户程序c_int0入口地址(for 28031)

//接收主控DLC=8
#define RX_D0    0xFC //接收主控D0
#define RX_D1    0xB0 //接收主控D1

//向主控发送DLC=8
#define TX_D0    0xF5 //向主控发数D0
#define TX_D1    0x07 //向主控发数D1

#define MachineControl GE
#define Ax8 1
#define GE  2

#define DSP_Update_Ena   1   // "1"对应DSP软件升级使能
#define Origa_Ena 1

#if(MachineControl == GE)
#define HardwareNumber  0x32 //硬件识别码
#define SoftWareVersion 0x01 //软件版本号
#elif(MachineControl == Ax8)
#define HardwareNumber  0x33 //硬件识别码
#define SoftWareVersion 0x01 //软件版本号
#endif

#define MachineType Double
#define Double 1
#define Three 2

//------Step6:FOC控制参数设置---------//
#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

#define HS_Distance 2080

#define PI 3.14159265358979
#define SYSTEM_FREQUENCY    90    //DSP28030 DSP_CLK=60MHz
#define Pole_Pairs          50    //步进电机极对数=50
#define BASE_FREQ           2000  //speed_Base=60*2000/50=2400rpm

#define EncoderLine         1000   //#42电机,编码器线数(1000线/转)1000

#define POS_SET_MAX         4000   //1600
#define POS_SET_MAX_HALF    2000   //1600

#define ISR_FREQUENCY       20     //PWM中断频率=20kHz(Drv8412)
#define SpeedLoopPrescaler   4     //转速环分频系数=20kHz/2=10kHz
#define SpeedCalPrescaler    2     //转速计算分频系数--转速计算频率=20kHz/(2*10)=1kHz
//软件过流保护
#define Over_Current_Limit   _IQ15(1.1)  //相电流过流保护值--IQ(0.1)--0.54A
#define OC_Delay_Set    100         //延时时间=25us*100=2.5ms
#define Ds_Align_Time   40000L      //d轴定位时间2s（20000--1s）
//转速环滤波
#define Speedref_filter_Freq  800  //转速指令低通滤波频率
#define Speedfdb_filter_Freq  200  //转速反馈低通滤波频率
//电流环滤波
#define Iqref_filter_Freq     1000 //电流指令低通滤波频率
#define Idqfdb_filter_Freq    1000 //电流反馈低通滤波频率

//---------Hardware Setup---------------//

//------Step1: DSP外设选择---------//
#define IIC_Ena        1  // IIC_Ena使能选择
#define CAN_Ena        1
#define ECAP_Ena       1
#define SCI_Ena       0
//------Step2: GPIO管脚定义---------//
//EEPROM地址
#define eprom_addr_Error        0x30   //故障码IIC地址
#define eprom_addr_Reset        0xF0   //写入初始化参数用IIC地址
#define eprom_addr_Motor_Dir    0x01   //IIC地址(0x1)

//输入IO量
//#define DIP1           GpioDataRegs.AIODAT.bit.AIO12   //DIP_on对应“0”低电平
//#define DIP2           GpioDataRegs.AIODAT.bit.AIO14   //DIP_on对应“0”低电平
//SY200--DIP硬件直接连到GND
//#define DIP1           GpioDataRegs.GPADAT.bit.GPIO12
//#define DIP2           GpioDataRegs.GPADAT.bit.GPIO13
//#define Enc_Z_IN       GpioDataRegs.GPADAT.bit.GPIO23    //编码器Z信号—IO，高电平时，红灯亮

//#define DIP1  1
//#define DIP2  1
#define DIP1  INPUT_HC597Regs.bit.DIP1
#define DIP2  INPUT_HC597Regs.bit.DIP2



//Drv8412相关控制信号
//#define Drv8412_FLT    GpioDataRegs.GPADAT.bit.GPIO5   //Drv8412故障信号,低有效
//#define Drv8412_OTW    GpioDataRegs.GPADAT.bit.GPIO3   //Drv8412过温信号,低有效

//#define Drv8412_RESET  GpioDataRegs.GPACLEAR.bit.GPIO7=1 //复位Drv8412,低有效
//#define Drv8412_ENA    GpioDataRegs.GPASET.bit.GPIO7=1   //使能Drv8412
#define Drv8412_RESET  GpioDataRegs.GPBCLEAR.bit.GPIO43=1 //复位Drv8412,低有效
#define Drv8412_ENA    GpioDataRegs.GPBSET.bit.GPIO43=1   //使能Drv8412

//#define Drv8412_FLT_B    GpioDataRegs.GPBDAT.bit.GPIO39   //Drv8412故障信号,低有效
//#define Drv8412_OTW_B    GpioDataRegs.GPADAT.bit.GPIO11   //Drv8412过温信号,低有效

//#define Drv8412_RESET_B  GpioDataRegs.GPBCLEAR.bit.GPIO41=1 //复位Drv8412,低有效
//#define Drv8412_ENA_B    GpioDataRegs.GPBSET.bit.GPIO41=1   //使能Drv8412
#define Drv8412_RESET_B  GpioDataRegs.GPACLEAR.bit.GPIO25=1 //复位Drv8412,低有效
#define Drv8412_ENA_B    GpioDataRegs.GPASET.bit.GPIO25=1   //使能Drv8412


//Led
#define LED_FLT_OUT    GpioDataRegs.GPBCLEAR.bit.GPIO34=1   //报错故障灯
//#define LED_KEY_GRN_OFF  GpioDataRegs.GPBSET.bit.GPIO43=1  //工作状态指示灯
#define LED_KEY_RED_OFF  GpioDataRegs.GPBSET.bit.GPIO44=1  //工作状态指示灯
//#define LED_KEY_GRN_ON  GpioDataRegs.GPBCLEAR.bit.GPIO43=1  //工作状态指示灯
#define LED_KEY_RED_ON  GpioDataRegs.GPBCLEAR.bit.GPIO44=1  //工作状态指示灯
//#define LED_RED_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO34=1  //工作状态指示灯
#define LED_RED_BLINK   GpioDataRegs.AIOTOGGLE.bit.AIO12=1  //工作状态指示灯

//#define LED_KEY_GRN_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO43=1  //工作状态指示灯
#define LED_KEY_RED_BLINK  GpioDataRegs.GPBTOGGLE.bit.GPIO44=1  //工作状态指示灯

#define  State_MA_ERR   100  //故障状态
#define  State_MA_STOP  110  //停机状态
#define  State_MA_IDLE  10   //等待状态
#define  State_MA_HAND  11   //对齐状态
#define  State_MA_REACH 12
#define  State_MA_FINDZ  0
#define  State_MA_INIT    1   //等待状态
#define  State_MA_Rdy     2   //状态
#define  State_MA_WORK    3   //状态
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
//输出IO量(onewire使用)
//#define OneWire_IO  GpioDataRegs.GPADAT.bit.GPIO22

//------Step6: 全局函数声明---------//
//#define CPU_CLOCK_SPEED      16.667L  //16.667L  for a 60MHz CPU clock speed
//#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_CLOCK_SPEED) - 9.0L) / 5.0L)
extern void DSP28x_usDelay(unsigned long Count);

//DSP外设初始化函数声明
extern void InitFlash(void);
extern void InitI2C_NOINT(void); //IIC初始化函数(查询法)
extern void InitAdc(void);
extern void Motor_Ctrl_Init(void);//电机控制初始化
extern void Motor_Fault_Check(void); //故障检测函数
extern void Motor_Par_Init(void);//EEPROM参数初始化
extern void Motor_Ctrl_FSM(void);
extern void Wri_Rst_Par(void);//写入参数复位标志,下次上电后执行参数复位
extern void Eeprom_Err_Clear(void);  //故障信息清零
extern void Eeprom_Err_Wri(void);  //故障信息写入EEPROM
extern void CAN_Handle(void);
extern void SCI_CMD_Handle(void);
//自测试相关函数
extern void Self_Run_Test(void); //电机闭环自转控制函数
//------Step7: 外部结构体变量声明---------//
extern Uint16 RamfuncsLoadStart,RamfuncsLoadEnd,RamfuncsRunStart;
extern Uint16 IQmathLoadStart,IQmathLoadEnd,IQmathRunStart;

//控制外设结构体
extern ADConvter adc1,adc2;  //AD采样结构体
extern PWMGEN pwm1,pwm2; //PWM计算结构体
extern QEP qep1,qep2;
extern SPEED_MEAS_QEP speed1,speed2;
//控制参数结构体
extern SPTA motor1;
extern MOTOR_CTL MC,MB;//电机控制参数结构体
extern TRANSFER_PARA PA; //控制参数结构体PA初始化
//foc控制相关结构体
extern FOC foc1,foc2; //foc控制相关结构体
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
