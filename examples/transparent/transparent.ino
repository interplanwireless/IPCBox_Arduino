/*
 * transparent
 * IM920sLコンソール入力をパケット送信 / 受信パケットをコンソール出力
 * D4～GND間にトリガ用押しボタンSWを接続
 * IMBLE2も同手順で送信可(node=0のみ対応)
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
int i;
unsigned short n;
String s, c;
char node[6];

void setup() {
  pinMode(D4, INPUT_PULLUP);
  Serial.begin(19200);
  Serial.setTimeout(0xFFFFFFFF);      // [ms] この時間以内にコマンドを入力
  ib.init();

  delay(100);                         // IM920sL起動待ち
  while (ib.get_imbusy());            // BUSY解除待ち
                                      // IM920sLパラメータ設定 (IMBLEは不要)
  while (!ib.put_line("STRT1\r\n"));    // 通信モード:高速
  while (!ib.put_line("STCH01\r\n"));   // 通信ch:01
  while (!ib.put_line("STPO2\r\n"));    // 送信電力:10mW
  while (!ib.put_line("STTR03\r\n"));   // キャリアセンスリトライ:3回
  while (!ib.put_line("RDNN\r\n"));     // 確認のためノード番号を出力
}

void loop() {
  if (!digitalRead(D4)) {       // ボタンが押された場合は・・・
    Serial.print("node >");       // キー入力待ち / 宛先ノード番号hex
    Serial.readBytesUntil('\n', node, 6);
    n = strtol(node, 0, 16);
    Serial.print("data >");       // キー入力待ち / ペイロード
    s = Serial.readStringUntil('\n');
    s.remove(s.length()-1);
    c = ib.encode_pkt(n, s);      // 送信用コマンドを生成
    ib.put_line(c);               // IM920sLへコマンド投入
    
    while (!digitalRead(D4));     // チャタリングキャンセル処理
    delay(10);
  }

  s = ib.get_line();            // IM920sLからの受信処理
  if (s.length()) {             // データがあった場合は・・・
    ib.decode_pkt(s);             // パケット解析して表示
    if (ib.rfdata) {              // 受信パケットの場合は要素ごと
      Serial.printf("node:%04X, rssi:%03d, ", ib.node, ib.rssi);
      Serial.println(ib.payload);
    } else {                      // 受信パケット以外はそのまま表示 
      Serial.print(s);
    }
  }
}
