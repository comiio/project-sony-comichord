/******************************************************************************
----------------�����̽���ѧϰʹ�ã�δ������������������������;��-----------
------------------------��Ȩ���У���ð�ؾ���-----------------------------------
----------------�Ա���ַ��Ilovemcu.taobao.com----------------------------------
    STC89C52/STC12C5A60S2��Һ���ӿڵ���Сϵͳ��
    	https://item.taobao.com/item.htm?id=26410708738
    FSR��Ĥѹ�������� Բ�Σ�
    	https://item.taobao.com/item.htm?id=564232635110
    FSR��Ĥѹ�������� �����Σ�
    	https://item.taobao.com/item.htm?id=564085737164
    �Ű��ߣ�
    	https://item.taobao.com/item.htm?id=562848773709
	LCD1602Һ����
		https://item.taobao.com/item.htm?id=21282627385
------------------���ߣ����زر���---------------------------------------------*/
#include "main.h"
#include "LCD1602.h"
unsigned int ADC_Buffer = 0;
long PRESS_AO = 0;
int VOLTAGE_AO = 0;

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

//****************************************************
//������
//****************************************************
void main()
{
	ADC_CONTR = ADC_360T | ADC_ON;
	AUXR1 |= ADRJ;									//ADRJ = 1;			//10bitAD�Ҷ���
	Init_LCD1602();									//��ʼ��LCD1602
	LCD1602_write_com(0x80);						//ָ�����õ�
	LCD1602_write_word("Welcome to use!");			//��ʾ����


	LCD1602_write_com(0x01);				//����

	while(1)
	{
		LCD1602_write_com(0x80);						//ָ�����õ���һ��
		LCD1602_write_word("Initialize OK!");			//��ʾ����

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

		LCD1602_write_com(0x80+0x40);					//ָ������
		LCD1602_write_word("F = ");			//��ʾ����
		LCD1602_write_data(PRESS_AO/10000+0x30);			//��ʾ����
		LCD1602_write_data(PRESS_AO%10000/1000+0x30);			//��ʾ����
		LCD1602_write_data(PRESS_AO%1000/100+0x30);
		LCD1602_write_data(PRESS_AO%100/10+0x30);
		LCD1602_write_data(PRESS_AO%10+0x30);
		LCD1602_write_word(" g");	
		
		if( Sensor_DO == 0 )		//���Ƚ������
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


//map����
long map(long x, long in_min, long in_max, long out_min, long out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//****************************************************
//��һ��ADCת��
//****************************************************
unsigned int adc10_start(unsigned char channel)	//channel = 0~7
{
	unsigned int	adc;
	unsigned char	i;

	P1ASF = (1 << channel);			//12C5A60AD/S2ϵ��ģ������(AD)ѡ��

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
			ADC_CONTR &= ~ADC_FLAG;				//�������ADC_FLAG
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
//MS��ʱ����(12M�����²���)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}