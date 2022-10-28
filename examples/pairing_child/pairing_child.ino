/*
 * pairing_child
 * グループ番号登録(子機：ノード番号 ≠ 0001h)
 * IMBLE2は非対応(不要)
 * D4~GND間に押しボタンSWを接続
 *
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"

const String SET_NODE = "1234";
IPCBox ib;
String s;

void setup() {
  bool flg = true;
  pinMode(D4, INPUT_PULLUP);
  ib.init();                
  while (ib.get_imbusy());              // BUSY解除までループ
  while (!ib.put_line("ENWR\r\n"));     // Flash書き込み許可
  while (!ib.put_line("STNN"+SET_NODE+"\r\n")); // 設定するノード番号 / 0000,0001,FFF0~FFFFは設定不可
  while (!ib.put_line("STGN\r\n"));     // グループ番号登録開始
  
  while (flg) {                         // 電源Offまで登録待機状態 / 親機の50cm以内に近づけて下さい
    ib.ledg_on();
    delay(100);
    ib.ledg_off();
    delay(100);
    ib.ledr_on();
    delay(100);
    ib.ledr_off();
    delay(700);

    s = ib.get_line();
    if (s.length()) {
      if (s == "GRNOREGD\r\n") {            // 登録完了したらLEDを連続点灯に / 完了後はIM920sLを再起動して下さい
        ib.ledg_on();
        while (flg) {
          if (!digitalRead(D4)) {             //ボタンが押された場合はIM920sLを再起動
            ib.imrst_on();                      // RESET
            delay(10);                          // 10ms wait
            ib.imrst_off();                     // RESET解除
            ib.ledg_off();
            flg = false;                        // loop()へ
          }
        }
      }
    }
  }
}

void loop() {
}
