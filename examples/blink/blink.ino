/*
 * blink
 * LEDを1秒周期で点滅する
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"

IPCBox ib;

void setup() {
  ib.init();
}

void loop() {
  ib.ledr_on();
  ib.ledg_off();
  delay(500);
  ib.ledr_off();
  ib.ledg_on();
  delay(500);
}
