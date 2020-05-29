/* =================================================================================
File name:       SPTA.H 
===================================================================================*/


#ifndef __SPTA_H__
#define __SPTA_H__

/*SPTA控制状态定义*/
#define IDLE				0
#define ACCELERATING		1
#define AT_MAX				2
#define DECELERATING		3
#define STOP                4

/*SPTA算法控制结构体*/
typedef struct
{
	Uint16  enable;        		//使能
	Uint16  dir;			  	//方向
	Uint16  running;		  	//转动完成标志
	Uint16  reseting;  			//复位标志，为1时，限位开关强停。
	Uint16  divnum;		  		//分频数
	Uint16  speedenbale;		//是否使能速度控制
	/*Uint16  clockwise;			//顺时针方向对应的值*/

	Uint32 step_move    ;		//total move requested,设定运行的步数(指令输入值）
	Uint32 step_count   ;		//step counter;已经运行的步数
	
	Uint32 step_acced   ;		//steps in acceled stage;加速阶段的加速步数
	Uint32 step_middle  ;		//mid-point of move, = (step_move - 1) >> 1;设定运行步数的1/2，超过该步数就要减速
	
	//Uint32 step_accel   ;		//accel/decel rate, 8.8 bit format；设定的电机加速度数值
	Uint32 step_accel_up   ;    //accel/decel rate, 8.8 bit format；设定的电机加速度数值,升速段
	Uint32 step_accel_down ;    //accel/decel rate, 8.8 bit format；设定的电机加速度数值,减速段
	Uint32 speed_frac   ;		//speed counter fraction;速度片段累加器，每次都累加step_accel，该数值大于某个值后step_speed增加
	
	Uint32 step_speed   ;		//current speed, 16.8 bit format (HI byte always 0)；步数片段累加速度值，speed_frac溢出值加到这里
    Uint32 step_frac    ;		//step counter fraction;步数片段累加器，每次累加step_speed，该数值大于某个值后产生一个步进脉冲，运行一步

	Uint32 step_spmax   ;		//maximum speed,设定的电机最大速度
	Uint16 step_state   ;		//move profile state;电机运转状态变量，加速、运行、减速

/*
	Uint32 CurrentPosition;		 //当前位置，按照细分前的步数统计
	Uint32 MaxPosition;			 //最大位置，超过该位置置0
	Uint32 CurrentPosition_Pulse;//当前位置，按照脉冲数统计，CurrentPosition_Pulse=CurrentPosition*细分数
	Uint32 MaxPosition_Pulse;	 //最大位置，超过该位置置0
*/

	//User definition
	int32 SPTA_PULSE;
	int32 SPTA_PULSE_OLD;
	int32 SPTA_PULSE_ERR;
	Uint16 PulseEnable;

	Uint16 DisableFlag;
	Uint16 EnableFlag;
	Uint16 Degree;
	Uint16 TempTest;
	Uint16 TempFlag;
 
} MOTOR_CONTROL_SPTA ;

#define MOTOR_CONTROL_SPTA_DEFAULTE {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6400,0,0}

extern MOTOR_CONTROL_SPTA motorC;  //拌珠C电机
extern MOTOR_CONTROL_SPTA motorD;  //拌珠D电机
extern MOTOR_CONTROL_SPTA motorE;  //拌珠E电机

extern void IRQHandler_SPTA(MOTOR_CONTROL_SPTA *pmotor);
void Start_Motor_SPTA(Uint16 dir,Uint16 Degree,MOTOR_CONTROL_SPTA *pmotor);

typedef struct{
	Uint16 curve1;
	Uint16 curve2;
	Uint16 curve3;
	Uint16 curve4;
}DISTINGUISH;

#define DISTINGUISH_DEFAULTE {0,0,0,0}

//#define CPU_FREQUENCE		72000000	//CPU频率
//#define F2TIME_PARA		12000000   	//将频率值转换为定时器寄存器值得转换参数

#define STEP_SPTA			20			//SPTA最大速度等级

#define STEP_AA				45       	//加加速阶段，离散化点数
#define STEP_UA				60			//匀加速阶段，离散化点数
#define STEP_RA				30			//减加速阶段，离散化点数

#define STEP_AR				30       	//加减速阶段，离散化点数
#define STEP_UR				60			//匀减速阶段，离散化点数
#define STEP_RR				45			//减减速阶段，离散化点数



#endif // __SPTA_H__

