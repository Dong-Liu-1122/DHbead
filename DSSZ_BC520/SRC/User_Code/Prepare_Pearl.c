/*
 * Sci_Com.c
 *
 * Created on: 2019��01��08��
 * Author: liudong
 */
#include "F2806x_Device.h"  //DSP28030���趨��ͷ�ļ�
#include "Stepper-Settings.h"

PREPEARL_HANDLE PrePearl_C = PrePearl_HANDLE_DEFAULT;
PREPEARL_HANDLE PrePearl_D = PrePearl_HANDLE_DEFAULT;

extern MOTOR_CONTROL_SPTA motorC;
extern MOTOR_CONTROL_SPTA motorD;

extern void CAN_TX_DLC2(Uint16 TX_Data); //(DLC=2)

Uint16 StepDelay[10] = {7000,7000,4000,2000,1300,1000,300,0,0,0};

Uint16 BCorigin = 0;//����ԭ�����־
Uint16 ClearPulseFlag = 0;//��������������־

//���㷨
#define FilterTime 10

Uint16 Cnt = 0;

void PearlMotorC(void);
void PearlMotorD(void);

//BC520_F:��һ�����
//BC520_G:�ڶ�������������źŷ���
#define BoardType BC520_G
#define BC520_F 1
#define BC520_G 2

Uint16 OPTO1_Check(void)
{
    static Uint16 FilterCnt1 = 0;
    static Uint16 FilterCnt11 = 0;
    static Uint16 CheckResult1 = 0;
#if(BoardType==BC520_F)
    if(INPUT_HC597Regs.bit.OPTO1 == 0)
#elif(BoardType==BC520_G)
    if(INPUT_HC597Regs.bit.OPTO1 == 1)//��Ӧ�������ź� ����ָʾ����
#endif
    {
        FilterCnt11 = 0;
        FilterCnt1++;
        if(FilterCnt1 >= FilterTime)
        {
            FilterCnt1 = FilterTime;
            CheckResult1 = 1;
        }
    }
    else
    {
        FilterCnt1 = 0;
        FilterCnt11++;
        if(FilterCnt11 >= FilterTime)
        {
            FilterCnt11 = FilterTime;
            CheckResult1 = 0;
        }
    }
    return CheckResult1;
}
Uint16 OPTO2_Check(void)
{
    static Uint16 FilterCnt2 = 0;
    static Uint16 FilterCnt21 = 0;
    static Uint16 CheckResult2 = 0;
#if(BoardType==BC520_F)
    if(INPUT_HC597Regs.bit.OPTO2 == 0)
#elif(BoardType==BC520_G)
    if(INPUT_HC597Regs.bit.OPTO2 == 1)
#endif
    {
        FilterCnt21 = 0;
        FilterCnt2++;
        if(FilterCnt2 >= FilterTime)
        {
            FilterCnt2 = FilterTime;
            CheckResult2 = 1;
        }
    }
    else
    {
        FilterCnt2 = 0;
        FilterCnt21++;
        if(FilterCnt21 > FilterTime)
        {
            FilterCnt21 = FilterTime;
            CheckResult2 = 0;
        }
    }
    return CheckResult2;
}
Uint16 OPTO3_Check(void)
{
    static Uint16 FilterCnt3 = 0;
    static Uint16 FilterCnt31 = 0;
    static Uint16 CheckResult3 = 0;
    if(INPUT_HC597Regs.bit.OPTO3 == 0)//��Ӧ�������ź� ����ָʾ����
    {
        FilterCnt31 = 0;
        FilterCnt3++;
        //if(FilterCnt3 >= 10)
        if(FilterCnt3 >= 80)//�õ�Ƭ�뿪�����ԶһЩ��ʹ����˿ʱ��Ƭ��δ�������
        {
            FilterCnt3 = 10;
            CheckResult3 = 1;
        }
    }
    else
    {
        FilterCnt3 = 0;
        FilterCnt31++;
        if(FilterCnt31 >= 10)
        {
            FilterCnt31 = 10;
            CheckResult3 = 0;
        }
    }
    return CheckResult3;
}

void PrePearlHandle(void)
{
    static Uint16 waitDelay = 20000;

    PrePearl_C.OPTO_Flag = OPTO1_Check();
    PrePearl_D.OPTO_Flag = OPTO2_Check();
    BCorigin = OPTO3_Check();//����ԭ���ź�
    if(BCorigin == 0)
        ClearPulseFlag = 0;

    waitDelay--;
    if(waitDelay <= 1)
    {
        waitDelay = 1;

        PearlMotorC();
        PearlMotorD();
    }

    if(KeyHandle.Mode == 0)
    {
        if(KeyHandle.DownTrg == 1)
        {
            KeyHandle.DownTrg = 0;
            //��ͷ�̵�ʱ��ӦA��,���ʱ��ӦB��
            if(HS.Color_state == 0){
                PrePearl_C.KeyCnt++;
                PrePearl_C.KeyEnable++;
            }
            else if(HS.Color_state == 1){
                PrePearl_D.KeyCnt++;
                PrePearl_D.KeyEnable++;
            }
        }
    }

    if(PrePearl_C.Mount >= 35)//��׼��25
    {
        PrePearl_C.Mount = 0;
        PrePearl_C.StartFlag = 1;
        PrePearl_C.KeyCnt = 0;
    }
    if(PrePearl_D.Mount >= 25)//��׼��25
    {
        PrePearl_D.Mount = 0;
        PrePearl_D.StartFlag = 1;
        PrePearl_D.KeyCnt = 0;
    }


    if(PrePearl_C.ErrorEnable == 0)
    {
        if(PrePearl_C.ErrorMount > TZ.JiaZhu)
        {
            if(Embroider.LightState != 1)
            {
                //ɢ�鱨��ͣ��
                PrePearl_C.ErrorMount = 0;//Ӧ�ͻ�Ҫ������û�˱����������˲��ٱ�����ֱ������2019.07.31
                CAN_TX_DLC2(0x0D98);
                PrePearl_C.ErrorEnable = 1;
                KeyHandle.DisplayData = 0x0077;//A�鱨����ʾ��A��
                KeyHandle.DisplayEnable = 1;
            }
        }
    }

    if(PrePearl_D.ErrorEnable == 0)
    {
        if(PrePearl_D.ErrorMount > TZ.JiaZhu)
        {
            if(Embroider.LightState != 1)
            {
                //ɢ�鱨��ͣ��
                PrePearl_D.ErrorMount = 0;//Ӧ�ͻ�Ҫ������û�˱����������˲��ٱ�����ֱ������2019.07.31
                CAN_TX_DLC2(0x0D98);
                PrePearl_D.ErrorEnable = 1;
                KeyHandle.DisplayData = 0x007C;//B�鱨����ʾ��B��
                KeyHandle.DisplayEnable = 1;
            }
        }
    }
}
void AirValveHandle(void)
{
    if(KeyHandle.AirTrg == 1)
    {
        KeyHandle.AirTrg = 0;
        if(Embroider.AirValvePos == 1)
        {
            Air1_Valve_Clc = 1;
            Air2_Valve_Clc = 1;
            Embroider.AirValvePos = 0;
        }
        else if(Embroider.AirValvePos == 0)
        {
            Air1_Valve_Set = 1;
            Air2_Valve_Set = 1;
            Embroider.AirValvePos = 1;
            CAN_TX_DLC2(0xF710);
        }
    }
}
void PearlMotorC(void)
{
    if(PrePearl_C.OPTO_Flag == 0)//���ӱպϣ���������
    {
        PrePearl_C.KeyCnt = 0;
    }

    if(PrePearl_C.KeyEnable == 1)
    {
        if(PrePearl_C.KeyCnt < 5)
        {
            PrePearl_C.State = 1;
            PrePearl_C.KeyEnable = 2;
        }
        else
        {
            PrePearl_C.KeyEnable = 0;
            PrePearl_C.State = 3;
        }
    }
    else if(PrePearl_C.KeyEnable == 3)//ֹͣ
    {
        PrePearl_C.State = 31;
        PrePearl_C.KeyEnable = 0;
    }
    switch(PrePearl_C.State)
    {
    case 0:
        if((Embroider.Init == 1)&&(Embroider.PearlNeedle != 0))//���ӷ�ɢ����λ���������鹦�� 2019.11.05
        {
/*            if(PrePearl_C.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_C.State = 1;//�������
            }
            else//�����ſ�����������
            {
                PrePearl_C.State = 4;//����״̬
            }*/
            PrePearl_C.State = 4;//����״̬
        }
        break;
    case 1:
        motorC.EnableFlag = 1;
        motorC.speedenbale = 1;
        motorC.PulseEnable = 1;
        motorC.step_count = 0;
        PrePearl_C.State = 11;//���Ԥ������
        PrePearl_C.TimerEnable = 1;
        break;
    case 11:
        //------���ת����ɲ��ҵ���������仯,��λ�õ��л������ģʽPrePearl_C.State = 2-------------------
        if((PrePearl_C.OPTO_Flag != PrePearl_C.OPTO_Flag_HIS)&&(PrePearl_C.OPTO_Flag == 0))
        {
            if(PrePearl_C.C_FLAG == 0)
            {
                motorC.step_count = 0;
                PrePearl_C.C_FLAG = 1;
                PrePearl_C.State = 2;//�������
            }
        }
        PrePearl_C.OPTO_Flag_HIS = PrePearl_C.OPTO_Flag;

        //------���ת����һȦ�ڼ��������ޱ仯,��λ�õ㣨�������㣩�л���ͣ��ģʽPrePearl_C.State = 3------
        if((motorC.step_count >= 0)&&(motorC.step_count < 6000))
        {
            if(PrePearl_C.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_C.OnCnt++;
                PrePearl_C.InvalidStart = 0;
                PrePearl_C.KeyCnt = 0;
            }
            else//�����ſ�����������
            {
                PrePearl_C.OffCnt++;
            }
        }
        else if(motorC.step_count >= 6000)
        {
            if((PrePearl_C.OffCnt > PrePearl_C.OnCnt)&&(PrePearl_C.OnCnt <= 10))
            {
                PrePearl_C.ErrorMount = 0;
                if(PrePearl_C.InvalidStart > 3)
                {
                    if(Embroider.LightState != 1)
                    {
                        //ɢ�鱨��ͣ��
                        CAN_TX_DLC2(0x0D98);
                        KeyHandle.DisplayData = 0x0006;//������ʾ��1��
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                PrePearl_C.State = 21;
            }
            else if(motorC.step_count >= 6400)
            {
                motorC.step_count = 0;
                PrePearl_C.OnCnt = 0;
                PrePearl_C.OffCnt = 0;
            }
        }
        break;
    case 2:
#if 0
        if((motorC.step_count >= 0)&&(motorC.step_count < 6000))
        {
            if(PrePearl_C.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_C.OnCnt++;
                PrePearl_C.InvalidStart = 0;
                PrePearl_C.KeyCnt = 0;
            }
            else//�����ſ�����������
            {
                PrePearl_C.OffCnt++;
            }
        }
        else if(motorC.step_count >= 6000)
        {
            if((PrePearl_C.OffCnt > PrePearl_C.OnCnt)&&(PrePearl_C.OnCnt <= 10))
            {
                PrePearl_C.ErrorMount = 0;
                if(PrePearl_C.InvalidStart > 3)
                {
                    if(Embroider.LightState != 1)
                    {
                        //ɢ�鱨��ͣ��
                        CAN_TX_DLC2(0x0D98);
                        KeyHandle.DisplayData = 0x0006;//������ʾ��1��
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                PrePearl_C.State = 21;
            }
            else if(motorC.step_count >= 6400)
            {
                motorC.step_count = 0;
                PrePearl_C.OnCnt = 0;
                PrePearl_C.OffCnt = 0;
            }
        }
#else
        if((motorC.step_count >= 0)&&(motorC.step_count < 100))
        {
            if(PrePearl_C.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_C.OnCnt++;
                PrePearl_C.InvalidStart = 0;
                PrePearl_C.KeyCnt = 0;
            }
            else//�����ſ�����������
            {
                PrePearl_C.OffCnt++;
            }
        }
        else if(motorC.step_count == 100)
        {
            if((PrePearl_C.OffCnt > PrePearl_C.OnCnt)&&(PrePearl_C.OnCnt <= 10))
            {
                PrePearl_C.ErrorMount = 0;
                if(PrePearl_C.InvalidStart > 3)
                {
                    if(Embroider.LightState != 1)
                    {
                        //ɢ�鱨��ͣ��
                        CAN_TX_DLC2(0x0D98);
                        KeyHandle.DisplayData = 0x0006;//������ʾ��1��
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                PrePearl_C.State = 22;
            }
        }
        else if(motorC.step_count >= 6400)
        {
            motorC.step_count = 0;
            PrePearl_C.OnCnt = 0;
            PrePearl_C.OffCnt = 0;
        }
        if((PrePearl_C.OPTO_Flag != PrePearl_C.OPTO_Flag_HIS)&&(PrePearl_C.OPTO_Flag == 0))
        {
            if(PrePearl_C.C_FLAG == 0)
            {
                motorC.step_count = 0;
                PrePearl_C.C_FLAG = 1;
            }
        }
        PrePearl_C.OPTO_Flag_HIS = PrePearl_C.OPTO_Flag;
#endif
        break;
    case 21:
        if(motorC.step_count >= 6400)
        {
            motorC.step_count = 0;
            PrePearl_C.OnCnt = 0;
            PrePearl_C.OffCnt = 0;
            PrePearl_C.State = 3;
        }
        break;
    case 22:
        if(motorC.step_count >= 200)
        {
            motorC.step_count = 0;
            PrePearl_C.OnCnt = 0;
            PrePearl_C.OffCnt = 0;
            PrePearl_C.State = 3;
        }
        break;
    case 3:
        motorC.speedenbale = 0;
        motorC.step_state = 4;
        PrePearl_C.State = 4;
        PrePearl_C.C_FLAG = 0;
        PrePearl_C.TimerEnable = 0;
        PrePearl_C.Timer = 0;
        break;
    case 31:
        if((PrePearl_C.OPTO_Flag != PrePearl_C.OPTO_Flag_HIS)&&(PrePearl_C.OPTO_Flag == 0))
        {
            motorC.step_count = 0;
            PrePearl_C.State = 22;
        }
        PrePearl_C.OPTO_Flag_HIS = PrePearl_C.OPTO_Flag;
        break;
    case 4:
        if(PrePearl_C.StartFlag == 1)
        {
            PrePearl_C.StartFlag = 0;
            PrePearl_C.InvalidStart++;
            PrePearl_C.State = 1;
        }
        break;
    }
}
void PearlMotorD(void)
{
    if(PrePearl_D.OPTO_Flag == 0)//���ӱպϣ���������
    {
        PrePearl_D.KeyCnt = 0;
    }

    if(PrePearl_D.KeyEnable == 1)
    {
        if(PrePearl_D.KeyCnt < 5)
        {
            PrePearl_D.State = 1;
            PrePearl_D.KeyEnable = 2;
        }
        else
        {
            PrePearl_D.KeyEnable = 0;
            PrePearl_D.State = 3;
        }
    }
    else if(PrePearl_D.KeyEnable == 3)//ֹͣ
    {
        PrePearl_D.State = 31;
        PrePearl_D.KeyEnable = 0;
    }
    switch(PrePearl_D.State)
    {
    case 0:
        if((Embroider.Init == 1)&&(Embroider.PearlNeedle != 0))//���ӷ�ɢ����λ���������鹦�� 2019.11.05
        {
           /* if(PrePearl_D.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_D.State = 1;//�������
            }
            else//�����ſ�����������
            {
                PrePearl_D.State = 4;//����״̬
            }*/
            PrePearl_D.State = 4;//����״̬
        }
        break;
    case 1:
        motorD.EnableFlag = 1;
        motorD.speedenbale = 1;
        motorD.PulseEnable = 1;
        motorD.step_count = 0;
        PrePearl_D.State = 11;//���Ԥ������
        PrePearl_D.TimerEnable = 1;
        break;
    case 11:
        //------���ת����ɲ��ҵ���������仯,��λ�õ��л������ģʽPrePearl_D.State = 2-------------------
        if((PrePearl_D.OPTO_Flag != PrePearl_D.OPTO_Flag_HIS)&&(PrePearl_D.OPTO_Flag == 0))
        {
            if(PrePearl_D.C_FLAG == 0)
            {
                motorD.step_count = 0;
                PrePearl_D.C_FLAG = 1;
                PrePearl_D.State = 2;//�������
            }
        }
        PrePearl_D.OPTO_Flag_HIS = PrePearl_D.OPTO_Flag;

        //------���ת����һȦ�ڼ��������ޱ仯,��λ�õ㣨�������㣩�л���ͣ��ģʽPrePearl_D.State = 3------
        if((motorD.step_count >= 0)&&(motorD.step_count < 6000))
        {
            if(PrePearl_D.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_D.OnCnt++;
                PrePearl_D.InvalidStart = 0;
                PrePearl_D.KeyCnt = 0;
            }
            else//�����ſ�����������
            {
                PrePearl_D.OffCnt++;
            }
        }
        else if(motorD.step_count >= 6000)
        {
            if((PrePearl_D.OffCnt > PrePearl_D.OnCnt)&&(PrePearl_D.OnCnt <= 10))
            {
                PrePearl_D.ErrorMount = 0;
                if(PrePearl_D.InvalidStart > 3)
                {
                    if(Embroider.LightState != 1)
                    {
                        //ɢ�鱨��ͣ��
                        CAN_TX_DLC2(0x0D98);
                        KeyHandle.DisplayData = 0x005B;//������ʾ��2��
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                PrePearl_D.State = 21;
            }
            else if(motorD.step_count >= 6400)
            {
              motorD.step_count = 0;
              PrePearl_D.OnCnt = 0;
              PrePearl_D.OffCnt = 0;
            }
        }
        break;
    case 2:
        if((motorD.step_count >= 0)&&(motorD.step_count < 100))
        {
            if(PrePearl_D.OPTO_Flag == 0)//���ӱպϣ���������
            {
                PrePearl_D.OnCnt++;
                PrePearl_D.InvalidStart = 0;
                PrePearl_D.KeyCnt = 0;
            }
            else//�����ſ�����������
            {
                PrePearl_D.OffCnt++;
            }
        }
        else if(motorD.step_count == 100)
        {
            if((PrePearl_D.OffCnt > PrePearl_D.OnCnt)&&(PrePearl_D.OnCnt <= 10))
            {
                PrePearl_D.ErrorMount = 0;
                if(PrePearl_D.InvalidStart > 3)
                {
                    if(Embroider.LightState != 1)
                    {
                        //ɢ�鱨��ͣ��
                        CAN_TX_DLC2(0x0D98);
                        KeyHandle.DisplayData = 0x005B;//������ʾ��2��
                        KeyHandle.DisplayEnable = 1;
                    }
                }
                PrePearl_D.State = 22;
            }
        }
        else if(motorD.step_count >= 6400)
        {
            motorD.step_count = 0;
            PrePearl_D.OnCnt = 0;
            PrePearl_D.OffCnt = 0;
        }
        if((PrePearl_D.OPTO_Flag != PrePearl_D.OPTO_Flag_HIS)&&(PrePearl_D.OPTO_Flag == 1))
        {
            if(PrePearl_D.C_FLAG == 0)
            {
                motorD.step_count = 0;
                PrePearl_D.C_FLAG = 1;
            }
        }
        PrePearl_D.OPTO_Flag_HIS = PrePearl_D.OPTO_Flag;
        break;
    case 21:
        if(motorD.step_count >= 6400)
        {
            motorD.step_count = 0;
            PrePearl_D.OnCnt = 0;
            PrePearl_D.OffCnt = 0;
            PrePearl_D.State = 3;
        }
        break;
    case 22:
        if(motorD.step_count >= 200)
        {
            motorD.step_count = 0;
            PrePearl_D.OnCnt = 0;
            PrePearl_D.OffCnt = 0;
            PrePearl_D.State = 3;
        }
        break;
    case 3:
        motorD.speedenbale = 0;
        motorD.step_state = 4;
        PrePearl_D.State = 4;
        PrePearl_D.C_FLAG = 0;
        PrePearl_D.TimerEnable = 0;
        PrePearl_D.Timer = 0;
        break;
    case 31:
        if((PrePearl_D.OPTO_Flag != PrePearl_D.OPTO_Flag_HIS)&&(PrePearl_D.OPTO_Flag == 0))
        {
            motorD.step_count = 0;
            PrePearl_D.State = 22;
        }
        PrePearl_D.OPTO_Flag_HIS = PrePearl_D.OPTO_Flag;
        break;
    case 4:
        if(PrePearl_D.StartFlag == 1)
        {
            PrePearl_D.StartFlag = 0;
            PrePearl_D.InvalidStart++;
            PrePearl_D.State = 1;
        }
        break;
    }
}

//ĳ�����ӳ�ʱ�䲻�ã�����ת��3����ֹͣ
void Pearl_Continuity_Check(void)
{
    if((PrePearl_C.TimerEnable == 1)&&(HS.Color_state == 1))//��ǰ��B����A������ת��
    {
        PrePearl_C.Timer++;
        if(PrePearl_C.Timer >= 3000000)//5����
        {
            PrePearl_C.State = 31;
            PrePearl_C.Timer = 0;
            PrePearl_C.TimerEnable = 0;
        }
    }
    if((PrePearl_D.TimerEnable == 1)&&(HS.Color_state == 0))//��ǰ��A����B������ת��
    {
        PrePearl_D.Timer++;
        if(PrePearl_D.Timer >= 3000000)//5����
        {
            PrePearl_D.State = 31;
            PrePearl_D.Timer = 0;
            PrePearl_D.TimerEnable = 0;
        }
    }
}
