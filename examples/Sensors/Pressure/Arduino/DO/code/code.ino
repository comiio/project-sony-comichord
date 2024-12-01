/******************************************************************************
----------------本例程仅供学习使用，未经作者允许，不得用于其他用途。-----------
------------------------版权所有，仿冒必究！-----------------------------------
----------------1.开发环境:Arduino IDE 1.0.6版本-------------------------------
----------------2.测试使用开发板型号：Arduino UNO R3---------------------------
----------------3.单片机使用晶振：16M------------------------------------------
----------------4.淘宝网址：Ilovemcu.taobao.com--------------------------------
    ARDUINO UNO R3 开发板：
    	https://item.taobao.com/item.htm?id=551784439011
    FSR薄膜压力传感器 圆形：
    	https://item.taobao.com/item.htm?id=564232635110
    FSR薄膜压力传感器 长条形：
    	https://item.taobao.com/item.htm?id=564085737164
    杜邦线：
    	https://item.taobao.com/item.htm?id=562848773709
----------------5.作者：神秘藏宝室---------------------------------------------*/
#define SENSOR 2
int KEY_NUM = 0;
int count = 0;
int state = 0;

void setup() 
{ 
  pinMode(SENSOR,INPUT);
  Serial.begin(9600); // setup serial 
}

void loop() 
{ 
  scanSensor();
  if(KEY_NUM == 1)
  {
    KEY_NUM = 0;
  	Serial.println("press!");
  	Serial.print("count =");
  	Serial.println(count);
  	count++;

    
    if(state == 2)
      state = 0;
  }
  
}

void scanSensor()
{
  if(digitalRead(SENSOR) == LOW)
  {
    delay(10);
    if(digitalRead(SENSOR) == LOW) 
    {
      while(digitalRead(SENSOR) == LOW);
      KEY_NUM = 1;
    } 
  }
}



