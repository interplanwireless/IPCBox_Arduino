/*
 * I2C スキャン
 * SDA = コネクタ1番ピン / SCL = 2番ピン / 内蔵プルアップ抵抗使用
 * 1秒毎にI2Cバスをスキャンし応答があったアドレスを表示します
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"
#include "Wire.h"

IPCBox ib;
bool flg;
byte adr, err;

void setup() {
  Serial.begin(19200);
  ib.init();

  ib.i2cpu_on();          			// 内臓I2Cプルアップ抵抗On
  Wire.begin();           			// I2C初期化
  Wire.setClock(100000);  			// CLK周波数:100kHz
}

void loop() {
  flg = false;
  for (adr=1; adr<127; adr++) {
    Wire.beginTransmission(adr);    // スキャン実行
    err = Wire.endTransmission();   

    if (!err) {                     // 応答があった場合
      flg = true;
      Serial.printf("%02X\r\n", adr);
    }
  }
  if (!flg) {                       // 応答が無い場合
    Serial.println("--");
  }
  delay(1000);
}
