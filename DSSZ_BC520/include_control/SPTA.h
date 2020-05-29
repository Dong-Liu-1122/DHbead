/* =================================================================================
File name:       SPTA.H 
===================================================================================*/


#ifndef __SPTA_H__
#define __SPTA_H__

/*SPTA����״̬����*/
#define IDLE				0
#define ACCELERATING		1
#define AT_MAX				2
#define DECELERATING		3
#define STOP                4

/*SPTA�㷨���ƽṹ��*/
typedef struct
{
	Uint16  enable;        		//ʹ��
	Uint16  dir;			  	//����
	Uint16  running;		  	//ת����ɱ�־
	Uint16  reseting;  			//��λ��־��Ϊ1ʱ����λ����ǿͣ��
	Uint16  divnum;		  		//��Ƶ��
	Uint16  speedenbale;		//�Ƿ�ʹ���ٶȿ���
	/*Uint16  clockwise;			//˳ʱ�뷽���Ӧ��ֵ*/

	Uint32 step_move    ;		//total move requested,�趨���еĲ���(ָ������ֵ��
	Uint32 step_count   ;		//step counter;�Ѿ����еĲ���
	
	Uint32 step_acced   ;		//steps in acceled stage;���ٽ׶εļ��ٲ���
	Uint32 step_middle  ;		//mid-point of move, = (step_move - 1) >> 1;�趨���в�����1/2�������ò�����Ҫ����
	
	//Uint32 step_accel   ;		//accel/decel rate, 8.8 bit format���趨�ĵ�����ٶ���ֵ
	Uint32 step_accel_up   ;    //accel/decel rate, 8.8 bit format���趨�ĵ�����ٶ���ֵ,���ٶ�
	Uint32 step_accel_down ;    //accel/decel rate, 8.8 bit format���趨�ĵ�����ٶ���ֵ,���ٶ�
	Uint32 speed_frac   ;		//speed counter fraction;�ٶ�Ƭ���ۼ�����ÿ�ζ��ۼ�step_accel������ֵ����ĳ��ֵ��step_speed����
	
	Uint32 step_speed   ;		//current speed, 16.8 bit format (HI byte always 0)������Ƭ���ۼ��ٶ�ֵ��speed_frac���ֵ�ӵ�����
    Uint32 step_frac    ;		//step counter fraction;����Ƭ���ۼ�����ÿ���ۼ�step_speed������ֵ����ĳ��ֵ�����һ���������壬����һ��

	Uint32 step_spmax   ;		//maximum speed,�趨�ĵ������ٶ�
	Uint16 step_state   ;		//move profile state;�����ת״̬���������١����С�����

/*
	Uint32 CurrentPosition;		 //��ǰλ�ã�����ϸ��ǰ�Ĳ���ͳ��
	Uint32 MaxPosition;			 //���λ�ã�������λ����0
	Uint32 CurrentPosition_Pulse;//��ǰλ�ã�����������ͳ�ƣ�CurrentPosition_Pulse=CurrentPosition*ϸ����
	Uint32 MaxPosition_Pulse;	 //���λ�ã�������λ����0
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

extern MOTOR_CONTROL_SPTA motorC;  //����C���
extern MOTOR_CONTROL_SPTA motorD;  //����D���
extern MOTOR_CONTROL_SPTA motorE;  //����E���

extern void IRQHandler_SPTA(MOTOR_CONTROL_SPTA *pmotor);
void Start_Motor_SPTA(Uint16 dir,Uint16 Degree,MOTOR_CONTROL_SPTA *pmotor);

typedef struct{
	Uint16 curve1;
	Uint16 curve2;
	Uint16 curve3;
	Uint16 curve4;
}DISTINGUISH;

#define DISTINGUISH_DEFAULTE {0,0,0,0}

//#define CPU_FREQUENCE		72000000	//CPUƵ��
//#define F2TIME_PARA		12000000   	//��Ƶ��ֵת��Ϊ��ʱ���Ĵ���ֵ��ת������

#define STEP_SPTA			20			//SPTA����ٶȵȼ�

#define STEP_AA				45       	//�Ӽ��ٽ׶Σ���ɢ������
#define STEP_UA				60			//�ȼ��ٽ׶Σ���ɢ������
#define STEP_RA				30			//�����ٽ׶Σ���ɢ������

#define STEP_AR				30       	//�Ӽ��ٽ׶Σ���ɢ������
#define STEP_UR				60			//�ȼ��ٽ׶Σ���ɢ������
#define STEP_RR				45			//�����ٽ׶Σ���ɢ������



#endif // __SPTA_H__

