/* =========================================================================
File name:  Cut_Thread.H
Originator:	Digital Control Systems Group Texas Instruments
Description: 勾线控制头文件
============================================================================
 History: 2016.01.30	Version 1.0
==========================================================================  */
#ifndef YaJiao_H
#define YaJiao_H
/***************************************************/

//定义勾线命令
#define Motion_YJ_Stop      0  //动作命令：电机停止
#define Motion_YJ_Start     1  //动作命令：电机启动
#define Cmd_YJ_Hand_Clear   0  //动作命令：手动指令清除
#define Cmd_YJ_Hand_High    1  //动作命令：手动到最高点
#define Cmd_YJ_Hand_Low     2  //动作命令:手动到最低点
#define Cmd_YJ_Run_Pusle    3  //动作命令：电机转指定脉冲数


#define  State_ERR   100  //故障状态
#define  State_STOP  110  //停机状态
#define  State_IDLE  10   //等待状态
#define  State_INIT  0   //等待状态
#define  State_Zero_Calib 1 //零位矫正状态
#define  State_ALIGN  2   //对齐状态
#define  State_FIND_Z  3   //状态
#define  State_CNT_RISE  4   //计数动作状态
#define  State_CNT_DROP  5   //计数动作状态
#define   State_Wait     6
#define   State_WORK_Rdy 7
#define   State_WORK     8







#define  Rise_Main_Pos      387    //压脚启动，上升时主轴位置   (240-100)/360*1000=1550
#define  High_Main_Pos      2833    //压脚工作高度最高点时主轴位置   (240+115-100)/360*4000=1550
#define  Drop_Main_Pos      708    //压脚工作高度最低点（到位）时主轴位置   (240+230-100)/360*1000=1550
#define  Work_Distance      1283    //工作高度对应主轴脉冲数量=High_Main_Pos-Rise_Main_Pos
//定义勾线状态反馈(DLC=2)
#define GX_OK          0xF706  //勾线到位
#define GX_ERR         0xF707  //勾线不到位

//定义勾线状态反馈(DLC=8)
#define GX_OK_DLC8          1  //勾线到位
#define GX_ERR_DLC8         0  //勾线不到位 2

//定义剪线故障码
#define ErrNum_OverTime_Open      1   //勾线电机动作超时
#define ErrNum_OverTime_Find_Z    2   //勾线电机回原点超时
#define ErrNum_OverTime_Hand    2   //勾线电机回原点超时
//读取GX原点输入信号,"0"勾线信号低有效

//剪线控制参数结构体
typedef struct
{
    Uint16 AB_Pulse;         //输入:指令
    int16  Pos_Set;     //压脚高度设定
    int16  Pos_Real;     //当前位置
    int16  Pos_Off;
    Uint16 Ctl_Ena;          //编码器Z信号状态，“0”为Z信号有效，对应红色指示灯亮
    Uint16 High_Index;    //高度类别，1为限位高度;0为工作高度

    int16 Max_High;    //限位高度
    Uint16 Work_High;    //工作高度
    int16 Low_High;    //最低点调整
    int16 Work_Pul_Out;    //工作高度对应脉冲数量=Work_High*2.5
    //控制状态标志
    Uint16 Motion_Cmd;    //计数动作指令
    Uint16 Hand_Cmd;    //压脚手动操作，0-压脚下落至最低点，1-压脚抬升至最高点
    Uint16 Motor_Ori_Flag;    //计数动作到最高点标志
    Uint16 Motor_Dir_Flag; //电机方向

    int32  Main_Shaft_Pos;  //输入：主轴位置脉冲计数0~3999)
    Uint16 Work_Flag; //电机方向
    Uint16  Main_Shaft_Pos_Last;  //输入：主轴位置脉冲计数0~3999)

    int16 Pulse_Num;
    int16 Pulse_e;
    int16 Pulse_Num_last;
    Uint16 Err_Value;    //输出：压脚故障码
    Uint16 State_now;    //内部变量：当前控制状态
    Uint16 State_next;   //内部变量：下一个控制状态
    int32  Delay;        //内部变量：延时计数
    Uint16 Pos_max_min;
}YAJIAO;  //控制参数结构体

#define YAJIAO_DEFAULTS { 0,0,0,0,\
                               0,0,70,30,\
                               0,120,0,0,\
                               0,0,0,0,\
                               0,0,0,0,\
                               0,0,0,0,0}

extern YAJIAO YJ;
extern void YJ_Control(void);  //完成勾线控制(计数模式)


//剪线控制参数结构体
typedef struct
{
    Uint16 AB_Pulse;    //AB脉冲计数
    Uint16 Type;        //0:平绣 1：特种绣
    Uint16 ControlState;//0:正绣 1：补绣
    Uint16 ControlStateTrg;
    Uint16 ControlStateEna;
    Uint16 StartStop;      //0:拉杆停止 1：拉杆启动
    Uint16 StartStopFlag;  //0:拉杆停止 1：拉杆启动
    Uint16 Init;        //0：主控未初始化 1：主控初始化完成

    Uint16 AirValvePos; //1:装置在上位 0：装置在下位
    Uint16 AirValveState;//1:抬装置 0：降装置
    Uint16 LightState;  //1:关机头  2：绿灯 3：红灯
    Uint16 WorkState;   //
    Uint16 PearlNeedle; //0:非散珠针位 1：散珠针位
    Uint16 Speed;
    Uint16 LightFlag;
    Uint16 NoEmbFlag;
    Uint16 LightStateOld;
}EMBROIDER;  //控制参数结构体

#define EMBROIDER_DEFAULTS {0,0,0,0,0,0,0,0,\
                            1,1,0,0,0,0,0,0,0}

extern EMBROIDER Embroider;

#endif
