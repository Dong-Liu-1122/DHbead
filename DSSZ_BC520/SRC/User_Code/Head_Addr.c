#include "F2806x_Device.h"     //DSP28030外设定义头文件
#include "Stepper-Settings.h"

HeaderAddress_PARA HeaderAddress = HeaderAddress_PARA_DEFAULTE;
//Uint16 ReadEEPROMCounter = 0;
extern Uint16 LightState;
extern Uint16 Wri_eprom_addr;
extern Uint16 I2CRD_PAR[8]; //I2C读出数组
extern Uint16 I2CWD_PAR[2]; //I2C写入数组

void WriteHeaderAddress(void);
void CheckHeaderAddress(void);
extern void CAN_TRS_MBOX3(Uint16 trs_data_D0D1,Uint16 trs_data_D2D3);
extern Uint16 I2CA_WriteData( Uint16  RomAddress , Uint16 *Wdata, Uint16  number);
extern Uint16 Read_Par(Uint16 Read_eprom_addr);


void HandleHeaderAddress(void)
{
	if(HeaderAddress.Command == 1)
	{
		WriteHeaderAddress();
	}
/*
	else if(HeaderAddress.Command == 2)
	{
		CheckHeaderAddress();
	}
	else if(HeaderAddress.Command == 3)
	{}
	*/
}




//-------------------------------------------------------------------------
void WriteHeaderAddress(void)
{
	if((Embroider.LightState == RED)&&(HeaderAddress.StartWrite == 1))
	{
		I2CWD_PAR[0]= HeaderAddress.Address;//
		Wri_eprom_addr = 0x07;//
		I2CA_WriteData(Wri_eprom_addr,&I2CWD_PAR[0],1);
		HeaderAddress.WriteOver = 2;
		HeaderAddress.Command = 0;
		HeaderAddress.StartWrite = 0;
		//PA.Head_Add = Read_Par(0x00);
		//HeaderAddress.ReadAddress = (HeaderAddress.ReadAddress<<8)|HeaderAddress.GroupNumber;
		//HeaderAddress.GroupNumber = 0;
		//CAN_TRS_MBOX3(0x0115,HeaderAddress.ReadAddress);   ////CAN  15  01  01(group)  01(addr)
		//HeaderAddress.ReadAddress = 0;
	}
}
//-------------------------------------------------------------------------------------------
/*
void CheckHeaderAddress(void)
{
	if(ReadEEPROMCounter == 0)
	{
		HeaderAddress.ReadAddress = Read_Par(0x00);
		ReadEEPROMCounter = 1;
	}
	if(HeaderAddress.ReadAddress == HeaderAddress.Address)
	{
		//HeaderAddress.ReadAddress = (HeaderAddress.ReadAddress<<8)|HeaderAddress.GroupNumber;
		//CAN_TRS_MBOX3(0x0215,HeaderAddress.ReadAddress);     //CAN  15  02  01(group)  01(addr)
		HeaderAddress.WriteOver = 1;
		HeaderAddress.Command = 0;
	}
	else
	{
		// HRD1_RED_DSP_OFF = 1;
		// HRD1_GREEN_DSP_OFF = 1;
		 HeaderAddress.Command = 0 ;
	}

}
*/
//------------------------------------------------------------------------------
void HeaderAddress_ReadAndWrite(void)
{
	if((CAN.DLC==8)&&(CAN.Rec[0]==0x00E0)&&(CAN.Rec[1]==0x00EA)&&(CAN.Rec[5]==0x0000)&&(CAN.Rec[6]==0x0000)&&(CAN.Rec[7]==0x0000) )
	{
		//写地址，读地址（机头）
		HeaderAddress.Command = CAN.Rec[2];
		HeaderAddress.GroupNumber = CAN.Rec[3];
		HeaderAddress.Address = CAN.Rec[4];
		//ReadEEPROMCounter = 0;   //读取EEProm次数
		//HeaderAddress.StartCheck = 1 ;
	}
}


