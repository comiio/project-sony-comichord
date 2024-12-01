/******************************************************************************
----------------本例程仅供学习使用，未经作者允许，不得用于其他用途。-----------
------------------------版权所有，仿冒必究！-----------------------------------
----------------淘宝网址：Ilovemcu.taobao.com----------------------------------
    STC89C52/STC12C5A60S2带液晶接口的最小系统：
    	https://item.taobao.com/item.htm?id=26410708738
    FSR薄膜压力传感器 圆形：
    	https://item.taobao.com/item.htm?id=564232635110
    FSR薄膜压力传感器 长条形：
    	https://item.taobao.com/item.htm?id=564085737164
    杜邦线：
    	https://item.taobao.com/item.htm?id=562848773709
	LCD1602液晶：
		https://item.taobao.com/item.htm?id=21282627385
------------------作者：神秘藏宝室---------------------------------------------*/
#include "main.h"
#include "LCD1602.h"
unsigned int ADC_Buffer = 0;
long PRESS_AO = 0;
int VOLTAGE_AO = 0;

//下面4项内容需要根据实际型号和量程修正

//最小量程 根据具体型号对应手册获取,单位是g，这里以RP-18.3-ST型号为例，最小量程是20g
#define PRESS_MIN	20
//最大量程 根据具体型号对应手册获取,单位是g，这里以RP-18.3-ST型号为例，最大量程是6kg
#define PRESS_MAX	6000

//以下2个参数根据获取方法：
//理论上：
// 1.薄膜压力传感器不是精准的压力测试传感器，只适合粗略测量压力用，不能当压力计精确测量。
// 2. AO引脚输出的电压有效范围是0.1v到3.3v，而实际根据不同传感器范围会在这个范围内，并不一定是最大值3.3v，也可能低于3.3v，要实际万用表测量，
// 	例程只是给出理论值，想要精确请自行万用表测量然后修正以下2个AO引脚电压输出的最大和最小值
//调节方法：
//薄膜压力传感器的AO引脚输出的增益范围是通过板载AO_RES电位器调节实现的，
//想要稍微精准点，需要自己给定具体已知力，然后调节AO_RES电位器到串口输出重量正好是自己给定力就可以了
#define VOLTAGE_MIN 100
#define VOLTAGE_MAX 3300

//****************************************************
//主函数
//****************************************************
void main()
{
	ADC_CONTR = ADC_360T | ADC_ON;
	AUXR1 |= ADRJ;									//ADRJ = 1;			//10bitAD右对齐
	Init_LCD1602();									//初始化LCD1602
	LCD1602_write_com(0x80);						//指针设置到
	LCD1602_write_word("Welcome to use!");			//显示内容


	LCD1602_write_com(0x01);				//清屏

	while(1)
	{
		LCD1602_write_com(0x80);						//指针设置到第一行
		LCD1602_write_word("Initialize OK!");			//显示内容

		ADC_Buffer = adc10_start(0);		// P1.0 ADC

		VOLTAGE_AO = map(ADC_Buffer, 0, 1023, 0, 5000);

		if(VOLTAGE_AO < VOLTAGE_MIN)
		{
			PRESS_AO = 0;
		}
		else if(VOLTAGE_AO > VOLTAGE_MAX)
		{
			PRESS_AO = PRESS_MAX;
		}
		else
		{
			PRESS_AO = map(VOLTAGE_AO, VOLTAGE_MIN, VOLTAGE_MAX, PRESS_MIN, PRESS_MAX);
		}

		LCD1602_write_com(0x80+0x40);					//指针设置
		LCD1602_write_word("F = ");			//显示内容
		LCD1602_write_data(PRESS_AO/10000+0x30);			//显示内容
		LCD1602_write_data(PRESS_AO%10000/1000+0x30);			//显示内容
		LCD1602_write_data(PRESS_AO%1000/100+0x30);
		LCD1602_write_data(PRESS_AO%100/10+0x30);
		LCD1602_write_data(PRESS_AO%10+0x30);
		LCD1602_write_word(" g");	
		
		if( Sensor_DO == 0 )		//检测比较器输出
		{
			LED1 = LED_ON;
		}
		else
		{
			LED1 = LED_OFF;	
		}
		Delay_ms(100);		
	}
}


//map函数
long map(long x, long in_min, long in_max, long out_min, long out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//****************************************************
//做一次ADC转换
//****************************************************
unsigned int adc10_start(unsigned char channel)	//channel = 0~7
{
	unsigned int	adc;
	unsigned char	i;

	P1ASF = (1 << channel);			//12C5A60AD/S2系列模拟输入(AD)选择

	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = (ADC_CONTR & 0xe0) | ADC_START | channel;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	 

//	for(i=0; i<250; i++)		//13T/loop, 40*13=520T=23.5us @ 22.1184M
	i = 250;
	do{
		if(ADC_CONTR & ADC_FLAG)
		{
			ADC_CONTR &= ~ADC_FLAG;				//软件清零ADC_FLAG
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			adc = 0;
			adc = (ADC_RES << 8) | ADC_RESL;	//ADRJ_enable()


			return	adc;
		}
	}while(--i);
	return	1024;
}

//****************************************************
//MS延时函数(12M晶振下测试)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}