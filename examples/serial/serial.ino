/*
 * serial
 * IB-DUAL外部接続用のシリアルポートを初期化する
 *
 * I2C/SPI/UART使用時は初期化順に制限あり
 * 下記の順番で設定すること
 * 1)IU920を初期化
 * 2)UART(A0/A1)を初期化
 * 3)SPI(又はUART2)(D2～5)を初期化
 * 4)I2C(又はUART3)(D0/1)を初期化,
 * ※IU920以外は使用しない物は省略可
 * ※UART2を使う場合はSPI.hをincludeしないかSPI_INTERFACES_COUNTを0
 * ※UART3を使う場合はWire.hをincludeしないかWIRE_INTERFACES_COUNTを0
 * 
 * Ver. 0.01  2022/09/01 test version
 *
 * 本ソフトウェアは無保証です。
 * 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
 * 改変・流用はご自由にどうぞ。
 * Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
*/

#include "IPCBox.h"
#include <Wire.h>
#include <SPI.h>

IPCBox ib;
Uart Serial2(&sercom1, D3, D2, SERCOM_RX_PAD_1, UART_TX_PAD_0);
Uart Serial3(&sercom2, D1, D0, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void setup() {
  Serial.begin(19200);
  delay(2000);
  ib.init();

  Serial1.begin(19200);   

  SPI.begin();            
  pinMode(CS, OUTPUT);    // CSpin (D2)
  digitalWrite(CS, HIGH);
  //Serial2.begin(19200);   
 
  ib.i2cpu_on();          // I2C pull-up
  Wire.begin();           
  //Serial3.begin(19200); 
}

void loop() {
}

#if 0	// Serial2を使用する場合
void SERCOM1_0_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM1_1_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM1_2_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM1_3_Handler()
{
  Serial2.IrqHandler();
}
#endif

#if 0	// Serial3を使用する場合
void SERCOM2_0_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM2_1_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM2_2_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM2_3_Handler()
{
  Serial3.IrqHandler();
}
#endif
