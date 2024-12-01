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

u8 state = 0;

int main(void)
{		
	delay_init();	
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	FSR_IO_Init();

	delay_ms(1000);

	printf("Test start\r\n");
	while(1)
	{
		if(FSR_Scan(1) == 1 && state == 0)
		{
				printf("大于阀值\r\n");
				state = 1;
		}
		
		if(FSR_Scan(1) == 0 && state == 1)
		{
				printf("低于阀值\r\n");
				state = 0;
		}
			
			
			
		delay_ms(500);
	}

}

