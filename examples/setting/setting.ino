/*
 * setting
 * コンソール <-> IM920sL/IMBLE2
 * モジュールのパラメータを設定する
 * 
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"

#define def_SET_MODULE  1 // 0=IM920sL, 1=IMBLE2

IPCBox ib;
String cmd="";
String s;

void setup() {
  Serial.begin(19200);
  Serial.setTimeout(0);

  ib.init();
  delay(100);
#if def_SET_MODULE == 0			// 使わないモジュールをSLEEP
  while (ib.get_blebusy());
  ib.ble_sleep();     			
#else
  while (ib.get_imbusy());
  ib.im_sleep();
#endif
}

void loop() {
  if (Serial.available()) {
    cmd += Serial.readStringUntil('\n');    
    if (cmd.endsWith("\r")) {   // 改行コードを検出したら
#if def_SET_MODULE == 0
      ib.put_line(cmd);           // モジュールにコマンド送信
#else
      ib.put_lineb(cmd);
#endif
      cmd = "";
    }
  }

#if def_SET_MODULE == 0
  s = ib.get_line();
#else
  s = ib.get_lineb();
#endif
  if (s.length()) {
    Serial.print(s);
  }
}
