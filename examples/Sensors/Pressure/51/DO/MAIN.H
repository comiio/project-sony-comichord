#ifndef __MAIN_H__
#define __MAIN_H__

#include <reg52.h>



//按键 IO设置
sbit SENSOR = P1^1;


//函数或者变量声明
extern void Delay_ms(unsigned int n);
extern void scanSensor();

extern unsigned char KEY_NUM;

#endif