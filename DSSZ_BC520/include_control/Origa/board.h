#ifndef _BOARD_H_
#define _BOARD_H_

void board_init(unsigned char dev);

void timing_init(void);

unsigned char get_pin();

void set_pin(unsigned char level);

void set_pin_dir(unsigned char dir);

void test_output(unsigned char testnum, unsigned char res, unsigned char * str);

void test_pass(void);

void test_fail(void);

void udelay(volatile unsigned long ul_Loops);

void mdelay(volatile unsigned long ul_Loops);

void Delay(unsigned short delay);

//New LED Functions//
void LED_On(unsigned char); //turn on the led number

#endif
