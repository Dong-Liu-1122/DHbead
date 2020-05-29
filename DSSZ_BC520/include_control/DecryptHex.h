/*
 * DecryptHex.h
 *
 *  Created on: 2018��10��9��
 *      Author: LIUDONG
 */

#ifndef DECRYPTHEX_H_
#define DECRYPTHEX_H_

#ifndef DSP28_DATA_TYPES
#define DSP28_DATA_TYPES
typedef int             int16;
typedef long            int32;
typedef unsigned int    Uint16;
typedef unsigned long   Uint32;
typedef float           float32;
typedef long double     float64;
typedef long long       int64;
#endif

extern Uint16 EncryptBlock[8];//��żӽ�������
extern void CANDataDecryptHandle(Uint16 *RecData,Uint16 Size);  //����CANָ������
extern void DecryptHandle(void);  //���ܺ���
extern Uint16 DecryptFlag;  //���ܱ�־λ
extern void DecryptOutputDataHandle(void);//�ú����ṩ���û���������ȡ���ܺ��ָ��

#endif /* DECRYPTHEX_H_ */
