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
// 例程只是给出理论值，想要精确请自行万用表测量然后修正以下2个AO引脚电压输出的最大和最小值
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
 const SDSink::Item table[36] = {
    {60, "voice/guitar/f/twang_c4_f_rr1.wav"},   // C4
    {61, "voice/guitar/f/twang_db4_f_rr1.wav"},  // C#4
    {62, "voice/guitar/f/twang_d4_f_rr1.wav"},   // D4
    {63, "voice/guitar/f/twang_eb4_f_rr1.wav"},  // D#4
    {64, "voice/guitar/f/twang_e4_f_rr1.wav"},   // E4
    {65, "voice/guitar/f/twang_f4_f_rr1.wav"},   // F4
    {66, "voice/guitar/f/twang_gb4_f_rr1.wav"},  // F#4
    {67, "voice/guitar/f/twang_g4_f_rr1.wav"},   // G4
    {68, "voice/guitar/f/twang_ab4_f_rr1.wav"},  // G#4
    {69, "voice/guitar/f/twang_a4_f_rr1.wav"},   // A4
    {70, "voice/guitar/f/twang_bb4_f_rr1.wav"},  // A#4
    {71, "voice/guitar/f/twang_b4_f_rr1.wav"},    // B4
    {72, "voice/guitar/mf/twang_c4_mf_rr1.wav"},   // C4
    {73, "voice/guitar/mf/twang_db4_mf_rr1.wav"},  // C#4
    {74, "voice/guitar/mf/twang_d4_mf_rr1.wav"},   // D4
    {75, "voice/guitar/mf/twang_eb4_mf_rr1.wav"},  // D#4
    {76, "voice/guitar/mf/twang_e4_mf_rr1.wav"},   // E4
    {77, "voice/guitar/mf/twang_f4_mf_rr1.wav"},   // F4
    {78, "voice/guitar/mf/twang_gb4_mf_rr1.wav"},  // F#4
    {79, "voice/guitar/mf/twang_g4_mf_rr1.wav"},   // G4
    {80, "voice/guitar/mf/twang_ab4_mf_rr1.wav"},  // G#4
    {81, "voice/guitar/mf/twang_a4_mf_rr1.wav"},   // A4
    {82, "voice/guitar/mf/twang_bb4_mf_rr1.wav"},  // A#4
    {83, "voice/guitar/mf/twang_b4_mf_rr1.wav"},   // B4
    {84, "voice/guitar/p/twang_c4_p_rr1.wav"},   // C4
    {85, "voice/guitar/p/twang_db4_p_rr1.wav"},  // C#4
    {86, "voice/guitar/p/twang_d4_p_rr1.wav"},   // D4
    {87, "voice/guitar/p/twang_eb4_p_rr1.wav"},  // D#4
    {88, "voice/guitar/p/twang_e4_p_rr1.wav"},   // E4
    {89, "voice/guitar/p/twang_f4_p_rr1.wav"},   // F4
    {90, "voice/guitar/p/twang_gb4_p_rr1.wav"},  // F#4
    {91, "voice/guitar/p/twang_g4_p_rr1.wav"},   // G4
    {92, "voice/guitar/p/twang_ab4_p_rr1.wav"},  // G#4
    {93, "voice/guitar/p/twang_a4_p_rr1.wav"},   // A4
    {94, "voice/guitar/p/twang_bb4_p_rr1.wav"},  // A#4
    {95, "voice/guitar/p/twang_b4_p_rr1.wav"}  // B4
 };
// const SDSink::Item table[12] = {
//     {60, "voice/group1/group1_1.wav"}, 
//     {61, "voice/group1/group1_2.wav"}, 
//     {62, "voice/group1/group1_3.wav"}, 
//     {63, "voice/group1/group1_4.wav"}, 
//     {64, "voice/another/group/sound1.wav"}, 
//     {65, "voice/another/group/sound2.wav"}, 
//     {66, "voice/another/group/sound3.wav"}, 
//     {67, "voice/another/group/sound4.wav"}, 
//     {68, "voice/mlh/1.wav"}, 
//     {69, "voice/mlh/2.wav"}, 
//     {70, "voice/mlh/3.wav"}, 
//     {71, "voice/mlh/4.wav"}  
// };
SDSink inst(table, 36);
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

    //  change velocity dynamically (0-127) Cancel the overlapping part
  if (Y == 1 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);     //Stop the playing sound file
    note1 = 60 + selector;     //Determine the unique identifier of the sound file to be played based on the touch location and button selection
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 2 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 61 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 3 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 62 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 4 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 63 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 5 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 64 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 6 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 65 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 7 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 66 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 8 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 67 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 9 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 68 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 10 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 69 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 11 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 70 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (Y == 12 && lastState == NO_TOUCH_STATE) {
    inst.sendNoteOff(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    note1 = 71 + selector;
    Serial.println(note1); 
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }
  if (button4.hasChanged() && button4.isPressed()) {
      selector = 0;
  }

  if (button5.hasChanged() && button5.isPressed()) {
      selector = 12;
  }

  if (button6.hasChanged() && button6.isPressed()) {
      selector = 24;
  }
  // run instrument
  inst.update();
}
