#ifndef __MAIN_H__
#define __MAIN_H__


#include "STC12C5A.h"
#include <intrins.h>

#define ADC_OFF()	ADC_CONTR = 0
#define ADC_ON		(1 << 7)
#define ADC_90T		(3 << 5)
#define ADC_180T	(2 << 5)
#define ADC_360T	(1 << 5)
#define ADC_540T	0

#define ADC_CH0		0
#define ADC_CH1		1
#define ADC_CH2		2
#define ADC_CH3		3
#define ADC_CH4		4
#define ADC_CH5		5
#define ADC_CH6		6
#define ADC_CH7		7

//LED IO设置
sbit LED1 = P2^0;
sbit LED2 = P2^1;

#define LED_ON  0
#define LED_OFF 1

sbit Sensor_DO = P1^0;

//函数或者变量声明
extern void Delay_ms(unsigned int n);
extern unsigned int adc10_start(unsigned char channel);	//channel = 0~7
extern long map(long x, long in_min, long in_max, long out_min, long out_max);


#endif