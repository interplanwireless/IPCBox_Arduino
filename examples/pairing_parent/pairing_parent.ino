/*
 * pairing_parent
 * グループ番号登録(親機：ノード番号 = 0001h)
 * IMBLE2は非対応(不要)
 * A3~GND間に押しボタンSWを接続
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

void setup() {
  pinMode(A3, INPUT_PULLUP);
  ib.init();                
  while (ib.get_imbusy());              // BUSY解除までループ
  while (!ib.put_line("ENWR\r\n"));     // Flash書き込み許可
  while (!ib.put_line("STNN0001\r\n")); // ノード番号 = 0001h
  while (!ib.put_line("STGN\r\n"));     // グループ番号登録開始
  
  while (true) {                        // 電源Offまで登録待機状態
    ib.ledg_on();
    delay(100);
    ib.ledg_off();
    delay(400);
    if (!digitalRead(A3)) { // スイッチON
      ib.imrst_on();          // RESET
      delay(100);             // 100ms wait
      ib.imrst_off();
      break;
    }
  }
}

void loop() {
}
