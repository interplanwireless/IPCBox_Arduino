/*
 * peripheral
 * IB-DUALの搭載機能制御
 * 
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"

IPCBox ib;
bool sts; 
byte dsw;

void setup() {
  Serial.begin(19200);
  ib.init();
  sts = true;
}

void loop() {
  dsw = ib.get_dsw();   // DIP-SW 状態取得 (SW201)
  Serial.printf("DSW = 0x%X\r\n", dsw); // コンソールに16進数で出力する

  if (sts) {            // LED制御
    ib.ledr_on();         // 緑LED On (D200)
    ib.ledg_off();        // 赤LED Off (D201)
    Serial.println("LEDR On");
    Serial.println("LEDG Off");
  } else {              // LED制御
    ib.ledr_off();        // 緑LED Off (D200)
    ib.ledg_on();         // 赤LED On (D201)
    Serial.println("LEDR Off");
    Serial.println("LEDG On");
  }

  if (sts) {            // D0(SDA)/D1(SCL) I2C Pull-up On (4.7k)
    ib.i2cpu_on();
    Serial.println("I2C Pull-up On");
  } else {              // I2C Pull-up Off
    ib.i2cpu_off();
    Serial.println("I2C Pull-up Off");
  }

  sts = !sts;
  delay(3000);
}
