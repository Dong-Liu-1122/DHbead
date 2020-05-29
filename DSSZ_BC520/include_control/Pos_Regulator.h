/**********************************************************
*��Ȩ����2015�����
*�ļ����ƣ�//Pos_Regulator.h
*����ժҪ��//Pos_Regulator.c��ͷ�ļ��������C�ļ�����Ҫ�ĳ������ṹ��
*����˵����//
*��ǰ�汾��//V0.0
*��    �ߣ�//�����
*������ڣ�//2015��03��02��
*�޸ļ�¼��//
*  ��¼1��
*      �޸����ڣ�
*      �� �� �ţ�
*      �� �� �ˣ�
*      �޸����ڣ�
*      �޸����ݣ�
*********************************************************/
#ifndef Pos_Regulator_h
#define Pos_Regulator_h

/************************************************************************
λ�û�����
************************************************************************/


typedef struct POS_REG
{
    Uint32 lPulCount_In; //�ⲿ�������������
    Uint32 lPulCount;    //���������ѡ����ֵ,λ��������������
    Uint32 lPulCount_Old;//���������������ֵ
    int32  lPulCount_Err;//�����������ֵ����ֵ��������ǰ������(λ��ƫ�
    Uint32 lFeedBack;    //�������������
    Uint32 lFeedBack_Old;//��һ�ķ������������
    int32  lFeedBack_Err;//�����������ֵ��������
    int32  lProp_In;     //����ͨ������Q0,������Q18
    int32  lFor_In;      //ǰ������Q15
    int32  lSpd_Ref;     //�ٶ�ָ�����
    int16  iVol_Lim;     //�ٶ�����
    Uint16 iEncoderLine;//����������
}T_POS_REG;

#define tPosReg_default {0,0,0,0,0,0,0,0,0,0,30000,1000}

/************************************************************************
ǰ��ͨ��
************************************************************************/

//ǰ���˲����ṹ��
typedef struct FORWARD_FILTER
{
    int32  temp;
    int32  lTc;      //ʱ�䳣��Q0
    int32  lTs;      //����ʱ��Q0
    int32  lKa;      //ǰ���˲���ϵ��Q30
    int32  lKb;      //ǰ���˲���ϵ��Q30
    int32  lInPut;   //ǰ���˲�������q20
    int32  lOutPut;  //ǰ���˲������q20
    int32  lInPutOld;//ǰ���˲�����һ������
    int32  lOutPutOld;//ǰ���˲�����һ�����
    void(*ForFilInit)();//ǰ���˲�����ʼ��
}T_FORWARD_FILTER;

#define tForFilter_default {0,0,1,0,0,0,0,0,0,\
                           (void(*)(long))ForFilInit}


//���ӳ��ֽṹ�壬ǰ��ͨ��
typedef struct ELEC_GEAR
{
    Uint16 uNum;//����
    Uint16 uDen;//��ĸ
    Uint32 uResult;//Q30��ĸ�ĵ���
    int32  lEGear_In;
    int32  lEGear_Out;//���ӳ������,��ǰ���˲���
    Uint16 EGear_Err;//���ӳ��ִ����־
    void(*EGear_Init)();//���ӳ��ֳ�ʼ������
}T_ELEC_GEAR;

#define tEGear_default {0,0,0,0,0,0,(void(*)(long))EGear_Init}


//΢�ֽṹ��
typedef struct DIFFER
{
    int32  lForLimit;//΢�����޷�ֵQ15,1500ת32767
    int32  lDiff_In;//΢������
    int32  lDiff_Out;//΢�����
}T_DIFFER;

#define tDiffer_default {32767,0,0}

//ǰ��ͨ���ṹ��
typedef struct POS_FORWARD
{
    Uint16 uKP_fwd;//ǰ��ϵ����Ҫ���û�������ɺ���и�ʽת����Q��ʽ,��λ�ǰٷֱ�
    Uint16 uFLT_t;//ǰ���˲���ʱ���������λ��0.01ms
    Uint16 uEnum;//���ӳ��ֱȣ����ӣ�
    Uint16 uEden;//���ӳ��ֱȣ���ĸ��
    Uint16 uClrStat;//���״̬,���Ϊ�������λ��ƫ�����������ʹλ�û���Ч
    int16  iPulCount_Err;//������������֮��������������ֵ
    int32  lGain_Out;//ǰ���������
    int32  lFor_Out;//ǰ�����
    T_DIFFER tDiffer;//΢�ֽṹ��
    T_ELEC_GEAR tEGear;//���ӳ��ֽṹ��
    T_FORWARD_FILTER tForFilter;//ǰ���˲����ṹ��
}T_POS_FORWARD;


#define tPosFor_default {0,0,0,0,0,0,0,0,\
                        tDiffer_default,\
                        tEGear_default,\
                        tForFilter_default}
//ǰ��ͨ����������

void EGear_Init(T_POS_FORWARD *p);
void ForFilInit(T_POS_FORWARD *p);
/************************************************************************
����ͨ��
************************************************************************/
//ƫ�������
typedef struct Err_Count
{
    Uint16 uNum;        //����
    Uint16 uDen;        //��ĸ
    Uint32 uDivNum;     //Q30,���ӵĵ���
    int64 lPosCount_Err;//λ������ֵ
    int64 lPosCount_Num;//λ�ü�������С��
    int64 lFeed_Den;    //������������ӳ��ַ�ĸ�ĳ˻�
    int32 lFeedBack_Err;//���������������
    int64 lErrCount_Sum;//ƫ��������ۼ�
    int64 lErrCount_Sum_Old;
    int16 iErr_Display; //��lErrCount_Sum,���������������ʾ
    int32 lErrCount_Int;//ƫ�������
    int32 lErrCount_Out;//ƫ����������ֵ
//    Uint32 uPos_Lim;       //��־λ
//    Uint16 uFlag2;
    Uint16 uClrStat;    //ƫ�����������ʹ��λ,0:�����㣬1������
    int64 lRef_Sum;     //�˲���֮ǰ��ƫ��ֵ���ۼ�
    int32 lRef_Int;     //���Է��Ӻ��ƫ��ֵ
//    int64 lDelta_Err;   //�˲���ǰ��ĸ������
    int32 iDelta_Integ; //�˲���ǰ��ƫ�������ֵ
}T_Err_Count;

#define tErrCounter_default {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

//����ͨ��
typedef struct POS_PROP
{
    Uint16 uKp_Pos;//λ�û�����
//    Uint16 uPn200;//200.1ѡ������ź�״̬��0-�ߵ�ƽ��1-�����أ�2-�͵�ƽ��3-�½���
    Uint16 uEnum;//����
    Uint16 uEden;//��ĸ
//    Uint16 uPn204;//λ��ָ��Ӽ���ʱ�����
//    Uint16 uPn207;//207.0ѡ��λ��ָ���˲���
//    Uint16 uPn208;//ƽ���ƶ��˲���ʱ��
    Uint16 uCLR;//�����źŽ����ⲿCLR�źţ�0-�ߵ�ƽ��1-�����أ�2-�͵�ƽ��3-�½���
    Uint16 uSVON;//�����ź��ŷ�ON״̬
    Uint16 uErrStat;//�����źŹ���״̬
    Uint16 uPower;//�����ź�����Դ״̬
//    Uint16 iPn500;//��λ����ź�
//    Uint16 iPn504;//��λ�����ź�
    int16  iPosCount_Err;//��ǰλ�ü���������ֵ���з��ţ���ʾ����
    int16  iPosCount_Err_Old;
//    Uint16 iErr_Flag;    //��ʾ����ֵ��������1��ʾ����0��ʾ��
    int32  lFeedBack_Err;//���뷴���������
    int32  lErrCount;//ƫ�������
    int16  iProp_Out;//����ͨ�����
    int32  lProp_Out;//����ͨ�����֮ǰ�����ڼ���
    Uint16 uClrStat;//���״̬,���Ϊ�������λ��ƫ�����������ʹλ�û���Ч
	int16  iLimit;//
    Uint16 u_COIN;//��λ�������źţ��͵�ƽ_��λ��ɣ��ߵ�ƽ_��λδ��ɡ�
//    Uint16 u_NEAR;//��λ��������źţ��͵�ƽ_���︽�����ߵ�ƽ_δ��������
    Uint16 uPos_Flg;//01:λ�ó��02:���ֱ����ô���04:��λ��ɡ�
    T_Err_Count      tErrCounter;//ƫ�������
}T_POS_PROP;

#define tPosPro_default {1,1,1,0,0,0,1,0,0,0,0,0,0,0,30000,0,0,\
                        tErrCounter_default}

////////////////////////////////////////////////////////////////////
#define DivTp 600 //λ�û�ǰ��������λ�û�Ϊ5���������ڣ�ǰ��Ϊ4����ʱ��10khz Q0,(10000*60/4000)=600,�����ת/��
#define DivTpQ15   8192 //DivTp*32767/2400=8192,�ٶ� ��׼Ϊ 2400rpm
#define Over_Band	20000L
extern T_POS_REG     tPosReg_x;
extern T_POS_FORWARD tPosFor_x;    //λ�û�ǰ��
extern T_POS_PROP    tPosPro_x;   //λ�û�����

extern T_POS_PROP    tPosPro_y;
/////////////////////////�ú�����Ӧ�ĺ�//////////////////
#define PosChoiceMACRO(p)											\
{																	\
/*��������ƫ��*/														\
     p.lFeedBack_Err = p.lFeedBack - p.lFeedBack_Old;				\
/*QEP�趨���ֵΪ4000*/											     	\
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

/////////////////////////�ú�����Ӧ�ĺ�//////////////////
#define VolRefMACRO(p)									\
{														\
    p.lSpd_Ref = p.lProp_In ; /*Q24  +(p.lFor_In)*/		\
/*�ٶȸ����޷�*/											\
    if(p.lSpd_Ref > ((int32)p.iVol_Lim)<<9)				\
    {													\
        p.lSpd_Ref = ((int32)p.iVol_Lim)<<9;			\
    }													\
    if(p.lSpd_Ref < -((int32)p.iVol_Lim)<<9)			\
    {													\
        p.lSpd_Ref = -((int32)p.iVol_Lim)<<9;			\
    }													\
}
/////////////////////////�ú�����Ӧ�ĺ�//////////////////////////////////
#define DifferMACRO(p)													\
{																		\
    p.lDiff_Out = (p.lDiff_In * DivTpQ15);/*΢�ֵ��ٶ�ֵ��Q15��ʽ*/		    \
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
void EGear_Init(T_POS_FORWARD *p)// 0.01 <= ���ӳ��ֱ� <= 100
{
    int32 temp;
    temp = 0x40000000L / p->tEGear.uDen;//Q30
    p->tEGear.uResult = ((int64)p->tEGear.uNum * temp)>>6;//Q24
//�жϷ�Χ���ܳ���0.01~100
    if((p->tEGear.uResult <= 0x28F5cL) || (p->tEGear.uResult >= 0x64000000L))
    {
		p->tEGear.EGear_Err = 1;
		p->tEGear.uNum = 1;
		p->tEGear.uDen = 1;
//    	asm("   ESTOP0");//��ʾ����
//		error();
    }
}// End of function
/////////////////////////�ú�����Ӧ�ĺ�/////////////////////////////////////////////////
#define EGearMACRO(p)/* 0.01 <= ���ӳ��ֱ� <= 100*/										\
{																					\
    p.lEGear_Out = ((int64)p.uResult * p.lEGear_In )>>24;/*Q0*/						\
}

//ǰ���˲�����ʼ��
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
/////////////////////////�ú�����Ӧ�ĺ�/////////////////////////////////////////////////
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
/*���ӳ���*/																					\
    p.tEGear.lEGear_In = p.iPulCount_Err;													\
	EGearMACRO(p.tEGear)																	\
/*΢�ּ���*/																					\
    if(p.uClrStat == 0)/*��ʾ�����*/														    \
    {																						\
	    p.tDiffer.lDiff_In = p.tEGear.lEGear_Out;											\
    }																						\
    if(p.uClrStat == 1)/*��ʾ���*/															\
    {																						\
        p.tDiffer.lDiff_In = 0;																\
	}																						\
	DifferMACRO(p.tDiffer)																	\
	/*ǰ������*/																				\
    temp_pos= ((int32)p.uKP_fwd * 328);/*Q15,��λ�ǰٷֱ�32767/100=328*/						  \
    p.lGain_Out = (int16)((p.tDiffer.lDiff_Out * temp_pos)>>6);/*ǰ��ϵ��,�õ�(Q15*Q15)>>6=Q24*/  \
    /*ǰ���˲�������*/																		    \
    p.tForFilter.lInPut = (p.lGain_Out);/*Q24*/												\
	ForwardFilterMACRO(p.tForFilter);														\
    p.lFor_Out = p.tForFilter.lOutPut;/*Q24*/												\
}
/////////////////////////�ú�����Ӧ�ĺ�//////////////////////////////
volatile Uint16 Pos_offset_Ena=0; //λ��ƫ���ʹ��
volatile int16  Pos_offset_Set=0; //λ��ƫ���ֵ����λ��������������)
//λ������ƫ�����
#define ErrCountMACRO(p)										\
{																\
/*64λ�������������˷�ĸ,ȫ���ŵ���32λ,Q32*/					        \
    p.lFeed_Den = (((int64)p.lFeedBack_Err)<<32) * p.uDen;		\
/*64λ,ָ���������˷���,Q32,64λ��С���ۼ�,ע�����*/			            \
    p.lPosCount_Num = (p.lPosCount_Err<<11) * p.uNum;/*Q32*/	\
/*ƫ�����������ۼ�*/											    \
    if(p.uClrStat == 0)											\
    {															\
        /*******����λ��ƫ������ܣ����磺���XY�Ļ�е�ز��)********/    \
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
/*ȡ�ۻ�������*/												    \
     p.lErrCount_Out = p.lErrCount_Int/ p.uDen ;                \
}                                                               \

//λ�û�����
#define PosPropMacro0(p)                                                                       \
{                                                                                              \
/*ƫ�����������*/                                                                                 \
     p.tErrCounter.uDen = p.uEden;                                                             \
     p.tErrCounter.uNum = p.uEnum;                                                             \
     p.tErrCounter.lFeedBack_Err = p.lFeedBack_Err;/*Q0,32λ*/                                  \
     p.tErrCounter.lPosCount_Err = ((int32)p.iPosCount_Err)<<21;/*Q0->Q21*/                    \
     p.tErrCounter.uClrStat = p.uClrStat;/*�����־*/                                             \
}

#define PosPropMacro1(p)																		\
{																								\
   ErrCountMACRO(p.tErrCounter)																    \
}

#define PosPropMacro2(p)                                                                        \
{                                                                                               \
     /*FAL����������*/                                                                             \
     p.lProp_Out = (int32)((int64)FAL(p.tErrCounter.lErrCount_Out) * (p.uKp_Pos)>>6);/*Q24*/    \
     /*�޷�*/                                                                                    \
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


//Ϊ����ÿ������ʱ�䣬��λ�û���3�μ���
#define POS_PD_X1Macro()												\
{																		\
/*ƫ������������ж�,�ŷ�OFF������ԴOFF��������*/								    \
    if(tPosPro_x.uCLR == 1)                                             \
    {																	\
        tPosPro_x.uClrStat = 1;/*����*/									\
    }																	\
    else																\
    {																	\
  	    tPosPro_x.uClrStat = 0;/*������*/									\
	}																	\
	PosChoiceMACRO(tPosReg_x)											\
	tPosPro_x.lFeedBack_Err = tPosReg_x.lFeedBack_Err;					\
	PosPropMacro0(tPosPro_x) /*���������������ʼ��*/                        \
}


//Ϊ����ÿ������ʱ�䣬��λ�û���3�μ���
#define POS_PD_Y1Macro()												\
{																		\
/*ƫ������������ж�,�ŷ�OFF������ԴOFF��������*/								    \
    if(tPosPro_y.uCLR == 1)                                             \
    {																	\
        tPosPro_y.uClrStat = 1;/*����*/									\
    }																	\
    else																\
    {																	\
  	    tPosPro_y.uClrStat = 0;/*������*/									\
	}																	\
	PosChoiceMACRO(tPosReg_y)											\
	tPosPro_y.lFeedBack_Err = tPosReg_y.lFeedBack_Err;					\
	PosPropMacro0(tPosPro_y) /*���������������ʼ��*/                        \
}

#define POS_PD_X2Macro()												\
PosPropMacro1(tPosPro_x) /*����������(������3us)*/

#define POS_PD_Y2Macro()												\
PosPropMacro1(tPosPro_y) /*����������(������3us)*/

#define POS_PD_X3Macro()                                                \
PosPropMacro2(tPosPro_x)  /*λ�û�FAL����*/                                \
tPosPro_x.iPosCount_Err = 0;/*����*/                                     \
tPosReg_x.lSpd_Ref = tPosPro_x.lProp_Out;/*Q24*/

#define POS_PD_Y3Macro()                                                \
PosPropMacro2(tPosPro_y)  /*λ�û�FAL����*/                                \
tPosPro_y.iPosCount_Err = 0;/*����*/                                     \
tPosReg_y.lSpd_Ref = tPosPro_y.lProp_Out;/*Q24*/

/**************************************************************************************
 �������ܣ������Ժ���fal
��ڲ���: λ��ƫ�� int32 e
���ز�����  fal��������� iq
 **************************************************************************************/
//�����Ժ���fal
#define  EDIVQ24        1678    //Q24,1/30000->559,1/100000->168, 1/20000->839, 1/10000->1678
#define  alpha  8388608//Q24,0.5
const Uint32    delta[5]={1677722,503316,167772,50331,16777};   //Q24..0.001->16777;0.003->50331;0.01->167772;0.03->503316;0.1->1677722
const Uint16    K1[5]={3,6,10,18,32};   //Q0..0.001->32;0.003->18;0.01->10;0.03->6;0.1->3

#pragma CODE_SECTION(FAL, "ramfuncs");
inline _iq FAL(register int32 e)
{
	 register _iq Fal_Out,Fal_Tmp;
	    register int Sgn;
	    static   int i=5; //Transfer_Para.Reserve3;(�ɸ���ʵ��ѡ��i=1~5��

	    Fal_Tmp = e * EDIVQ24;//���б��ۻ�

	    if(i == 0)
	    {
	        return(e<<11);//��ʹ�÷����Կ��ƣ����ǵ�Ҫ����5λ��������Q5��ʽ���棬������6λ��ת����Q24λ�ٶ�ָ�
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

