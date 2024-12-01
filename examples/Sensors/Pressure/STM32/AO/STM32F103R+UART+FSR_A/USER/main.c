/************************************************************************************
							本例程提供自以下店铺：
								Ilovemcu.taobao.com
							实验相关外围扩展模块均来自以上店铺
							作者：神秘藏宝室							
*************************************************************************************/
#include "stm32f10x.h"
#include "delay.h"
#include "FSR.h"
#include "usart.h"
#include "adc.h"

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

u8 state = 0;
u16 val = 0;
u16 value_AD = 0;

long PRESS_AO = 0;
int VOLTAGE_AO = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

int main(void)
{		
	delay_init();	
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	Adc_Init();

	delay_ms(1000);

	printf("Test start\r\n");
	while(1)
	{
		value_AD = Get_Adc_Average(1,10);	//10次平均值
		VOLTAGE_AO = map(value_AD, 0, 4095, 0, 3300);
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
		printf("AD值 = %d,电压 = %d mv,压力 = %ld g\r\n",value_AD,VOLTAGE_AO,PRESS_AO);	
						
		delay_ms(500);
	}

}


long map(long x, long in_min, long in_max, long out_min, long out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

