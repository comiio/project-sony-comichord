/************************************************************************************
							�������ṩ�����µ��̣�
								Ilovemcu.taobao.com
							ʵ�������Χ��չģ����������ϵ���
							���ߣ����زر���							
*************************************************************************************/
#include "stm32f10x.h"
#include "delay.h"
#include "FSR.h"
#include "usart.h"

u8 state = 0;

int main(void)
{		
	delay_init();	
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 //���ڳ�ʼ��Ϊ9600
	FSR_IO_Init();

	delay_ms(1000);

	printf("Test start\r\n");
	while(1)
	{
		if(FSR_Scan(1) == 1 && state == 0)
		{
				printf("���ڷ�ֵ\r\n");
				state = 1;
		}
		
		if(FSR_Scan(1) == 0 && state == 1)
		{
				printf("���ڷ�ֵ\r\n");
				state = 0;
		}
			
			
			
		delay_ms(500);
	}

}

