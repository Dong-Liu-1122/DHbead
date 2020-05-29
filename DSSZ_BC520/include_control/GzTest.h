/*
 * GzTest.h
 *
 *  Created on: 2019Äê9ÔÂ10ÈÕ
 *      Author: Lenovo
 */

#ifndef INCLUDE_CONTROL_GZTEST_H_
#define INCLUDE_CONTROL_GZTEST_H_

typedef struct {      // bit  description
    Uint16      Cmd;
    Uint16      State;
    Uint16      Flag;
    Uint16      Delay;
    Uint16      loop;
    Uint16      Enable;
}GZTEST_HANDLE ;

#define GZTEST_HANDLE_DEFAULT {0,0,0,0,0,0}

extern GZTEST_HANDLE GZ;

#endif /* INCLUDE_CONTROL_GZTEST_H_ */
