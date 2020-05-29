/* =========================================================================
File name:  Cut_Thread.H
Originator:	Digital Control Systems Group Texas Instruments
Description: ���߿���ͷ�ļ�
============================================================================
 History: 2016.01.30	Version 1.0
==========================================================================  */
#ifndef YaJiao_H
#define YaJiao_H
/***************************************************/

//���年������
#define Motion_YJ_Stop      0  //����������ֹͣ
#define Motion_YJ_Start     1  //��������������
#define Cmd_YJ_Hand_Clear   0  //��������ֶ�ָ�����
#define Cmd_YJ_Hand_High    1  //��������ֶ�����ߵ�
#define Cmd_YJ_Hand_Low     2  //��������:�ֶ�����͵�
#define Cmd_YJ_Run_Pusle    3  //����������תָ��������


#define  State_ERR   100  //����״̬
#define  State_STOP  110  //ͣ��״̬
#define  State_IDLE  10   //�ȴ�״̬
#define  State_INIT  0   //�ȴ�״̬
#define  State_Zero_Calib 1 //��λ����״̬
#define  State_ALIGN  2   //����״̬
#define  State_FIND_Z  3   //״̬
#define  State_CNT_RISE  4   //��������״̬
#define  State_CNT_DROP  5   //��������״̬
#define   State_Wait     6
#define   State_WORK_Rdy 7
#define   State_WORK     8







#define  Rise_Main_Pos      387    //ѹ������������ʱ����λ��   (240-100)/360*1000=1550
#define  High_Main_Pos      2833    //ѹ�Ź����߶���ߵ�ʱ����λ��   (240+115-100)/360*4000=1550
#define  Drop_Main_Pos      708    //ѹ�Ź����߶���͵㣨��λ��ʱ����λ��   (240+230-100)/360*1000=1550
#define  Work_Distance      1283    //�����߶ȶ�Ӧ������������=High_Main_Pos-Rise_Main_Pos
//���年��״̬����(DLC=2)
#define GX_OK          0xF706  //���ߵ�λ
#define GX_ERR         0xF707  //���߲���λ

//���年��״̬����(DLC=8)
#define GX_OK_DLC8          1  //���ߵ�λ
#define GX_ERR_DLC8         0  //���߲���λ 2

//������߹�����
#define ErrNum_OverTime_Open      1   //���ߵ��������ʱ
#define ErrNum_OverTime_Find_Z    2   //���ߵ����ԭ�㳬ʱ
#define ErrNum_OverTime_Hand    2   //���ߵ����ԭ�㳬ʱ
//��ȡGXԭ�������ź�,"0"�����źŵ���Ч

//���߿��Ʋ����ṹ��
typedef struct
{
    Uint16 AB_Pulse;         //����:ָ��
    int16  Pos_Set;     //ѹ�Ÿ߶��趨
    int16  Pos_Real;     //��ǰλ��
    int16  Pos_Off;
    Uint16 Ctl_Ena;          //������Z�ź�״̬����0��ΪZ�ź���Ч����Ӧ��ɫָʾ����
    Uint16 High_Index;    //�߶����1Ϊ��λ�߶�;0Ϊ�����߶�

    int16 Max_High;    //��λ�߶�
    Uint16 Work_High;    //�����߶�
    int16 Low_High;    //��͵����
    int16 Work_Pul_Out;    //�����߶ȶ�Ӧ��������=Work_High*2.5
    //����״̬��־
    Uint16 Motion_Cmd;    //��������ָ��
    Uint16 Hand_Cmd;    //ѹ���ֶ�������0-ѹ����������͵㣬1-ѹ��̧������ߵ�
    Uint16 Motor_Ori_Flag;    //������������ߵ��־
    Uint16 Motor_Dir_Flag; //�������

    int32  Main_Shaft_Pos;  //���룺����λ���������0~3999)
    Uint16 Work_Flag; //�������
    Uint16  Main_Shaft_Pos_Last;  //���룺����λ���������0~3999)

    int16 Pulse_Num;
    int16 Pulse_e;
    int16 Pulse_Num_last;
    Uint16 Err_Value;    //�����ѹ�Ź�����
    Uint16 State_now;    //�ڲ���������ǰ����״̬
    Uint16 State_next;   //�ڲ���������һ������״̬
    int32  Delay;        //�ڲ���������ʱ����
    Uint16 Pos_max_min;
}YAJIAO;  //���Ʋ����ṹ��

#define YAJIAO_DEFAULTS { 0,0,0,0,\
                               0,0,70,30,\
                               0,120,0,0,\
                               0,0,0,0,\
                               0,0,0,0,\
                               0,0,0,0,0}

extern YAJIAO YJ;
extern void YJ_Control(void);  //��ɹ��߿���(����ģʽ)


//���߿��Ʋ����ṹ��
typedef struct
{
    Uint16 AB_Pulse;    //AB�������
    Uint16 Type;        //0:ƽ�� 1��������
    Uint16 ControlState;//0:���� 1������
    Uint16 ControlStateTrg;
    Uint16 ControlStateEna;
    Uint16 StartStop;      //0:����ֹͣ 1����������
    Uint16 StartStopFlag;  //0:����ֹͣ 1����������
    Uint16 Init;        //0������δ��ʼ�� 1�����س�ʼ�����

    Uint16 AirValvePos; //1:װ������λ 0��װ������λ
    Uint16 AirValveState;//1:̧װ�� 0����װ��
    Uint16 LightState;  //1:�ػ�ͷ  2���̵� 3�����
    Uint16 WorkState;   //
    Uint16 PearlNeedle; //0:��ɢ����λ 1��ɢ����λ
    Uint16 Speed;
    Uint16 LightFlag;
    Uint16 NoEmbFlag;
    Uint16 LightStateOld;
}EMBROIDER;  //���Ʋ����ṹ��

#define EMBROIDER_DEFAULTS {0,0,0,0,0,0,0,0,\
                            1,1,0,0,0,0,0,0,0}

extern EMBROIDER Embroider;

#endif
