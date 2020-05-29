//=======================================================================
//  Project Number: stepper_controller
//  File Name : Can_handle.c
//  Description: eCAN通讯处理(CAN发送到必须有应答信号，否则无法发送成功）
//  Version    Date     Author     Description
//  1.0    2016.11.04   wangdayu        use
//-----------------------------------------------------------------------
//  2015.10.15--增加对接收ID的判断
//  2015.11.09--对接收PN291和M98的数据采用两个switch（）进行处理
//  2015.11.13--对接收数据的DLC进行判断（DLC=2）
//  2016.01.19--增加对在线升级数据的接收
//=========================================================================

//step1:头文件定义
#include "F2806x_Device.h"     //DSP28030外设定义头文件
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

//step4:子函数定义
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
extern Uint16 I2CWD_PAR[2];  //I2C写入数组
extern Uint16 Emb_Index;
extern Uint16 Head_En;
extern _iq BM_Open_Id;
extern HeaderAddress_PARA HeaderAddress;
extern Uint16 LightState;
extern int16 Angle_Par;
//extern Uint16 Pulse_cnt;
//-------------------CAN发送函数---------------------------------//
//Can发送函数(用于向主控发送）--采用邮箱0,发送ID--0x50a
#pragma CODE_SECTION(CAN_TX_DLC8,"ramfuncs");
void CAN_TX_DLC8(void)
{
   //发送时，先发送D0，D1，D2，D3,D4，D5，D6，D7
   // CAN.Tx_Ticker++;

    //清邮箱0发送成功标志
    ECanaRegs.CANTA.all = 0x00000001;

    //写入要发送的数据
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = 0x0D;
	ECanaMboxes.MBOX0.MDL.byte.BYTE1 = 0x07;
	ECanaMboxes.MBOX0.MDL.byte.BYTE2 = CAN.Trs[2];//板件地址
	ECanaMboxes.MBOX0.MDL.byte.BYTE3 = 0x34;

	//ECanaMboxes.MBOX0.MDH.byte.BYTE4 = MC.Err_Value;//
	ECanaMboxes.MBOX0.MDH.byte.BYTE4 = 0x01;//故障错误码
    ECanaMboxes.MBOX0.MDH.byte.BYTE5 = CAN.Trs[5];//CAN.Trs[5];
    ECanaMboxes.MBOX0.MDH.byte.BYTE6 = 0;// CAN.Trs[6];
    ECanaMboxes.MBOX0.MDH.byte.BYTE7 = 0;//CAN.Trs[7];

    //启动邮箱发送
    ECanaRegs.CANTRS.all = 0x00000001;//启动邮箱0发送
}

#pragma CODE_SECTION(CAN_TX_DLC2,"ramfuncs");
void CAN_TX_DLC2(Uint16 TX_Data) //(DLC=2)
{
    ECanaRegs.CANTA.all =0x00000020;//清邮箱5发送成功标志
    ECanaMboxes.MBOX5.MDL.word.HI_WORD = TX_Data;//DB=0,从高位开始发送HI_WORD = D0D1
    ECanaRegs.CANTRS.all = 0x00000020;//启动邮箱5发送
}

//-----------DSP在线升级相关处理函数---------------------//
#pragma CODE_SECTION(DSP_Update_Handle, "ramfuncs");
inline void DSP_Update_Handle(void)
{
    static Uint16 Update_OverTime = 0; //升级延时检测计数
    static Uint16 Update_State = 0; //升级控制状态

    if((CAN.ID&0x63F)==0) //主控ID判断
    {
      switch(Update_State)
      {
       //------------------------------------------------------------//
        case(0):  //等待启动升级
                if((CAN.DLC==2)&&(CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0x22))
                {
                  Update_State=1;
                  Update_OverTime = 0;//清零超时计数
                }
        break;
        //------------------------------------------------------------//
        case(1): //判断(DSP识别码--0x0A--DSP2803x),及驱动器硬件识别码HardwareNumber

                 //加入升级超时判断,防止状态机进入死循环
                  Update_OverTime++;
                  if(Update_OverTime>10) //接收10次数据后，复位升级状态
                  {
                     Update_OverTime = 0;
                     Update_State = 0; //复位升级状态
                  }

                 if((CAN.DLC==8)&&\
                    (CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0xE5)&&\
                    (CAN.Rec[3]==0x00)&&(CAN.Rec[5]==0x00)&&\
                    (CAN.Rec[6]==0x00)&&(CAN.Rec[7]==0x00))
                 {
                     //if((CAN.Rec[2]==0x0A)&&(CAN.Rec[4]==HardwareNumber))
                     if((CAN.Rec[2]==0x0A)&&(CAN.Rec[4]==PA.Hardware_Code))
                     {
                       PWM1_OFF_MACRO()//升级前，停止电机运行
		               PWM2_OFF_MACRO()//升级前，停止电机运行
                       DINT;//程序跳转前，需关闭所有中断

                       I2CWD_PAR[0]= 0x01;//写入在线升级标志
                       Wri_eprom_addr = 0xFB;//向0xFB写入用户程序跳转标志0x01
                       I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
                       DELAY_US(3000);

                       (*DSP_Flash_Update)(); /*跳转到DSP Bootloader*/
                     }
                 }
        break;
      //-------------------------------------------------------------//
      default: break;
     } //end of switch
   }//主控ID判断
} //end of DSP在线升级相关处理函数
//---------------------------------------------------------------//

//---------------CAN接收中断--------------------//
#pragma CODE_SECTION(ECAN0INT_ISR, "ramfuncs");
interrupt void ECAN0INT_ISR(void)//eCAN-A接收中断
{
    //处理中断需14个指令周期
    //错误和状态寄存器（ECanaRegs.CANES.all）

    //----------邮箱2-(接收所有ID)---------------//
    if(ECanaRegs.CANRMP.all == 0x0004)//等待2号邮箱接收指令
    {
        ECanaRegs.CANRMP.all = 0x0004; //清掉接收标志

        /*step1: 接收状态指示LED*/
        CAN.Rec_Flag = 1; //置位CAN接收标志

        /*step2: 读取接收到数据的DLC及ID*/
        CAN.DLC = ECanaMboxes.MBOX2.MSGCTRL.bit.DLC;    //接收数据DLC
        CAN.ID  = ECanaMboxes.MBOX2.MSGID.bit.STDMSGID; //接收数据ID-(11bit)-28:18

        /*step3:读取邮箱中的数据*/
        CAN.Rec[0] = ECanaMboxes.MBOX2.MDL.byte.BYTE0;  //CAN-D0
        CAN.Rec[1] = ECanaMboxes.MBOX2.MDL.byte.BYTE1;  //CAN-D1
        CAN_YJ_GB();

        GZ_TEST_CAN_Handle();//工装测试收发函数

       //主控只有发送在线升级数据时，DLC=8
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
              if((Embroider.WorkState == 0x02)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 0x07))//只有在绿灯正绣+红灯正绣+红灯补绣3种状态下才处理送珠相关命令
              {
                  //CAN_Data_JPJM();
                  CAN_Data_SZJM();
              }
          }

          HeaderAddress_ReadAndWrite();//读写机头地址

          //AX8使能
#if(MachineControl == Ax8)
          {
              if((CAN.DLC==8)&&(CAN.Rec[0]==0x00FC)&&(CAN.Rec[1]==0x00B0)&&(CAN.Rec1[2]==0x0000)&&\
                 (CAN.Rec[3]==0x0030)&&(CAN.Rec[4]==0x0003)&&(CAN.Rec[5]==0x0000)&&\
                 (CAN.Rec[6]==0x0000)&&(CAN.Rec[7]==0x0000)) //使能命令
              {
                  TZ.CPUEnable = 1;
              }
          }

          //非AX8使能
#elif(MachineControl == GE)
          {
              /*if((CAN.DLC==8)&&(CAN.Rec[0]==0x00FC)&&(CAN.Rec[1]==0x00B0)&&(CAN.Rec[2]==0x0000)&&\
                 (CAN.Rec[3]==0x00A2)&&(CAN.Rec[4]==0x0001)&&(CAN.Rec[5]==0x0000)&&\
                 (CAN.Rec[6]==0x0000)&&(CAN.Rec[7]==0x0000)) //使能命令*/
              {
                  TZ.CPUEnable = 1;
              }
          }
#endif

          if((CAN.DLC==8)&&(CAN.Rec[0]==0xE0)&&(CAN.Rec[1]==0xEC)&&(CAN.Rec[2]==0x02))
          {
              if(((CAN.Rec[3]&0x0080)==0x0080)&&(TZ.Address==1))//末针参数&&左装置
              {
#if(MachineType==Three)
                  TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 530);//三色散珠
#else(MachineType==Double)
                  TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 580);//雙色散珠 普通机型
                  //TZ.Step1 = -((int16)((CAN.Rec[3]&0x007F)-60) + 594);//雙色散珠 特殊机型 朱工反馈 2019.10.14
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
              if(((CAN.Rec[3]&0x0080)==0x0000)&&(TZ.Address==2))//首针参数&&右装置
              {
#if(MachineType==Three)
                  TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 530);//三色散珠
#else(MachineType==Double)
                  TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 580);//双色散珠
                  //TZ.Step1 = -((int16)((CAN.Rec[3])-60) + 594);//双色散珠
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

              if(((CAN.Rec[4]&0x0080)==0x0080)&&(TZ.Address==1))//末针参数&&左装置
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
              if(((CAN.Rec[4]&0x0080)==0x0000)&&(TZ.Address==2))//首针参数&&右装置
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
              if(((CAN.Rec[5]&0x0080)==0x0080)&&(TZ.Address==1))//末针参数&&左装置
              {
                  TZ.JiaZhu = (CAN.Rec[5]&0x007F)*4 + 72;
              }
              if(((CAN.Rec[5]&0x0080)==0x0000)&&(TZ.Address==2))//首针参数&&右装置
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
              PARA = CAN.Rec[3]<<3;//临时参数，调节拨叉检测到光耦信号后走的距离；
          }*/
       }
       //DSP_Update_Handle();
   } // end of 邮箱2接收

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;//复位中断应答寄存器

}  //end of ECAN0INT_ISR

//---------CAN数据处理函数---------------//
#pragma CODE_SECTION(CAN_TX_HS, "ramfuncs");
void CAN_TX_HS(Uint16 state)
{
     CAN.Trs[2] = HeaderAddress.Address;
     CAN.Trs[5] = MC.Err_Value;
     CAN_TX_DLC8();//向主控发送
}

//-------勾线控制CAN通讯处理-----------------//
#pragma CODE_SECTION(CAN_TX_TZ, "ramfuncs");
void CAN_TX_TZ(Uint16 state)
{
    CAN.Trs[2] = HeaderAddress.Address;
    CAN.Trs[5] = MB.Err_Value;
    CAN_TX_DLC8();//向主控发送
}

#pragma CODE_SECTION(CAN_YJ_GB, "ramfuncs");
inline void CAN_YJ_GB(void)
{
    if(((CAN.ID&0x63F)==0)&&(CAN.DLC==2)) //主控ID判断
    {
        switch(CAN.Rec[0])
        {
        case 0x4F:
            if(CAN.Rec[1]==0x8D)//换珠到A片
            {
                HS.cmdFlag = 1;
                HS.cmdstate = 0;
            }
            else if(CAN.Rec[1]==0x8E)//换珠到B片
            {
                HS.cmdFlag = 2;
                HS.cmdstate = 1;
            }
            break;
        case 0xF0://冠翔三色散珠協議
            if(CAN.Rec[1]==0x51)//换珠到A珠
            {
                HS.cmdFlag = 1;
                HS.cmdstate = 0;
            }
            else if(CAN.Rec[1]==0x52)//换珠到B珠
            {
                HS.cmdFlag = 2;
                HS.cmdstate = 1;
            }
            /*else if(CAN.Rec[1]==0x54)//换珠到C珠
            {
                HS.cmdFlag = 3;
            }*/
            break;
        case 0xD0:
            Embroider.Speed = CAN.Rec[1];
            break;
        case 0xE0:
            if(CAN.Rec[1]==0x00)	    //零位
            {
                //四倍频脉冲出现异常，小于3900或者大于4100时会将脉冲计数值发到CAN总线,2019.11.06
                /*if((Embroider.AB_Pulse < 3900)||(Embroider.AB_Pulse > 4100))
                {
                    CAN_TX_DLC2(Embroider.AB_Pulse);//测试四倍频信号是否正确
                }*/
                //GpioDataRegs.AIOTOGGLE.bit.AIO12 = 1;//测量零位信号*/
                Embroider.AB_Pulse = 0;
            }
            else if(CAN.Rec[1]==0x01)	//平绣
            {
                Embroider.Type = 1;
            }
            else if(CAN.Rec[1]==0x03)	//正绣
            {
                Embroider.ControlState = 0;
                Embroider.ControlStateTrg = 0;
                Embroider.ControlStateEna = 0;
            }
            else if(CAN.Rec[1]==0x04)	//补绣
            {
                Embroider.ControlState = 1;
                if((Embroider.ControlStateTrg == 0)&&(Embroider.ControlStateEna == 0))
                {
                    Embroider.ControlStateTrg = 1;
                    Embroider.ControlStateEna = 1;
                    HS.Color_state_old = HS.Color_state;
                }
            }
            else if(CAN.Rec[1]==0x05)	//拉杆停车
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
            else if(CAN.Rec[1]==0x06)	//拉杆启动
            {
                Embroider.StartStop = 1;
                Embroider.StartStopFlag = 1;//拉杆启动触发标志位
                PrePearl_C.InvalidStart = 0;
                PrePearl_D.InvalidStart = 0;
                KeyHandle.OkTrgCnt = 0;//解决拉杆停车后手动按拨叉键，停在原点再次拉杆启动会导致停在原点报错
                if(BCorigin == 1)
                    TZ.cmd = 3;
            }
            else if(CAN.Rec[1]==0xFE)	//初始化完毕
            {
                Embroider.Init = 1;
            }
            else if(CAN.Rec[1]==0xF6)   //全机头写机头地址命令开始
            {
                HeaderAddress.StartWrite = 1;
                HeaderAddress.RedOFFFlag = 1;
            }
            else if(CAN.Rec[1]== 0xF7)   //全机头写机头地址命令结束
            {
                HeaderAddress.StartWrite = 0;
                HeaderAddress.WriteOver = 0;
            }
            break;
            //---------------------------------------------------------------------------------

        case 0x1f://气阀升降
            switch(CAN.Rec[1])
            {
            case 0x81://升装置
                Air1_Valve_Set = 1;
                Air2_Valve_Set = 1;
                Embroider.AirValvePos = 1;
                Embroider.AirValveState = 1;//抬装置 2019.10.08
                break;
            case 0x82://降装置
                Embroider.AirValveState = 0;//降装置 2019.10.08
                if(((Embroider.PearlNeedle == 2)&&(TZ.Address == 1))||((Embroider.PearlNeedle == 1)&&(TZ.Address == 2)))
                {
                    if((Embroider.WorkState == 2)||(Embroider.WorkState == 3)||(Embroider.WorkState == 7))//解决红灯未补绣时不下装置的问题2019.02.25
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
                    Embroider.PearlNeedle = 1;//散珠右装置
                    if(MC.State == ST_Run)
                    {
                        KeyHandle.DisplayData = 0x0040;//"-"恢复显示默认状态
                        KeyHandle.DisplayEnable = 1;
                    }
                    break;
                case 0x72:
                    Embroider.PearlNeedle = 2;//散珠左装置
                    if(MC.State == ST_Run)
                    {
                        KeyHandle.DisplayData = 0x0040;//"-"恢复显示默认状态
                        KeyHandle.DisplayEnable = 1;
                    }
                    break;
                case 0x73:
                    Embroider.PearlNeedle = 0;//非散珠针位
                    TZ.MotorEnableFlag = 1;
                    break;
                default:
                    break;
                }
                break;
        default:
            break;
        }
        if((Embroider.WorkState == 0x02)||(Embroider.WorkState == 0x03)||(Embroider.WorkState == 0x07))//只有在绿灯正绣+红灯正绣+红灯补绣3种状态下才处理送珠相关命令
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
                PushPearl_Handle(1);//1:A珠
                PrePearl_C.Timer = 0;
            }
            if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x60))
            {
                PushPearl_Handle(2);//2:B珠
                PrePearl_D.Timer = 0;
            }
            if(Embroider.StartStop == 0)
            {
                if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x55))
                {
                    if(HS.Color_state != TZ.color)
                    {
                        //A回退动作
                        TZ.cmd = 2;
                        TZ.cmdFlag = 0;
                    }

                }
                if((CAN.Rec[0]==0x4F)&&(CAN.Rec[1]==0x56))
                {
                    if(HS.Color_state != TZ.color)
                    {
                        //B回退动作
                        TZ.cmd = 2;
                        TZ.cmdFlag = 0;
                    }
                }
            }
        }
        if((CAN.Rec[0]==0xFA)&&(CAN.Rec[1]==0x30))
        {
            //全头换珠开始
            TZ.All_START = 1;
        }
        if((CAN.Rec[0]==0xFA)&&(CAN.Rec[1]==0x31))
        {
            //全头换珠结束
            TZ.All_START = 0;
        }
    }
}

//-------CAN通讯处理(程序框架)-----------------//
#pragma CODE_SECTION(CAN_Handle, "ramfuncs");
void CAN_Handle(void)
{
    if(CAN.Rec_Flag == 1)//CAN接收中断中置接收标志
    {
       CAN.Rec_Flag = 0; //清CAN接收标志

       if(Embroider.LightState == 3)
       {
           DSP_Update_Handle();//在线升级处理(MD612无需DIP状态判断)
       }
    }
}
void DecryptOutputDataHandle()
{
    /*---------散珠指令加密规则----------------------
    02 02 D1 D2 R1 R2 R3 R4  其中 D1 D2是同步散珠回退指令
    02 01 D1 D2 R1 R2 R3 R4  其中 D1 D2是同步散珠送片指令
    1E 04 D1 D2 S1 0  0  0   其中S1 对应具体指令
    -----------------END-----------------------*/
    if((EncryptBlock[0] == 0x02)&&(EncryptBlock[1] == 0x02)) //回退
    {
        CAN.back1 = EncryptBlock[2];
        CAN.back2 = EncryptBlock[3];
    }
    else if((EncryptBlock[0] == 0x02)&&(EncryptBlock[1] == 0x01)) //送珠
    {
        CAN.Forward1 = EncryptBlock[2];
        CAN.Forward2 = EncryptBlock[3];
    }
}
//加密后的散珠指令操作
#pragma CODE_SECTION(CAN_Data_SZJM, "ramfuncs");
inline void CAN_Data_SZJM(void)
{
    if((CAN.Rec[0] == 0x1E)&&(CAN.Rec[1] == 0x04)&&(CAN.Rec[2] == CAN.Forward1)&&(CAN.Rec[3] == CAN.Forward2) )
    {
        if(CAN.Rec[4] == 0x81)//A珠送珠
        {
            PushPearl_Handle(1);//1:A珠
        }
        else if(CAN.Rec[4] == 0x82)//B珠送珠
        {
            PushPearl_Handle(2);//2:B珠
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
            //A送珠动作
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//停车状态
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A送珠动作
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
            //A送珠动作
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//停车状态
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A送珠动作
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
            //A送珠动作
            TZ.cmd = 1;
        }
    }
    else
    {
        if(Embroider.StartStop == 0)//停车状态
        {
            if(HS.Color_state != TZ.color)
            {
                TZ.cmd = 1;
            }
        }
        else
        {
            //A送珠动作
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
