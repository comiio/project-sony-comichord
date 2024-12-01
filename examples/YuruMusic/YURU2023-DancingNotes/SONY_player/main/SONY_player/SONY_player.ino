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
#include <MP.h>
#include "yinfu.h"

int subcore = 1; /* Communication with SubCore1 */

const SDSink::Item table[12] = {
  { 60, "Piano/40.wav" },   // C4
  { 61, "Piano/41.wav" },  // C#4
  { 62, "Piano/42.wav" },   // D4
  { 63, "Piano/43.wav" },  // D#4
  { 64, "Piano/44.wav" },   // E4
  { 65, "Piano/45.wav" },   // F4
  { 66, "Piano/46.wav" },  // F#4
  { 67, "Piano/47.wav" },   // G4
  { 68, "Piano/48.wav" },  // G#4
  { 69, "Piano/49.wav" },   // A4
  { 70, "Piano/50.wav" },  // A#4
  { 71, "Piano/51.wav" }    // B4
  
};
SDSink inst(table, 12);
yinfu note;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  /* Launch SubCore1 */
  if ( MP.begin(subcore) < 0) {
    printf("MP.begin error ");
  }

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
  //inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);

  Serial.println("Ready to play string");

}

int note1 = INVALID_NOTE_NUMBER;
int note2 = INVALID_NOTE_NUMBER;
int prenote = 60;

uint8_t rcvid;
uint8_t rcvdata;


void loop() {

  note.update(Serial2);//更新音符

  if (note.haschanged()&&note.now_stat_!=0) {
    Serial.println(note.now_stat_);
    note1 = 59 + note.now_stat_;
    if(prenote != note1){
      inst.sendNoteOff(prenote, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    }
    prenote = note1;
    inst.sendNoteOn(note1, DEFAULT_VELOCITY, DEFAULT_CHANNEL);

    Serial.println("YES");
  }//判断标记的位置，改变之后则发出对应声音
  else if (note.now_stat_==0)
  {
    //inst.sendNoteOff(prenote, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }

  MP.RecvTimeout(10);

  if (MP.Recv(&rcvid, &rcvdata, subcore) < 0) {
    printf("MP.Recv error");
  }

  get_stamp(rcvdata);

  inst.update();
}


void get_stamp(uint8_t stamp_index){
  static bool flag = false;
  static int pre_drum;
  static int times = 0;
  int drum;
  switch(stamp_index){
    case(0): flag = false; break;
    case(1):drum = 1; flag = true; break;
    case(2):drum = 1; flag = true; break;
    case(3):drum = 1; flag = true; break;
  }

  if(!flag && times >= 20){
      inst.sendNoteOff(pre_drum, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
      times = 0;
      flag = false;
  }else{
    inst.sendNoteOn(drum, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
  }

  pre_drum = drum;
  
}
