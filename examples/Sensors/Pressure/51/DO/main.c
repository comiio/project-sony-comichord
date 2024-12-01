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



//�������
unsigned char KEY_NUM = 0;   
int count = 0;
int state = 0;
	

//****************************************************
//������
//****************************************************
void main()
{
	Init_LCD1602();
	LCD1602_write_com(0x80);
	LCD1602_write_word("welcome to use!");
	while(1)
	{
    	scanSensor();
		if(KEY_NUM == 1)
		{
			KEY_NUM = 0;
			LCD1602_write_com(0x80+0x40);
			LCD1602_write_word("count =");
			LCD1602_write_data(count%10000/1000+0x30);
			LCD1602_write_data(count%1000/100+0x30);
			LCD1602_write_data(count%100/10+0x30);
			LCD1602_write_data(count%10+0x30);

			count++;
			
			if(state == 2)
			  state = 0;
		}

	}
}

void scanSensor()
{
//	SENSOR = 1;
	if(SENSOR == 0)
	{
		Delay_ms(10);
		if(SENSOR == 0) 
		{
		  while(SENSOR == 0);
		  KEY_NUM = 1;
		} 
	}
}


//****************************************************
//MS��ʱ����
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}

