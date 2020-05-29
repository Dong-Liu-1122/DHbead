/*
 * Baigan.h
 *
 *  Created on: 2017��8��7��
 *      Author: guozhenan
 */

#ifndef BAIGAN_H_
#define BAIGAN_H_

#define Cmd_BG_Hand_Clear   0  //��������ֶ�ָ�����
#define Cmd_BG_Hand   1  //��������ֶ�����ߵ�
#define BG_Start_Main_Pos      260    //�ڸ���ʼ����ʱ����λ��    (200-100)/360*4000=1111
#define BG_Finish_Main_Pos     3555   //����ʱ����λ��   (200+220-100)/360*4000=3999
#define BG_Dist_Main_Pos     2444
//#define  BG_ST_MainPos      1111

#define Cmd_BG_Hand_Ori    1  //��������ֶ�����ߵ�
#define Cmd_BG_Hand_NoOri     2  //��������:�ֶ�����͵�

#define  State_BG_ERR      100  //����״̬
#define  State_BG_STOP     110  //ͣ��״̬
#define  State_BG_IDLE     10   //�ȴ�״̬
#define  State_BG_INIT     0   //�ȴ�״̬
#define  State_BG_ALIGN    2   //����״̬
#define  State_BG_FIND_Z   3   //״̬
#define  State_BG_TO_NORI  4   //��������״̬
#define  State_BG_TO_ORI   5   //��������״̬
#define  State_BG_Wait     6
#define  State_BG_WORK_Rdy 7
#define  State_BG_WORK     8

typedef struct
{
    Uint16 Ctl_Ena;         //����:ָ��
    Uint16 ORIGN_Flag;       //�ϵ緢1�Σ����߳�ʼλ��=1����ԭ�㣻=0���ڷ�ԭ��
    int16  Pos_Set;     //�ڷ��Ƕ��趨
    //int16  Work_Set;     //���������趨
   // int16  Pos_Off;
    //Uint16 Enc_Z;          //������Z�ź�״̬����0��ΪZ�ź���Ч����Ӧ��ɫָʾ����
   // Uint16 High_Index;    //�߶����1Ϊ��λ�߶�;0Ϊ�����߶�
    //int16 Max_High;    //��λ�߶�
   // Uint16 Work_High;    //�����߶�
   // Uint16 Low_High;    //��͵����
    int16 Work_Pul_Out;    //�������ȶ�Ӧ��������
    //����״̬��־
    Uint16 Motion_Cmd;    //��������ָ��
    Uint16 Hand_Cmd;    //�ڸ��ֶ�����0-������1-����
    Uint16 Motor_Ori_Staus;    //������������ߵ��־
    Uint16 Motor_Dir_Flag; //�������

    int32  Main_Shaft_Pos;  //���룺����λ���������0~3999)
    Uint16 Work_Flag; //�������
    Uint16  Main_Shaft_Pos_Last;  //���룺����λ���������0~3999)
   // Uint16 Rise_Main_Pos   //ѹ������ʱ����λ��
    Uint16 Pos_max_min;
   // int16 Pulse_e;
    int16 Pos_max_min_last;
    Uint16 Err_Value;    //�����������
    Uint16 State_now;    //�ڲ���������ǰ����״̬
    Uint16 State_next;   //�ڲ���������һ������״̬
    int32  Delay;        //�ڲ���������ʱ����
    Uint16 Stop_Flag;    //ͣ����־
    Uint16 Distan_Real;    //��������ָ��
}BAIG;  //���Ʋ����ṹ��

#define BG_DEFAULTS {0,0,50,200,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0}

extern BAIG BG;

#endif /* BAIGAN_H_ */
