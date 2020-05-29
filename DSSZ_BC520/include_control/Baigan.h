/*
 * Baigan.h
 *
 *  Created on: 2017年8月7日
 *      Author: guozhenan
 */

#ifndef BAIGAN_H_
#define BAIGAN_H_

#define Cmd_BG_Hand_Clear   0  //动作命令：手动指令清除
#define Cmd_BG_Hand   1  //动作命令：手动到最高点
#define BG_Start_Main_Pos      260    //摆杆起始动作时主轴位置    (200-100)/360*4000=1111
#define BG_Finish_Main_Pos     3555   //上升时主轴位置   (200+220-100)/360*4000=3999
#define BG_Dist_Main_Pos     2444
//#define  BG_ST_MainPos      1111

#define Cmd_BG_Hand_Ori    1  //动作命令：手动到最高点
#define Cmd_BG_Hand_NoOri     2  //动作命令:手动到最低点

#define  State_BG_ERR      100  //故障状态
#define  State_BG_STOP     110  //停机状态
#define  State_BG_IDLE     10   //等待状态
#define  State_BG_INIT     0   //等待状态
#define  State_BG_ALIGN    2   //对齐状态
#define  State_BG_FIND_Z   3   //状态
#define  State_BG_TO_NORI  4   //计数动作状态
#define  State_BG_TO_ORI   5   //计数动作状态
#define  State_BG_Wait     6
#define  State_BG_WORK_Rdy 7
#define  State_BG_WORK     8

typedef struct
{
    Uint16 Ctl_Ena;         //输入:指令
    Uint16 ORIGN_Flag;       //上电发1次，告诉初始位置=1，在原点；=0，在非原点
    int16  Pos_Set;     //摆幅角度设定
    //int16  Work_Set;     //工作幅度设定
   // int16  Pos_Off;
    //Uint16 Enc_Z;          //编码器Z信号状态，“0”为Z信号有效，对应红色指示灯亮
   // Uint16 High_Index;    //高度类别，1为限位高度;0为工作高度
    //int16 Max_High;    //限位高度
   // Uint16 Work_High;    //工作高度
   // Uint16 Low_High;    //最低点调整
    int16 Work_Pul_Out;    //工作幅度对应脉冲数量
    //控制状态标志
    Uint16 Motion_Cmd;    //计数动作指令
    Uint16 Hand_Cmd;    //摆杆手动操作0-不动，1-动作
    Uint16 Motor_Ori_Staus;    //计数动作到最高点标志
    Uint16 Motor_Dir_Flag; //电机方向

    int32  Main_Shaft_Pos;  //输入：主轴位置脉冲计数0~3999)
    Uint16 Work_Flag; //电机动作
    Uint16  Main_Shaft_Pos_Last;  //输入：主轴位置脉冲计数0~3999)
   // Uint16 Rise_Main_Pos   //压脚上升时主轴位置
    Uint16 Pos_max_min;
   // int16 Pulse_e;
    int16 Pos_max_min_last;
    Uint16 Err_Value;    //输出：故障码
    Uint16 State_now;    //内部变量：当前控制状态
    Uint16 State_next;   //内部变量：下一个控制状态
    int32  Delay;        //内部变量：延时计数
    Uint16 Stop_Flag;    //停车标志
    Uint16 Distan_Real;    //计数动作指令
}BAIG;  //控制参数结构体

#define BG_DEFAULTS {0,0,50,200,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0}

extern BAIG BG;

#endif /* BAIGAN_H_ */
