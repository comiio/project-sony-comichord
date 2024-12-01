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
#include "adc.h"

//����4��������Ҫ����ʵ���ͺź���������

//��С���� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g��������RP-18.3-ST�ͺ�Ϊ������С������20g
#define PRESS_MIN	20
//������� ���ݾ����ͺŶ�Ӧ�ֲ��ȡ,��λ��g��������RP-18.3-ST�ͺ�Ϊ�������������6kg
#define PRESS_MAX	6000

//����2���������ݻ�ȡ������
//�����ϣ�
// 1.��Ĥѹ�����������Ǿ�׼��ѹ�����Դ�������ֻ�ʺϴ��Բ���ѹ���ã����ܵ�ѹ���ƾ�ȷ������
// 2. AO��������ĵ�ѹ��Ч��Χ��0.1v��3.3v����ʵ�ʸ��ݲ�ͬ��������Χ���������Χ�ڣ�����һ�������ֵ3.3v��Ҳ���ܵ���3.3v��Ҫʵ�����ñ������
// 	����ֻ�Ǹ�������ֵ����Ҫ��ȷ���������ñ����Ȼ����������2��AO���ŵ�ѹ�����������Сֵ
//���ڷ�����
//��Ĥѹ����������AO������������淶Χ��ͨ������AO_RES��λ������ʵ�ֵģ�
//��Ҫ��΢��׼�㣬��Ҫ�Լ�����������֪����Ȼ�����AO_RES��λ����������������������Լ��������Ϳ�����
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
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 //���ڳ�ʼ��Ϊ9600
	Adc_Init();

	delay_ms(1000);

	printf("Test start\r\n");
	while(1)
	{
		value_AD = Get_Adc_Average(1,10);	//10��ƽ��ֵ
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
		printf("ADֵ = %d,��ѹ = %d mv,ѹ�� = %ld g\r\n",value_AD,VOLTAGE_AO,PRESS_AO);	
						
		delay_ms(500);
	}

}


long map(long x, long in_min, long in_max, long out_min, long out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

