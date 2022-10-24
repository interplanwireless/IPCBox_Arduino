/*
 * センサノード
 * I2C接続温度湿度センサ(SHT31/Grove Temperature&Humidity Sensor)の値を設定間隔でIM920sLによりブロードキャスト送信
 * ※Grove SHT35も同一コードで使用可能です
 * SDA = コネクタ1番ピン / SCL = 2番ピン / 内蔵プルアップ抵抗使用
 * 
 * 出力フォーマット:温度(整数部),温度(小数部),湿度(整数部),湿度(小数部) / 小数部は測定値の小数部×256を設定(128=0.5)
 * 単位:温度[℃] / 湿度[%]
 * 
 * 送信間隔は5秒+DIP-SW設定値x1秒
 * 送信間の待機中はCPU/IM920sL共にスリープ状態に移行
 * ※ここではWatchdog.sleep()を使用します
 * 
 * ライブラリの説明はこちら https://github.com/adafruit/Adafruit_SHT31
 * 　　　　　　　　　　　　 https://github.com/adafruit/Adafruit_SleepyDog
 * 
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"
#include "Adafruit_SHT31.h"
#include "Adafruit_SleepyDog.h"

IPCBox ib;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

unsigned short tsleep;  // 送信周期
unsigned short wsleep;  // スリープ待機時間
float t,h;              // 温湿度データ
int ti, tf, hi, hf;     // データの整数部、小数部
String m, c;            // 送信ペイロード、送信データ
char buf[9];            // 温湿度データ(16進文字列)

void setup() {
  byte dsw;

  ib.init();
  
  while (ib.get_imbusy());          // BUSY解除までループ / 起動待ち
  while (ib.get_blebusy());         // BUSY解除までループ
  ib.ble_sleep();
  
  ib.ledr_on();
  ib.i2cpu_on();                    // 内臓I2Cプルアップ抵抗On
  if (!sht31.begin()) {             // センサ初期化 /  未接続時はエラー
    while (true) {          
    }
  }
  ib.ledr_off();

  dsw = ib.get_dsw();               // 送信周期
  tsleep = 5000 + (dsw*1000) - 100; // LED点灯時間分引く
}

void loop() {
  t = sht31.readTemperature();    // 温度を取得
  ti = int(t);                      // 整数部
  if (ti > 255) {
    ti = 255;
  }
  tf = int((t-ti)*256);             // 小数部
 
  h = sht31.readHumidity();       // 湿度を取得
  hi = int(h);                      // 整数部
  if (hi > 255) {
    hi = 255;
  }
  hf = int((h-hi)*256);             // 小数部

  sprintf(buf, "%02X%02X%02X%02X", ti, tf, hi, hf);
  m = String(buf);
  c = ib.encode_pkt(0, m);        // 送信用コマンドを生成 / ブロードキャスト
  ib.put_line(c);                 // IM920sLへコマンドを投入

  ib.ledg_on();                   // 送信したらLEDを点灯
  delay(100);
  ib.ledg_off();

  ib.im_sleep();                  // IM920sLをスリープ状態へ
  wsleep = 0;                     // 送信周期まで待機
  while (wsleep < tsleep) {
    wsleep += Watchdog.sleep(tsleep-wsleep);
  }
  ib.im_wakeup();                 // IM920sLのスリープを解除
}
