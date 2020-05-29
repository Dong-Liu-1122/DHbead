/*
 * DecryptHex.h
 *
 *  Created on: 2018年10月9日
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

extern Uint16 EncryptBlock[8];//存放加解密数据
extern void CANDataDecryptHandle(Uint16 *RecData,Uint16 Size);  //接收CAN指令密文
extern void DecryptHandle(void);  //解密函数
extern Uint16 DecryptFlag;  //解密标志位
extern void DecryptOutputDataHandle(void);//该函数提供给用户，用于提取解密后的指令

#endif /* DECRYPTHEX_H_ */
