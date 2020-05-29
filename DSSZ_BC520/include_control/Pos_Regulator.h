/**********************************************************
*版权所有2015，大豪
*文件名称：//Pos_Regulator.h
*内容摘要：//Pos_Regulator.c的头文件，定义该C文件所需要的常量、结构体
*其他说明：//
*当前版本：//V0.0
*作    者：//王彬杰
*完成日期：//2015年03月02日
*修改记录：//
*  记录1：
*      修改日期：
*      版 本 号：
*      修 改 人：
*      修改日期：
*      修改内容：
*********************************************************/
#ifndef Pos_Regulator_h
#define Pos_Regulator_h

/************************************************************************
位置环整合
************************************************************************/


typedef struct POS_REG
{
    Uint32 lPulCount_In; //外部脉冲计数器输入
    Uint32 lPulCount;    //脉冲计数器选择后的值,位置误差在这里计算
    Uint32 lPulCount_Old;//脉冲计数器上周期值
    int32  lPulCount_Err;//脉冲计数器差值，赋值给比例和前馈环节(位置偏差）
    Uint32 lFeedBack;    //反馈脉冲计数器
    Uint32 lFeedBack_Old;//上一拍反馈脉冲计数器
    int32  lFeedBack_Err;//反馈脉冲误差值，有正负
    int32  lProp_In;     //比例通道输入Q0,当作是Q18
    int32  lFor_In;      //前馈输入Q15
    int32  lSpd_Ref;     //速度指令给定
    int16  iVol_Lim;     //速度限制
    Uint16 iEncoderLine;//编码器线数
}T_POS_REG;

#define tPosReg_default {0,0,0,0,0,0,0,0,0,0,30000,1000}

/************************************************************************
前馈通道
************************************************************************/

//前馈滤波器结构体
typedef struct FORWARD_FILTER
{
    int32  temp;
    int32  lTc;      //时间常数Q0
    int32  lTs;      //采样时间Q0
    int32  lKa;      //前馈滤波器系数Q30
    int32  lKb;      //前馈滤波器系数Q30
    int32  lInPut;   //前馈滤波器输入q20
    int32  lOutPut;  //前馈滤波器输出q20
    int32  lInPutOld;//前馈滤波器上一拍输入
    int32  lOutPutOld;//前馈滤波器上一拍输出
    void(*ForFilInit)();//前馈滤波器初始化
}T_FORWARD_FILTER;

#define tForFilter_default {0,0,1,0,0,0,0,0,0,\
                           (void(*)(long))ForFilInit}


//电子齿轮结构体，前馈通道
typedef struct ELEC_GEAR
{
    Uint16 uNum;//分子
    Uint16 uDen;//分母
    Uint32 uResult;//Q30分母的倒数
    int32  lEGear_In;
    int32  lEGear_Out;//电子齿轮输出,给前馈滤波器
    Uint16 EGear_Err;//电子齿轮错误标志
    void(*EGear_Init)();//电子齿轮初始化函数
}T_ELEC_GEAR;

#define tEGear_default {0,0,0,0,0,0,(void(*)(long))EGear_Init}


//微分结构体
typedef struct DIFFER
{
    int32  lForLimit;//微分器限幅值Q15,1500转32767
    int32  lDiff_In;//微分输入
    int32  lDiff_Out;//微分输出
}T_DIFFER;

#define tDiffer_default {32767,0,0}

//前馈通道结构体
typedef struct POS_FORWARD
{
    Uint16 uKP_fwd;//前馈系数，要在用户更改完成后进行格式转换成Q格式,单位是百分比
    Uint16 uFLT_t;//前馈滤波器时间参数，单位是0.01ms
    Uint16 uEnum;//电子齿轮比（分子）
    Uint16 uEden;//电子齿轮比（分母）
    Uint16 uClrStat;//清除状态,如果为真则清除位置偏差计数器，并使位置环无效
    int16  iPulCount_Err;//两个采样周期之间的脉冲计数器差值
    int32  lGain_Out;//前馈增益输出
    int32  lFor_Out;//前馈输出
    T_DIFFER tDiffer;//微分结构体
    T_ELEC_GEAR tEGear;//电子齿轮结构体
    T_FORWARD_FILTER tForFilter;//前馈滤波器结构体
}T_POS_FORWARD;


#define tPosFor_default {0,0,0,0,0,0,0,0,\
                        tDiffer_default,\
                        tEGear_default,\
                        tForFilter_default}
//前馈通道函数声明

void EGear_Init(T_POS_FORWARD *p);
void ForFilInit(T_POS_FORWARD *p);
/************************************************************************
比例通道
************************************************************************/
//偏差计数器
typedef struct Err_Count
{
    Uint16 uNum;        //分子
    Uint16 uDen;        //分母
    Uint32 uDivNum;     //Q30,分子的倒数
    int64 lPosCount_Err;//位置增量值
    int64 lPosCount_Num;//位置计数器带小数
    int64 lFeed_Den;    //反馈脉冲与电子齿轮分母的乘积
    int32 lFeedBack_Err;//反馈脉冲误差输入
    int64 lErrCount_Sum;//偏差计数器累加
    int64 lErrCount_Sum_Old;
    int16 iErr_Display; //将lErrCount_Sum,四舍五入后用于显示
    int32 lErrCount_Int;//偏差计数器
    int32 lErrCount_Out;//偏差计数器输出值
//    Uint32 uPos_Lim;       //标志位
//    Uint16 uFlag2;
    Uint16 uClrStat;    //偏差计数器清零使能位,0:不清零，1：清零
    int64 lRef_Sum;     //滤波器之前的偏差值的累加
    int32 lRef_Int;     //除以分子后的偏差值
//    int64 lDelta_Err;   //滤波器前后的给定误差
    int32 iDelta_Integ; //滤波器前后偏差的整数值
}T_Err_Count;

#define tErrCounter_default {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

//比例通道
typedef struct POS_PROP
{
    Uint16 uKp_Pos;//位置环增益
//    Uint16 uPn200;//200.1选择清除信号状态，0-高电平，1-上升沿，2-低电平，3-下降沿
    Uint16 uEnum;//分子
    Uint16 uEden;//分母
//    Uint16 uPn204;//位置指令加减速时间参数
//    Uint16 uPn207;//207.0选择位置指令滤波器
//    Uint16 uPn208;//平均移动滤波器时间
    Uint16 uCLR;//输入信号接收外部CLR信号，0-高电平，1-上升沿，2-低电平，3-下降沿
    Uint16 uSVON;//输入信号伺服ON状态
    Uint16 uErrStat;//输入信号故障状态
    Uint16 uPower;//输入信号主电源状态
//    Uint16 iPn500;//定位完成信号
//    Uint16 iPn504;//定位附近信号
    int16  iPosCount_Err;//当前位置计数器增量值，有符号，表示正负
    int16  iPosCount_Err_Old;
//    Uint16 iErr_Flag;    //表示增量值的正负，1表示正，0表示负
    int32  lFeedBack_Err;//输入反馈脉冲误差
    int32  lErrCount;//偏差计数器
    int16  iProp_Out;//比例通道输出
    int32  lProp_Out;//比例通道输出之前，用于计算
    Uint16 uClrStat;//清除状态,如果为真则清除位置偏差计数器，并使位置环无效
	int16  iLimit;//
    Uint16 u_COIN;//定位完成输出信号，低电平_定位完成；高电平_定位未完成。
//    Uint16 u_NEAR;//定位附近输出信号，低电平_到达附近；高电平_未到附近。
    Uint16 uPos_Flg;//01:位置超差，02:齿轮比设置错误，04:定位完成。
    T_Err_Count      tErrCounter;//偏差计数器
}T_POS_PROP;

#define tPosPro_default {1,1,1,0,0,0,1,0,0,0,0,0,0,0,30000,0,0,\
                        tErrCounter_default}

////////////////////////////////////////////////////////////////////
#define DivTp 600 //位置环前馈采样（位置环为5个电流周期，前馈为4个）时间10khz Q0,(10000*60/4000)=600,换算成转/分
#define DivTpQ15   8192 //DivTp*32767/2400=8192,速度 基准为 2400rpm
#define Over_Band	20000L
extern T_POS_REG     tPosReg_x;
extern T_POS_FORWARD tPosFor_x;    //位置环前馈
extern T_POS_PROP    tPosPro_x;   //位置环比例

extern T_POS_PROP    tPosPro_y;
/////////////////////////该函数相应的宏//////////////////
#define PosChoiceMACRO(p)											\
{																	\
/*反馈脉冲偏差*/														\
     p.lFeedBack_Err = p.lFeedBack - p.lFeedBack_Old;				\
/*QEP设定最大值为4000*/											     	\
	if(p.lFeedBack_Err < -(int32)p.iEncoderLine>>1)					\
	{																\
		p.lFeedBack_Err = (int32)p.iEncoderLine + p.lFeedBack_Err;	\
	}																\
	else if(p.lFeedBack_Err > (int32)p.iEncoderLine>>1)				\
	{																\
		p.lFeedBack_Err = p.lFeedBack_Err - (int32)p.iEncoderLine;	\
	}																\
																	\
    p.lFeedBack_Old = p.lFeedBack;									\
}

/////////////////////////该函数相应的宏//////////////////
#define VolRefMACRO(p)									\
{														\
    p.lSpd_Ref = p.lProp_In ; /*Q24  +(p.lFor_In)*/		\
/*速度给定限幅*/											\
    if(p.lSpd_Ref > ((int32)p.iVol_Lim)<<9)				\
    {													\
        p.lSpd_Ref = ((int32)p.iVol_Lim)<<9;			\
    }													\
    if(p.lSpd_Ref < -((int32)p.iVol_Lim)<<9)			\
    {													\
        p.lSpd_Ref = -((int32)p.iVol_Lim)<<9;			\
    }													\
}
/////////////////////////该函数相应的宏//////////////////////////////////
#define DifferMACRO(p)													\
{																		\
    p.lDiff_Out = (p.lDiff_In * DivTpQ15);/*微分得速度值，Q15格式*/		    \
																		\
    if(p.lDiff_Out > p.lForLimit)										\
    {																	\
        p.lDiff_Out = p.lForLimit;										\
    }																	\
    if(p.lDiff_Out < -p.lForLimit)										\
    {																	\
        p.lDiff_Out = -p.lForLimit;										\
    }																	\
}
void EGear_Init(T_POS_FORWARD *p)// 0.01 <= 电子齿轮比 <= 100
{
    int32 temp;
    temp = 0x40000000L / p->tEGear.uDen;//Q30
    p->tEGear.uResult = ((int64)p->tEGear.uNum * temp)>>6;//Q24
//判断范围不能超过0.01~100
    if((p->tEGear.uResult <= 0x28F5cL) || (p->tEGear.uResult >= 0x64000000L))
    {
		p->tEGear.EGear_Err = 1;
		p->tEGear.uNum = 1;
		p->tEGear.uDen = 1;
//    	asm("   ESTOP0");//表示错误
//		error();
    }
}// End of function
/////////////////////////该函数相应的宏/////////////////////////////////////////////////
#define EGearMACRO(p)/* 0.01 <= 电子齿轮比 <= 100*/										\
{																					\
    p.lEGear_Out = ((int64)p.uResult * p.lEGear_In )>>24;/*Q0*/						\
}

//前馈滤波器初始化
void ForFilInit(T_POS_FORWARD *p)
{
   if (p->tForFilter.lTc<<1 >= p->tForFilter.lTs)
   {
//lKa,lKb,Q30
        p->tForFilter.lKa = ((int64)p->tForFilter.lTs<<30) / ((p->tForFilter.lTs) + (p->tForFilter.lTc<<1)); //Q30 Ka=Ts/(Ts+2*Tc)
        p->tForFilter.lKb = 0x40000000L - (p->tForFilter.lKa<<1) ; // Kb=1-2*Ka, Q30=0x8000L
    }
    else
   {
        p->tForFilter.lKa = 0;
        p->tForFilter.lKb = 0;
    }
}// End of function
/////////////////////////该函数相应的宏/////////////////////////////////////////////////
int64 temp_64;
#define ForwardFilterMACRO(p)															\
{																						\
   if ((p.lKa == 0) && (p.lKb == 0))													\
   {																					\
       p.lOutPut = p.lInPut;															\
    }																					\
   else																					\
   {																					\
       temp_64 = ((int64)p.lKa * (p.lInPut + p.lInPutOld)) + ((int64)p.lKb * p.lOutPutOld);\
       p.lOutPut = temp_64/0x40000000L;													\
       p.lOutPutOld = p.lOutPut;														\
       p.lInPutOld  = p.lInPut;															\
    }																					\
}

Uint16 temp_pos;
#define PosForwardMacro(p)																	\
{																							\
/*电子齿轮*/																					\
    p.tEGear.lEGear_In = p.iPulCount_Err;													\
	EGearMACRO(p.tEGear)																	\
/*微分计算*/																					\
    if(p.uClrStat == 0)/*表示不清除*/														    \
    {																						\
	    p.tDiffer.lDiff_In = p.tEGear.lEGear_Out;											\
    }																						\
    if(p.uClrStat == 1)/*表示清除*/															\
    {																						\
        p.tDiffer.lDiff_In = 0;																\
	}																						\
	DifferMACRO(p.tDiffer)																	\
	/*前馈增益*/																				\
    temp_pos= ((int32)p.uKP_fwd * 328);/*Q15,单位是百分比32767/100=328*/						  \
    p.lGain_Out = (int16)((p.tDiffer.lDiff_Out * temp_pos)>>6);/*前馈系数,得到(Q15*Q15)>>6=Q24*/  \
    /*前馈滤波器计算*/																		    \
    p.tForFilter.lInPut = (p.lGain_Out);/*Q24*/												\
	ForwardFilterMACRO(p.tForFilter);														\
    p.lFor_Out = p.tForFilter.lOutPut;/*Q24*/												\
}
/////////////////////////该函数相应的宏//////////////////////////////
volatile Uint16 Pos_offset_Ena=0; //位置偏差补偿使能
volatile int16  Pos_offset_Set=0; //位置偏差补偿值（单位：编码器脉冲数)
//位置脉冲偏差计算
#define ErrCountMACRO(p)										\
{																\
/*64位，反馈脉冲误差乘分母,全部放到高32位,Q32*/					        \
    p.lFeed_Den = (((int64)p.lFeedBack_Err)<<32) * p.uDen;		\
/*64位,指令脉冲误差乘分子,Q32,64位带小数累加,注意溢出*/			            \
    p.lPosCount_Num = (p.lPosCount_Err<<11) * p.uNum;/*Q32*/	\
/*偏差计数清零或累加*/											    \
    if(p.uClrStat == 0)											\
    {															\
        /*******加入位置偏差补偿功能（比如：绣框XY的机械回差补偿)********/    \
         if(Pos_offset_Ena==1)                                  \
         {                                                      \
             Pos_offset_Ena = 0;                                \
             p.lErrCount_Sum += p.lPosCount_Num - p.lFeed_Den + Pos_offset_Set;/*Q32*/ \
         }                                                      \
         else                                                   \
         {                                                      \
             p.lErrCount_Sum += p.lPosCount_Num - p.lFeed_Den;/*Q32*/ \
         }                                                      \
    }															\
    else if(p.uClrStat == 1)									\
    {															\
        p.lErrCount_Sum = 0;									\
    }															\
																\
    p.lErrCount_Int = (int32)((p.lErrCount_Sum>>32) & 0x0FFFFFFFFL);\
/*取累积误差的商*/												    \
     p.lErrCount_Out = p.lErrCount_Int/ p.uDen ;                \
}                                                               \

//位置环计算
#define PosPropMacro0(p)                                                                       \
{                                                                                              \
/*偏差计数器计算*/                                                                                 \
     p.tErrCounter.uDen = p.uEden;                                                             \
     p.tErrCounter.uNum = p.uEnum;                                                             \
     p.tErrCounter.lFeedBack_Err = p.lFeedBack_Err;/*Q0,32位*/                                  \
     p.tErrCounter.lPosCount_Err = ((int32)p.iPosCount_Err)<<21;/*Q0->Q21*/                    \
     p.tErrCounter.uClrStat = p.uClrStat;/*清零标志*/                                             \
}

#define PosPropMacro1(p)																		\
{																								\
   ErrCountMACRO(p.tErrCounter)																    \
}

#define PosPropMacro2(p)                                                                        \
{                                                                                               \
     /*FAL比例调节器*/                                                                             \
     p.lProp_Out = (int32)((int64)FAL(p.tErrCounter.lErrCount_Out) * (p.uKp_Pos)>>6);/*Q24*/    \
     /*限幅*/                                                                                    \
     if(p.lProp_Out > ((int32)p.iLimit)<<9)                                                     \
     {                                                                                          \
     p.lProp_Out = ((int32)p.iLimit)<<9;                                                        \
     }                                                                                          \
     if(p.lProp_Out < -((int32)p.iLimit)<<9)                                                    \
     {                                                                                          \
     p.lProp_Out = -((int32)p.iLimit)<<9;                                                       \
     }                                                                                          \
     p.iProp_Out = (int16)p.lProp_Out;                                                          \
     p.iPosCount_Err_Old = p.iPosCount_Err;                                                     \
}


//为减少每次运行时间，将位置环分3次计算
#define POS_PD_X1Macro()												\
{																		\
/*偏差计数器清零判断,伺服OFF或主电源OFF或发生警报*/								    \
    if(tPosPro_x.uCLR == 1)                                             \
    {																	\
        tPosPro_x.uClrStat = 1;/*清零*/									\
    }																	\
    else																\
    {																	\
  	    tPosPro_x.uClrStat = 0;/*不清零*/									\
	}																	\
	PosChoiceMACRO(tPosReg_x)											\
	tPosPro_x.lFeedBack_Err = tPosReg_x.lFeedBack_Err;					\
	PosPropMacro0(tPosPro_x) /*脉冲误差计算参数初始化*/                        \
}


//为减少每次运行时间，将位置环分3次计算
#define POS_PD_Y1Macro()												\
{																		\
/*偏差计数器清零判断,伺服OFF或主电源OFF或发生警报*/								    \
    if(tPosPro_y.uCLR == 1)                                             \
    {																	\
        tPosPro_y.uClrStat = 1;/*清零*/									\
    }																	\
    else																\
    {																	\
  	    tPosPro_y.uClrStat = 0;/*不清零*/									\
	}																	\
	PosChoiceMACRO(tPosReg_y)											\
	tPosPro_y.lFeedBack_Err = tPosReg_y.lFeedBack_Err;					\
	PosPropMacro0(tPosPro_y) /*脉冲误差计算参数初始化*/                        \
}

#define POS_PD_X2Macro()												\
PosPropMacro1(tPosPro_x) /*脉冲误差计算(计算需3us)*/

#define POS_PD_Y2Macro()												\
PosPropMacro1(tPosPro_y) /*脉冲误差计算(计算需3us)*/

#define POS_PD_X3Macro()                                                \
PosPropMacro2(tPosPro_x)  /*位置环FAL计算*/                                \
tPosPro_x.iPosCount_Err = 0;/*清零*/                                     \
tPosReg_x.lSpd_Ref = tPosPro_x.lProp_Out;/*Q24*/

#define POS_PD_Y3Macro()                                                \
PosPropMacro2(tPosPro_y)  /*位置环FAL计算*/                                \
tPosPro_y.iPosCount_Err = 0;/*清零*/                                     \
tPosReg_y.lSpd_Ref = tPosPro_y.lProp_Out;/*Q24*/

/**************************************************************************************
 函数功能：非线性函数fal
入口参数: 位置偏差 int32 e
返回参数：  fal非线性输出 iq
 **************************************************************************************/
//非线性函数fal
#define  EDIVQ24        1678    //Q24,1/30000->559,1/100000->168, 1/20000->839, 1/10000->1678
#define  alpha  8388608//Q24,0.5
const Uint32    delta[5]={1677722,503316,167772,50331,16777};   //Q24..0.001->16777;0.003->50331;0.01->167772;0.03->503316;0.1->1677722
const Uint16    K1[5]={3,6,10,18,32};   //Q0..0.001->32;0.003->18;0.01->10;0.03->6;0.1->3

#pragma CODE_SECTION(FAL, "ramfuncs");
inline _iq FAL(register int32 e)
{
	 register _iq Fal_Out,Fal_Tmp;
	    register int Sgn;
	    static   int i=5; //Transfer_Para.Reserve3;(可根据实际选择i=1~5）

	    Fal_Tmp = e * EDIVQ24;//进行标幺化

	    if(i == 0)
	    {
	        return(e<<11);//不使用非线性控制，考虑到要右移5位，并乘以Q5格式增益，并左移6位，转换成Q24位速度指令。
	    }
	    if(Fal_Tmp > 0)  {Sgn = 1;}
	    if(Fal_Tmp < 0)  {Sgn = -1;}
	    if(Fal_Tmp == 0) {Sgn = 0;}
	/*
	    if(_IQabs(Fal_Tmp) <= delta[i-1])
	    {
	        Fal_Out = Fal_Tmp * K1[i-1] + Fal_Tmp;
	    }
	    else
	    {
	        Fal_Tmp = _IQabs(Fal_Tmp);
	        Fal_Out = ( _IQsqrt(Fal_Tmp) + Fal_Tmp) * Sgn;
	    }
	    */
	    Fal_Out = Fal_Tmp * K1[i-1] + Fal_Tmp;

	    return Fal_Out;
}


#endif

