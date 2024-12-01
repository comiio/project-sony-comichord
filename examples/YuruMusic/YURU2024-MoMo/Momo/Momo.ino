#include <YuruInstrumentFilter.h>

/*
 * SPDX-License-Identifier: (Apache-2.0 OR LGPL-2.1-or-later)
 *
 * Copyright 2022 Sony Semiconductor Solutions Corporation
 */

#ifndef ARDUINO_ARCH_SPRESENSE
#error "Board selection is wrong!!"
#endif
#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <SDSink.h>

#include <Wire.h>
#include <mpr121.h>

#include "Button.h"

#include <Arduino.h>

#define DEBUGSerial Serial
int sensorPin = A0; 	//定义传感器的引脚

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

int X ;           // X-coordinate
int Y ;           // Y-coordinate
int lastState;

#define NO_TOUCH_STATE -1
#define MIDDLE_TOUCH_Y_POSITION 1
#define EDGE_TOUCH_Y_POSITION 13
#define EDGE_CHANNEL 2

int note1 = INVALID_NOTE_NUMBER;
int note2 = INVALID_NOTE_NUMBER;
int note3 = INVALID_NOTE_NUMBER;
int note4 = INVALID_NOTE_NUMBER;
int selector = 0;

int volume = 0;

// this file names are deifned middle C (60) as C4
// const SDSink::Item table[12] = {
//     {60, "SawLpf/60_C4.wav"},   // C4
//     {61, "SawLpf/61_C#4.wav"},  // C#4
//     {62, "SawLpf/62_D4.wav"},   // D4
//     {63, "SawLpf/63_D#4.wav"},  // D#4
//     {64, "SawLpf/64_E4.wav"},   // E4
//     {65, "SawLpf/65_F4.wav"},   // F4
//     {66, "SawLpf/66_F#4.wav"},  // F#4
//     {67, "SawLpf/67_G4.wav"},   // G4
//     {68, "SawLpf/68_G#4.wav"},  // G#4
//     {69, "SawLpf/69_A4.wav"},   // A4
//     {70, "SawLpf/70_A#4.wav"},  // A#4
//     {71, "SawLpf/71_B4.wav"}    // B4
// };
const SDSink::Item table[12] = {
    {60, "voice/group1/group1_1.wav"}, 
    {61, "voice/group1/group1_2.wav"}, 
    {62, "voice/group1/group1_3.wav"}, 
    {63, "voice/group1/group1_4.wav"}, 
    {64, "voice/another/group/sound1.wav"}, 
    {65, "voice/another/group/sound2.wav"}, 
    {66, "voice/another/group/sound3.wav"}, 
    {67, "voice/another/group/sound4.wav"}, 
    {68, "voice/mlh/1.wav"}, 
    {69, "voice/mlh/2.wav"}, 
    {70, "voice/mlh/3.wav"}, 
    {71, "voice/mlh/4.wav"}  
};
SDSink inst(table, 12);
Button button4(PIN_D04);
Button button5(PIN_D05);
Button button6(PIN_D06);

void setup() {
    // init built-in I/O
    Serial.begin(115200);
    Wire.begin();
    CapaTouch.begin();
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    // setup instrument
    if (!inst.begin()) {
        Serial.println("ERROR: init error.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("Ready to START");
}


void loop() {
  //Get position data from the position sensor(mpr121)
  lastState=Y;
  X=CapaTouch.getX();              // Get X position.
  Y=CapaTouch.getY();              // Get Y position.
  if(X>=1&&X<=9&&Y>=1&&Y<=13)      // Determine whether in the range.If not,do nothing.
   {                                 
      Serial.print("X=");
      Serial.print(X);
      Serial.print("  Y=");
      Serial.println(Y); 
   }

    //  change velocity dynamically (0-127) Cancel the overlapping part
  if (Y == MIDDLE_TOUCH_Y_POSITION && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);     //Stop the playing sound file
    note1 = 60 + (selector * 1);     //Determine the unique identifier of the sound file to be played based on the touch location and button selection
    long Fdata = getPressValue(sensorPin);       //Determine the intensity of the sound effect based on the strength of the tap
    DEBUGSerial.print("F = ");
    DEBUGSerial.print(Fdata);
    DEBUGSerial.println(" g,");
    if(Fdata >= 5000){
      volume = 120;
    }else if(Fdata >= 3000){
      volume = 100;
    }else{
      volume = DEFAULT_VELOCITY;
    }
    //volume = DEFAULT_VELOCITY + (Fdata * 63 / 6000 );
    Serial.print("  volume=");
    Serial.println(volume);
    inst.sendNoteOn(note1, volume, DEFAULT_CHANNEL);
      
  }
  if (Y == 5 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 61 + (selector * 1);
    inst.sendNoteOn(note1, 120, 2);
  }
  if (Y == 11 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 62 + (selector * 1);
    inst.sendNoteOn(note1, 120, 3);
  }
  if (Y == EDGE_TOUCH_Y_POSITION && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 63 + (selector * 1);
    inst.sendNoteOn(note1, 120, 4);
  }

  //Switch tones by button
  if (button4.hasChanged() && button4.isPressed()) {
      selector = 0;
  }

  if (button5.hasChanged() && button5.isPressed()) {
      selector = 4;
  }

  if (button6.hasChanged() && button6.isPressed()) {
      selector = 8;
  }
  // run instrument
  inst.update();
}

long getPressValue(int pin)
{
	long PRESS_AO = 0;
	int VOLTAGE_AO = 0;
	int value = analogRead(pin);

	DEBUGSerial.print("AD = ");
	DEBUGSerial.print(value);
	DEBUGSerial.print(" ,");

	VOLTAGE_AO = map(value, 0, 1023, 0, 5000);

	DEBUGSerial.print("V = ");
	DEBUGSerial.print(VOLTAGE_AO);
	DEBUGSerial.print(" mv,");

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
	

	return PRESS_AO;
}
