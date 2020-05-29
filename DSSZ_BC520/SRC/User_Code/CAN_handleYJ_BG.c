//=======================================================================
//  Project Number: stepper_controller
//  File Name : Can_handle.c
//  Description: eCANͨѶ����(CAN���͵�������Ӧ���źţ������޷����ͳɹ���
//  Version    Date     Author     Description
//  1.0    2016.11.04   wangdayu        use
//-----------------------------------------------------------------------
//  2015.10.15--���ӶԽ���ID���ж�
//  2015.11.09--�Խ���PN291��M98�����ݲ�������switch�������д���
//  2015.11.13--�Խ������ݵ�DLC�����жϣ�DLC=2��
//  2016.01.19--���Ӷ������������ݵĽ���
//=========================================================================

//step1:ͷ�ļ�����
#include "F2806x_Device.h"     //DSP28030���趨��ͷ�ļ�
#include "Stepper-Settings.h"

/*Uint16 H_Data = 0;
Uint16 L_Data = 0;*/

//#if(CAN_Ena)

CAN_Para CAN = CAN_DEFAULTS;
EMBROIDER Embroider = EMBROIDER_DEFAULTS;
extern TuiZhu_PARA TZ;
extern HuanSe_PARA HS;

extern Uint16 BCorigin;

extern Uint16 PARA;

//step4:�Ӻ�������
Uint16 cap_wat=0;
Uint16 Con_Init_End=1;
Uint16 Y_Mor_Ena=0;
Uint16 B_Mor_Ena=0;
extern Uint16 I2CA_WriteData(Uint16 RomAddress,Uint16 *Wdata,Uint16 number);
extern void HeaderAddress_ReadAndWrite(void);

inline void CAN_YJ_GB(void);
void GZ_TEST_CAN_Handle(void);

void PushA_Handle(void);
void PushB_Handle(void);
void PushPearl_Handle(Uint16 ColorSelect);
inline void CAN_Data_SZJM(void);

extern Uint16 Wri_eprom_addr;
extern Uint16 I2CWD_PAR[2];  //I2Cд������
extern Uint16 Emb_Index;
extern Uint16 Head_En;
extern _iq BM_Open_Id;
extern HeaderAddress_PARA HeaderAddress;
extern Uint16 LightState;
extern int16 Angle_Par;
//extern Uint16 Pulse_cnt;
//-------------------CAN���ͺ���---------------------------------//
//Can���ͺ���(���������ط��ͣ�--��������0,����ID--0x50a
#pragma CODE_SECTION(CAN_TX_DLC8,"ramfuncs");
void CAN_TX_DLC8(void)
{
   //����ʱ���ȷ���D0��D1��D2��D3,D4��D5��D6��D7
   // CAN.Tx_Ticker++;

    //������0���ͳɹ���־
    ECanaRegs.CANTA.all = 0x00000001;

    //д��Ҫ���͵�����
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = 0x0D;
	ECanaMboxes.MBOX0.MDL.byte.BYTE1 = 0x07;
	ECanaMboxes.MBOX0.MDL.byte.BYTE2 = CAN.Trs[2];//�����ַ
	ECanaMboxes.MBOX0.MDL.byte.BYTE3 = 0x34;

	//ECanaMboxes.MBOX0.MDH.byte.BYTE4 = MC.Err_Value;//
	ECanaMboxes.MBOX0.MDH.byte.BYTE4 = 0x01;//���ϴ�����
    ECanaMboxes.MBOX0.MDH.byte.BYTE5 = CAN.Trs[5];//CAN.Trs[5];
    ECanaMboxes.MBOX0.MDH.byte.BYTE6 = 0;// CAN.Trs[6];
    ECanaMboxes.MBOX0.MDH.byte.BYTE7 = 0;//CAN.Trs[7];

    //�������䷢��
    ECanaRegs.CANTRS.all = 0x00000001;//��������0����
}

#pragma CODE_SECTION(CAN_TX_DLC2,"ramfuncs");
void CAN_TX_DLC2(Uint16 TX_Data) //(DLC=2)
{
    ECanaRegs.CANTA.all =0x00000020;//������5���ͳɹ���־
    ECanaMboxes.MBOX5.MDL.word.HI_WORD = TX_Data;//DB=0,�Ӹ�λ��ʼ����HI_WORD = D0D1
    ECanaRegs.CANTRS.all = 0x00000020;//��������5����
}

//-----------DSP����������ش�����---------------------//
#pragma CODE_SECTION(DSP_Update_Handle, "ramfuncs");
inline void DSP_Update_Handle(void)
{
    static Uint16 Update_OverTime = 0; //������ʱ������
    static Uint16 Update_State = 0; //��������״̬

    if((CAN.ID&0x63F)==0) //����ID�ж�
    {
      switch(Update_State)
      {
       //------------------------------------------------------------//
        case(0):  //�ȴ���������
                if((CAN.DLC==2)&&(CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x22))
                {
                  Update_State=1;
                  Update_OverTime = 0;//���㳬ʱ����
                }
        break;
        //------------------------------------------------------------//
        case(1): //�ж�(DSPʶ����--0x0A--DSP2803x),��������Ӳ��ʶ����HardwareNumber

                 //����������ʱ�ж�,��ֹ״̬��������ѭ��
                  Update_OverTime++;
                  if(Update_OverTime>10) //����10�����ݺ󣬸�λ����״̬
                  {
                     Update_OverTime = 0;
                     Update_State = 0; //��λ����״̬
                  }

                 if((CAN.DLC==8)&&\
                    (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0xE5)&&\
                    (CAN.Rec[3]==0x00)&&(CAN.Rec[5]==0x00)&&\
                    (CAN.Rec[6]==0x00)&&(CAN.Rec[7]==0x00))
                 {
                     //if((CAN.Rec[2]==0x0A)&&(CAN.Rec[4]==HardwareNumber))
                     if((CAN.Rec[2]==0x0A)&&(CAN.Rec[4]==PA.Hardware_Code))
                     {
                       PWM1_OFF_MACRO()//����ǰ��ֹͣ�������
		               PWM2_OFF_MACRO()//����ǰ��ֹͣ�������
                       DINT;//������תǰ����ر������ж�

                       I2CWD_PAR[0]= 0x01;//д������������־
                       Wri_eprom_addr = 0xFB;//��0xFBд���û�������ת��־0x01
                       I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
                       DELAY_US(3000);

                       (*DSP_Flash_Update)(); /*��ת��DSP Bootloader*/
                     }
                 }
        break;
      //-------------------------------------------------------------//
      default: break;
     } //end of switch
   }//����ID�ж�
} //end of DSP����������ش�����
//---------------------------------------------------------------//

//---------------CAN�����ж�--------------------//
#pragma CODE_SECTION(ECAN0INT_ISR, "ramfuncs");
interrupt void ECAN0INT_ISR(void)//eCAN-A�����ж�
{
    //�����ж���14��ָ������
    //�����״̬�Ĵ�����ECanaRegs.CANES.all��

    //----------����2-(��������ID)---------------//
    if(ECanaRegs.CANRMP.all == 0x0004)//�ȴ�2���������ָ��
    {
        ECanaRegs.CANRMP.all = 0x0004; //������ձ�־

        /*step1: ����״ָ̬ʾLED*/
        CAN.Rec_Flag = 1; //��λCAN���ձ�־

        /*step2: ��ȡ���յ����ݵ�DLC��ID*/
        CAN.DLC = ECanaMboxes.MBOX2.MSGCTRL.bit.DLC;    //��������DLC
        CAN.ID  = ECanaMboxes.MBOX2.MSGID.bit.STDMSGID; //��������ID-(11bit)-28:18

        /*step3:��ȡ�����е�����*/
        CAN.Rec[0] = ECanaMboxes.MBOX2.MDL.byte.BYTE0;  //CAN-D0
        CAN.Rec[1] = ECanaMboxes.MBOX2.MDL.byte.BYTE1;  //CAN-D1
        CAN_YJ_GB();

        GZ_TEST_CAN_Handle();//��װ�����շ�����

       //����ֻ�з���������������ʱ��DLC=8
       if(CAN.DLC>2)
       {
          CAN.Rec[2] = ECanaMboxes.MBOX2.MDL.byte.BYTE2;  //CAN-D2
          CAN.Rec[3] = ECanaMboxes.MBOX2.MDL.byte.BYTE3;  //CAN-D3
          CAN.Rec[4] = ECanaMboxes.MBOX2.MDH.byte.BYTE4;  //CAN-D4
          CAN.Rec[5] = ECanaMboxes.MBOX2.MDH.byte.BYTE5;  //CAN-D5
          CAN.Rec[6] = ECanaMboxes.MBOX2.MDH.byte.BYTE6;  //CAN-D6
          CAN.Rec[7] = ECanaMboxes.MBOX2.MDH.byte.BYTE7;  //CAN-D7

          if(Embroider.StartStop == 0)
          {
              CANDataDecryptHandle(CAN.Rec,8);
          }
          else
          {
              if((Embroider.WorkState == 0x02)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 0x07))//ֻ�����̵�����+�������+��Ʋ���3��״̬�²Ŵ��������������
              {
                  //CAN_Data_JPJM();
                  CAN_Data_SZJM();
              }
          }

          HeaderAddress_ReadAndWrite();//��д��ͷ��ַ

          //AX8ʹ��
#if(MachineControl == Ax8)
          {
              if((CAN.DLC==8)&&(CAN.Rec[0]==0x00FC)&&(CAN.Rec[1]==0x00B0)&&(CAN.Rec1[2]==0x0000)&&\
                 (CAN.Rec[3]==0x0030)&&(CAN.Rec[4]==0x0003)&&(CAN.Rec[5]==0x0000)&&\
                 (CAN.Rec[6]==0x0000)&&(CAN.Rec[7]==0x0000)) //ʹ������
              {
                  TZ.CPUEnable = 1;
              }
          }

          //��AX8ʹ��
#elif(MachineControl == GE)
          {
              /*if((CAN.DLC==8)&&(CAN.Rec[0]==0x00FC)&&(CAN.Rec[1]==0x00B0)&&(CAN.Rec[2]==0x0000)&&\
                 (CAN.Rec[3]==0x00A2)&&(CAN.Rec[4]==0x0001)&&(CAN.Rec[5]==0x0000)&&\
                 (CAN.Rec[6]==0x0000)&&(CAN.Rec[7]==0x0000)) //ʹ������*/
              {
                  TZ.CPUEnable = 1;
              }
          }
#endif

          if((CAN.DLC==8)&&(CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0xEC)&&(CAN.Rec[2]==0x02))
          {
              if(((CAN.Rec[3]&0x0080)==0x0080)&&(TZ.Address==1))//ĩ�����&&��װ��
              {
#if(MachineType==Three)
                  TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 530);//��ɫɢ��
#else(MachineType==Double)
                  TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 580);//�pɫɢ�� ��ͨ����
                  //TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 594);//�pɫɢ�� ������� �칤���� 2019.10.14
#endif
                  TZ.Step3 = -(TZ.Step1 + TZ.Step2);
                  /*if(Embroider.Init == 1)
                  {
                      if(TZ.Step1 != TZ.Step1Old)
                      {
                          qep2.IndexSyncFlag = 0;
                          TZ.ParaEnable = 1;
                      }
                  }
                  TZ.Step1Old = TZ.Step1;*/
              }
              if(((CAN.Rec[3]&0x0080)==0x0000)&&(TZ.Address==2))//�������&&��װ��
              {
#if(MachineType==Three)
                  TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 530);//��ɫɢ��
#else(MachineType==Double)
                  TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 580);//˫ɫɢ��
                  //TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 594);//˫ɫɢ��
#endif
                  TZ.Step3 = -(TZ.Step1 + TZ.Step2);
                /*if(Embroider.Init == 1)
                  {
                      if(TZ.Step1 != TZ.Step1Old)
                      {
                          qep2.IndexSyncFlag = 0;
                          TZ.ParaEnable = 1;
                      }
                  }
                  TZ.Step1Old = TZ.Step1;*/
              }

              if(((CAN.Rec[4]&0x0080)==0x0080)&&(TZ.Address==1))//ĩ�����&&��װ��
              {
#if(MachineType==Three)
                  TZ.Step2 = -((int16)((CAN.Rec[4]&0x007F)-30) + 160);
#else(MachineType==Double)
                  TZ.Step2 = -((int16)((CAN.Rec[4]&0x007F)-30) + 80);
                  //TZ.Step2 = -((int16)((CAN.Rec[4]&0x007F)-30) + 120);
#endif
                  TZ.Step3 = -(TZ.Step1 + TZ.Step2);
                  /*if(Embroider.Init == 1)
                  {
                      if(TZ.Step2 != TZ.Step2Old)
                      {
                          qep2.IndexSyncFlag = 0;
                          TZ.ParaEnable = 1;
                      }
                  }
                  TZ.Step2Old = TZ.Step2;*/
              }
              if(((CAN.Rec[4]&0x0080)==0x0000)&&(TZ.Address==2))//�������&&��װ��
              {
#if(MachineType==Three)
                  TZ.Step2 = -((int16)((CAN.Rec[4])-30) + 160);
#else(MachineType==Double)
                  TZ.Step2 = -((int16)((CAN.Rec[4])-30) + 80);
                  //TZ.Step2 = -((int16)((CAN.Rec[4])-30) + 120);
#endif
                  TZ.Step3 = -(TZ.Step1 + TZ.Step2);
                  /*if(Embroider.Init == 1)
                  {
                      if(TZ.Step2 != TZ.Step2Old)
                      {
                          qep2.IndexSyncFlag = 0;
                          TZ.ParaEnable = 1;
                      }
                  }
                  TZ.Step2Old = TZ.Step2;*/
              }
              if(((CAN.Rec[5]&0x0080)==0x0080)&&(TZ.Address==1))//ĩ�����&&��װ��
              {
                  TZ.JiaZhu = (CAN.Rec[5]&0x007F)*4 + 72;
              }
              if(((CAN.Rec[5]&0x0080)==0x0000)&&(TZ.Address==2))//�������&&��װ��
              {
                  TZ.JiaZhu = CAN.Rec[5]*4 + 72;
              }
              TZ.DeltaTime = ((int16)CAN.Rec[7] - 30)*24;
              #if(MachineType==Three)
              TZ.DeltaTime = TZ.DeltaTime + 200;
              #endif
          }
          /*if((CAN.DLC==4)&&(CAN.Rec[0]==0x0E)&&(CAN.Rec[1]==0x00)&&(CAN.Rec[2]==0x14))
          {
              PARA = CAN.Rec[3]<<3;//��ʱ���������ڲ����⵽�����źź��ߵľ��룻
          }*/
       }
       //DSP_Update_Handle();
   } // end of ����2����

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;//��λ�ж�Ӧ��Ĵ���

}  //end of ECAN0INT_ISR

//---------CAN���ݴ�����---------------//
#pragma CODE_SECTION(CAN_TX_HS, "ramfuncs");
void CAN_TX_HS(Uint16 state)
{
     CAN.Trs[2] = HeaderAddress.Address;
     CAN.Trs[5] = MC.Err_Value;
     CAN_TX_DLC8();//�����ط���
}

//-------���߿���CANͨѶ����-----------------//
#pragma CODE_SECTION(CAN_TX_TZ, "ramfuncs");
void CAN_TX_TZ(Uint16 state)
{
    CAN.Trs[2] = HeaderAddress.Address;
    CAN.Trs[5] = MB.Err_Value;
    CAN_TX_DLC8();//�����ط���
}

#pragma CODE_SECTION(CAN_YJ_GB, "ramfuncs");
inline void CAN_YJ_GB(void)
{
    if(((CAN.ID&0x63F)==0)&&(CAN.DLC==2)) //����ID�ж�
    {
        switch(CAN.Rec[0])
        {
        case 0x4F:
            if(CAN.Rec[1]==0x8D)//���鵽AƬ
            {
                HS.cmdFlag = 1;
                HS.cmdstate = 0;
            }
            else if(CAN.Rec[1]==0x8E)//���鵽BƬ
            {
                HS.cmdFlag = 2;
                HS.cmdstate = 1;
            }
            break;
        case 0xF0://������ɫɢ��f�h
            if(CAN.Rec[1]==0x51)//���鵽A��
            {
                HS.cmdFlag = 1;
                HS.cmdstate = 0;
            }
            else if(CAN.Rec[1]==0x52)//���鵽B��
            {
                HS.cmdFlag = 2;
                HS.cmdstate = 1;
            }
            /*else if(CAN.Rec[1]==0x54)//���鵽C��
            {
                HS.cmdFlag = 3;
            }*/
            break;
        case 0xD0:
            Embroider.Speed = CAN.Rec[1];
            break;
        case 0xE0:
            if(CAN.Rec[1]==0x00)	    //��λ
            {
                //�ı�Ƶ��������쳣��С��3900���ߴ���4100ʱ�Ὣ�������ֵ����CAN����,2019.11.06
                /*if((Embroider.AB_Pulse < 3900)||(Embroider.AB_Pulse > 4100))
                {
                    CAN_TX_DLC2(Embroider.AB_Pulse);//�����ı�Ƶ�ź��Ƿ���ȷ
                }*/
                //GpioDataRegs.AIOTOGGLE.bit.AIO12 = 1;//������λ�ź�*/
                Embroider.AB_Pulse = 0;
            }
            else if(CAN.Rec[1]==0x01)	//ƽ��
            {
                Embroider.Type = 1;
            }
            else if(CAN.Rec[1]==0x03)	//����
            {
                Embroider.ControlState = 0;
                Embroider.ControlStateTrg = 0;
                Embroider.ControlStateEna = 0;
            }
            else if(CAN.Rec[1]==0x04)	//����
            {
                Embroider.ControlState = 1;
                if((Embroider.ControlStateTrg == 0)&&(Embroider.ControlStateEna == 0))
                {
                    Embroider.ControlStateTrg = 1;
                    Embroider.ControlStateEna = 1;
                    HS.Color_state_old = HS.Color_state;
                }
            }
            else if(CAN.Rec[1]==0x05)	//����ͣ��
            {
                Embroider.StartStop = 0;
                TZ.ErrCnt = 0;
                HS.ErrCnt = 0;
                if(TZ.cmdFlag == 1)
                {
                    TZ.cmdFlag = 0;
                    TZ.cmd = 2;
                }
                HS.Color_state_old = HS.Color_state;
                TZ.MotorEnableFlag = 1;
            }
            else if(CAN.Rec[1]==0x06)	//��������
            {
                Embroider.StartStop = 1;
                Embroider.StartStopFlag = 1;//��������������־λ
                PrePearl_C.InvalidStart = 0;
                PrePearl_D.InvalidStart = 0;
                KeyHandle.OkTrgCnt = 0;//�������ͣ�����ֶ����������ͣ��ԭ���ٴ����������ᵼ��ͣ��ԭ�㱨��
                if(BCorigin == 1)
                    TZ.cmd = 3;
            }
            else if(CAN.Rec[1]==0xFE)	//��ʼ�����
            {
                Embroider.Init = 1;
            }
            else if(CAN.Rec[1]==0xF6)   //ȫ��ͷд��ͷ��ַ���ʼ
            {
                HeaderAddress.StartWrite = 1;
                HeaderAddress.RedOFFFlag = 1;
            }
            else if(CAN.Rec[1]== 0xF7)   //ȫ��ͷд��ͷ��ַ�������
            {
                HeaderAddress.StartWrite = 0;
                HeaderAddress.WriteOver = 0;
            }
            break;
            //---------------------------------------------------------------------------------

        case 0x1f://��������
            switch(CAN.Rec[1])
            {
            case 0x81://��װ��
                Air1_Valve_Set = 1;
                Air2_Valve_Set = 1;
                Embroider.AirValvePos = 1;
                Embroider.AirValveState = 1;//̧װ�� 2019.10.08
                break;
            case 0x82://��װ��
                Embroider.AirValveState = 0;//��װ�� 2019.10.08
                if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
                {
                    if((Embroider.WorkState == 2)||(Embroider.WorkState == 3)||(Embroider.WorkState == 7))//������δ����ʱ����װ�õ�����2019.02.25
                    {
                        Air1_Valve_Clc = 1;
                        Air2_Valve_Clc = 1;
                        Embroider.AirValvePos = 0;
                    }
                }
                break;
            default:
                break;
            }
            break;
            case 0x3F:
                switch(CAN.Rec[1])
                {
                case 0x71:
                    Embroider.PearlNeedle = 1;//ɢ����װ��
                    if(MC.State == ST_Run)
                    {
                        KeyHandle.DisplayData = 0x0040;//"-"�ָ���ʾĬ��״̬
                        KeyHandle.DisplayEnable = 1;
                    }
                    break;
                case 0x72:
                    Embroider.PearlNeedle = 2;//ɢ����װ��
                    if(MC.State == ST_Run)
                    {
                        KeyHandle.DisplayData = 0x0040;//"-"�ָ���ʾĬ��״̬
                        KeyHandle.DisplayEnable = 1;
                    }
                    break;
                case 0x73:
                    Embroider.PearlNeedle = 0;//��ɢ����λ
                    TZ.MotorEnableFlag = 1;
                    break;
                default:
                    break;
                }
                break;
        default:
            break;
        }
        if((Embroider.WorkState == 0x02)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 0x07))//ֻ�����̵�����+�������+��Ʋ���3��״̬�²Ŵ��������������
        {
            if((CAN.Rec[0]==0x5f)&&(CAN.Rec[1]==0x55)||\
               (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x1E)||\
               (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x1F)||\
               (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x5B)||\
               (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x5E)||\
               (CAN.Rec[0]==0xFC)&&(CAN.Rec[1]==0x40)||\
               (CAN.Rec[0]==0xFC)&&(CAN.Rec[1]==0x42)||\
               (CAN.Rec[0]==0xFC)&&(CAN.Rec[1]==0x45)||\
               (CAN.Rec[0]==0x4f)&&(CAN.Rec[1]==0x62)||\
               (CAN.Rec[0]==0x4f)&&(CAN.Rec[1]==0x66)||\
               (CAN.Rec[0]==0x4f)&&(CAN.Rec[1]==0x6B)||\
               (CAN.Rec[0]==0x5f)&&(CAN.Rec[1]==0x53)||\
               (CAN.Rec[0]==0x5f)&&(CAN.Rec[1]==0x5A))
            {
                PushPearl_Handle(1);//1:A��
                PrePearl_C.Timer = 0;
            }
            if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x60))
            {
                PushPearl_Handle(2);//2:B��
                PrePearl_D.Timer = 0;
            }
            if(Embroider.StartStop == 0)
            {
                if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x55))
                {
                    if(HS.Color_state != TZ.color)
                    {
                        //A���˶���
                        TZ.cmd = 2;
                        TZ.cmdFlag = 0;
                    }

                }
                if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x56))
                {
                    if(HS.Color_state != TZ.color)
                    {
                        //B���˶���
                        TZ.cmd = 2;
                        TZ.cmdFlag = 0;
                    }
                }
            }
        }
        if((CAN.Rec[0]==0xFA)&&(CAN.Rec[1]==0x30))
        {
            //ȫͷ���鿪ʼ
            TZ.All_START = 1;
        }
        if((CAN.Rec[0]==0xFA)&&(CAN.Rec[1]==0x31))
        {
            //ȫͷ�������
            TZ.All_START = 0;
        }
    }
}

//-------CANͨѶ����(������)-----------------//
#pragma CODE_SECTION(CAN_Handle, "ramfuncs");
void CAN_Handle(void)
{
    if(CAN.Rec_Flag == 1)//CAN�����ж����ý��ձ�־
    {
       CAN.Rec_Flag = 0; //��CAN���ձ�־

       if(Embroider.LightState == 3)
       {
           DSP_Update_Handle();//������������(MD612����DIP״̬�ж�)
       }
    }
}
void DecryptOutputDataHandle()
{
    /*---------ɢ��ָ����ܹ���----------------------
    02 02 D1 D2 R1 R2 R3 R4  ���� D1 D2��ͬ��ɢ�����ָ��
    02 01 D1 D2 R1 R2 R3 R4  ���� D1 D2��ͬ��ɢ����Ƭָ��
    1E 04 D1 D2 S1 0  0  0   ����S1 ��Ӧ����ָ��
    -----------------END-----------------------*/
    if((EncryptBlock[0] == 0x02)&&(EncryptBlock[1] == 0x02)) //����
    {
        CAN.back1 = EncryptBlock[2];
        CAN.back2 = EncryptBlock[3];
    }
    else if((EncryptBlock[0] == 0x02)&&(EncryptBlock[1] == 0x01)) //����
    {
        CAN.Forward1 = EncryptBlock[2];
        CAN.Forward2 = EncryptBlock[3];
    }
}
//���ܺ��ɢ��ָ�����
#pragma CODE_SECTION(CAN_Data_SZJM, "ramfuncs");
inline void CAN_Data_SZJM(void)
{
    if((CAN.Rec[0] == 0x1E)&&(CAN.Rec[1] == 0x04)&&(CAN.Rec[2] == CAN.Forward1)&&(CAN.Rec[3] == CAN.Forward2) )
    {
        if(CAN.Rec[4] == 0x81)//A������
        {
            PushPearl_Handle(1);//1:A��
        }
        else if(CAN.Rec[4] == 0x82)//B������
        {
            PushPearl_Handle(2);//2:B��
        }
    }
}
#pragma CODE_SECTION(PushPearl_Handle, "ramfuncs");
void PushPearl_Handle(Uint16 ColorSelect)
{
    if(Embroider.ControlStateTrg == 1)
    {
        Embroider.ControlStateTrg = 0;
        if(HS.Color_state != HS.Color_state_old)
        {
            //A���鶯��
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//ͣ��״̬
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A���鶯��
            TZ.cmd = 1;
            TZ.cmdFlag = 1;
            TZ.AB_Pulse = 0;
            TZ.Enable = 1;
            if((Embroider.WorkState == 2)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 7))
            {
                switch(ColorSelect)
                {
                case 1:
                    PrePearl_C.Mount++;
                    PrePearl_C.ErrorMount++;
                    break;
                case 2:
                    PrePearl_D.Mount++;
                    PrePearl_D.ErrorMount++;
                    break;
                default:
                    break;
                }
            }
            switch(ColorSelect)
            {
            case 1:
                if(PrePearl_C.ErrorEnable == 1)
                {
                    PrePearl_C.ErrorRestartMount++;
                    if(PrePearl_C.ErrorRestartMount >= 20)
                    {
                        PrePearl_C.ErrorRestartMount = 0;
                        PrePearl_C.ErrorEnable = 0;
                    }
                }
                break;
            case 2:
                if(PrePearl_D.ErrorEnable == 1)
                {
                    PrePearl_D.ErrorRestartMount++;
                    if(PrePearl_D.ErrorRestartMount >= 20)
                    {
                        PrePearl_D.ErrorRestartMount = 0;
                        PrePearl_D.ErrorEnable = 0;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}
void PushA_Handle(void)
{
    if(Embroider.ControlStateTrg == 1)
    {
        Embroider.ControlStateTrg = 0;
        if(HS.Color_state != HS.Color_state_old)
        {
            //A���鶯��
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//ͣ��״̬
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A���鶯��
            TZ.cmd = 1;
            TZ.cmdFlag = 1;
            TZ.AB_Pulse = 0;
            TZ.Enable = 1;
            if((Embroider.WorkState == 2)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 7))
            {
                PrePearl_C.Mount++;
                PrePearl_C.ErrorMount++;
            }
            if(PrePearl_C.ErrorEnable == 1)
            {
                PrePearl_C.ErrorRestartMount++;
                if(PrePearl_C.ErrorRestartMount >= 20)
                {
                    PrePearl_C.ErrorRestartMount = 0;
                    PrePearl_C.ErrorEnable = 0;
                }
            }
        }
    }
}

void PushB_Handle(void)
{
    if(Embroider.ControlStateTrg == 1)
    {
        Embroider.ControlStateTrg = 0;
        if(HS.Color_state != HS.Color_state_old)
        {
            //A���鶯��
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//ͣ��״̬
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A���鶯��
            TZ.cmd = 1;
            TZ.cmdFlag = 1;
            TZ.AB_Pulse = 0;
            TZ.Enable = 1;
            if((Embroider.WorkState == 2)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 7))
            {
                PrePearl_D.Mount++;
                PrePearl_D.ErrorMount++;
            }
            if(PrePearl_D.ErrorEnable == 1)
            {
                PrePearl_D.ErrorRestartMount++;
                if(PrePearl_D.ErrorRestartMount >= 20)
                {
                    PrePearl_D.ErrorRestartMount = 0;
                    PrePearl_D.ErrorEnable = 0;
                }
            }
        }
    }
}
//#endif // end of CAN_Ena
//======================================================================================
//         COPYRIGHT(C) 2015 Beijing technology Co., Ltd.
//                     ALL RIGHTS RESERVED 
//======================================================================================
