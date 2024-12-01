#include "HardwareSerial.h"
#ifndef YINFU_H_
#define YINFU_H_

#include <Arduino.h>

String readDataStringFromSerial() {
  String dataString = "";
  char receivedChar;
  unsigned long startTime = millis();  // 记录函数开始时间

  while (millis() - startTime < 1000) {  // 等待最多1秒钟
    if (Serial2.available()) {
      receivedChar = Serial2.read();
      if (receivedChar == '\n') {  // 遇到换行符，停止读取
        break;
      }
      dataString += receivedChar;
    }
  }
  return dataString;
}


void parseDataString(String dataString, int vars[]) {
  int index = 0;
  int valueIndex = 0;
  String valueString = "";

  // 跳过第一个数据项
  int commaIndex = dataString.indexOf(',');
  if (commaIndex != -1) {
    dataString = dataString.substring(commaIndex + 1);
  }

  while (index < dataString.length()) {
    char currentChar = dataString[index];
    if (currentChar == ',') {
      // 解析逗号分隔的整数值
      vars[valueIndex++] = valueString.toInt();
      valueString = "";
    } else if (currentChar != ' ' && currentChar != '\r') {
      valueString += currentChar;
    }
    index++;
  }
  // 解析最后一个整数值
  if (valueString.length() > 0) {
    vars[valueIndex] = valueString.toInt();
  }
}

int calculate(int x, int y) {
  // 圆心坐标
  int cx = (48 + 240) / 2;
  int cy = (14 + 174) / 2;

  // 圆半径
  int radius = 32;

  // 计算点到圆心的距离
  double distance = sqrt(pow(x - cx, 2) + pow(y - cy, 2));

  // 如果距离小于等于圆半径，则在圆内，返回0
  if (distance <= radius) {
    return 0;
  } else {
    // 计算点相对于圆心的角度（弧度）
    double angle = atan2(y - cy, x - cx);
    // 将弧度转换为角度
    double degrees = angle * 180 / M_PI;

    if (degrees < 0) {
      degrees += 360;
    }

    // 根据角度返回相应的象限
    if (degrees >= 0 && degrees < 22.5) {
      return 1;//do
    } else if (degrees >= 22.5 && degrees < 67.5) {
      return 3;//re
    } else if (degrees >= 67.5 && degrees < 112.5) {
      return 5;//mi
    } else if (degrees >= 112.5 && degrees < 157.5) {
      return 6;//fa
    } else if (degrees >= 157.5 && degrees < 202.5) {
      return 8;//sol
    } else if (degrees >= 202.5 && degrees < 247.5) {
      return 10;//ra
    } else if (degrees >= 247.5 && degrees < 292.5) {
      return 12;//ti
    } else if (degrees >= 292.5 && degrees < 337.5) {
      return 0;//not found
    } else {
      return 1;
    }
  }
}

int vars[5];

class yinfu {
public:
  yinfu()
    : prev_stat_(NULL), now_stat_(NULL) {}

  ~yinfu() {}

  bool update(HardwareSerial &aa) {

    if (aa.available()) {
      String dataString = readDataStringFromSerial();

      Serial.print("Received data: ");
      Serial.println(dataString);
      parseDataString(dataString, vars);

      int x = vars[1];
      int y = vars[2];
      Serial.print("x: ");
      Serial.println(x);
      Serial.print("y: ");
      Serial.println(y);
      now_stat_ = calculate(x, y);

      return true;
    } else {
      // Serial.println("no");
      delayMicroseconds(100);
      return false;
    }
  }

  bool haschanged() {
    bool signal=prev_stat_ != now_stat_;
    prev_stat_ = now_stat_;
    return (signal);
  }

public:
  int prev_stat_;
  int now_stat_;
};

#endif
