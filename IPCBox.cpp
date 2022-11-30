//
// IB-DUAL Library for Arduino
//
// Ver. 0.01  2022/09/01  test version
//
// 本ソフトウェアは無保証です。
// 本ソフトウェアの不具合により損害が発生した場合でも補償は致しません。
// 改変・流用はご自由にどうぞ。
// Copyright (C)2022 Interplan Co., Ltd. all rights reserved.
//

#include "IPCBox.h"

//内部変数
static Uart im920Ser(&sercom4, def_IB_IMRXD, def_IB_IMTXD, SERCOM_RX_PAD_1, UART_TX_PAD_0);
static Uart imBleSer(&sercom5, def_IB_BLERXD, def_IB_BLETXD, SERCOM_RX_PAD_1, UART_TX_PAD_0); 


// 初期化
void IPCBox::init(void)
{											
                                        // IM920sL I/F
  pinMode(def_IB_IMBUSY, INPUT_PULLUP);  	// BUSY
  pinMode(def_IB_IMRST, OUTPUT);         	// RESET / 最初はリセット状態
  imrst_on();

  delay(100);                               // 100ms間リセット保持
  imrst_off();                              // リセット解除
  _comrdy = false;                  
  start_uart();                             // UART初期化

  pinMode(def_IB_BLEBUSY, INPUT_PULLUP); 	// BUSY
  pinMode(def_IB_BLERST, OUTPUT);        	// RESET / 最初はリセット状態
  blerst_on();

  delay(100);                               // 100ms間リセット保持
  blerst_off();                             // リセット解除
  _comrdyb = false;
  _i2crdyb = false;
  start_uartb();                            // UART初期化
      
                                        // その他内蔵I/O                              
  pinMode(def_IB_LEDR, OUTPUT);         	// LED(赤) / 最初は消灯
  ledr_off(); 
  pinMode(def_IB_LEDG, OUTPUT);          	// LED(緑) / 最初は消灯
  ledg_off();

  pinMode(def_IB_DSW0, INPUT_PULLUP);  		// DIP-SW
  pinMode(def_IB_DSW1, INPUT_PULLUP);
  pinMode(def_IB_DSW2, INPUT_PULLUP);
  pinMode(def_IB_DSW3, INPUT_PULLUP);

  pinMode(def_IB_I2CPU, OUTPUT);       		// I2C pull-up制御 / 最初はOff / 負論理
  i2cpu_off();

                                        // 変数類設定
  _rxbuf = "";
  _rxbufb = "";
}


/* パケット処理 */
// 1パケット組立
// IMシリーズ指定のノード宛にデータを送信する為のコマンドを生成する
// 宛先 = 0の場合はブロードキャスト用コマンド(TXDA) / その他はユニキャスト用コマンド(TXDU)を生成
// ※IMBLE2の場合は宛先 = 0のみ使用可
// 入力：
//  node：宛先ノード番号 / ブロードキャスト時は0を指定
//  msg：送信するデータ / 最大32byte
// 戻り値：
//  生成したコマンド行を返す
String IPCBox::encode_pkt(unsigned short node, String &msg)
{
  String res="";
  char chr[6];

  if (!node) {
    res += "TXDA" + msg + "\r\n";
  } else {
    sprintf(chr, "%04X,", node);
    res += "TXDU" + String(chr) + msg + "\r\n";
  }

  return res;
}


// 受信パケット解析
// IMシリーズの受信パケットを要素に分解する
// 入力：
//  rxmsg：UART受信データ(1行分)
// クラス内変数
//   (node:int,rssi:int,payload:str)
// node：送信元ノード番号 / 受信パケット以外は0xFFFF
// rssi：受信RSSI値 / 受信パケット以外は0x00
// payload：受信データ / 受信パケット以外は""
// mi：msgID / 受信パケット以外は0x00
// rt：受信Route / Rtが無い場合はrt[0]=0x0000
void IPCBox::decode_pkt(String &rxmsg)
{
  byte paylen, i, j;
  char ch[5], wk;

  node = def_NOT_RFDT;        // 各要素を初期化
  mi = 0x00;
  rssi = 0x00;
  rt[0] = 0x0000;
  payload = "";
  payload_chr[0] = 0x00;
  rfdata = false;
  if (chk_rfdata(rxmsg)) {    // RFデータなら各種取得
    rfdata = true;
    ch[4] = 0;

    if (rxmsg[2] == ',') {    // Miなし
      rxmsg.remove(0, 3);
    } else {                  // Miあり
      ch[0] = rxmsg.charAt(0); ch[1] = rxmsg.charAt(1);
      ch[2] = rxmsg.charAt(2); ch[3] = rxmsg.charAt(3);
      mi = strtoul(ch, NULL, 16);
      rxmsg.remove(0, 8);
    }
    ch[0] = rxmsg.charAt(0); ch[1] = rxmsg.charAt(1);
    ch[2] = rxmsg.charAt(2); ch[3] = rxmsg.charAt(3);
    node = strtoul(ch, NULL, 16);
    rxmsg.remove(0, 5);
    
    j = 0;
    paylen = rxmsg.length();
    for (i=0; i<paylen; i++) {
      if (rxmsg[i] == ':') {        // RSSIを取得
        ch[2] = 0;
        ch[0] = rxmsg.charAt(i-2); ch[1] = rxmsg.charAt(i-1);
        rssi = strtoul(ch, NULL, 16);
        rxmsg.remove(0, i+1);
        break;
      } else if (rxmsg[i] == ',') { // ルートを取得
        ch[0] = rxmsg.charAt(i-4); ch[1] = rxmsg.charAt(i-3);
        ch[2] = rxmsg.charAt(i-2); ch[3] = rxmsg.charAt(i-1);
        rt[j++] = strtoul(ch, NULL, 16);
        rt[j] = 0;
      }
    }

    j = 0;
    ch[2] = 0x00;
    paylen = rxmsg.length() - 2;
    for (i=0; i<paylen; i+=3) {   // payloadを取得
      ch[0] = rxmsg.charAt(i+0);
      ch[1] = rxmsg.charAt(i+1);
      wk = strtoul(ch, NULL, 16);
      payload += String(wk);
      payload_chr[j++] = wk;
    }        
    payload_chr[j] = 0x00;
  }
}


/* IM920sLアクセス */
// 1文字送信
// 入力データをIM920sLへ1文字送信する
// BUSYチェックしない
// 入力:
//  ch:送信する文字
//  UART受信データ有無チェック
void IPCBox::put_char(char ch){
  im920Ser.print(ch);   
}


// 1行送信
// 入力データをそのままIM920sLへUART 1行送信する
// 主にパケット送信以外のコマンド入力時に使用
// 入力:
//  buf:送信するデータ
// 戻り値:
//  BUSY = Highの場合 又は タイムアウト等でIM920sLへのコマンド入力に失敗した場合はFalseを返す
int IPCBox::put_line(String &buf)
{ 
  if (buf.charAt(0) == '?') {
    put_char('?');
    im920Ser.flush();
    delay(10);
    buf.remove('?', 1);
  }

  if (get_imbusy()) { // BUSYの場合はエラー終了
    return false;
  } else {
    im920Ser.print(buf);
  }
  return true;
}

// 1行送信
// 例：
//  iu.put_line("RDNN\r\n");
int IPCBox::put_line(const char *buf)
{
  bool key = false;
  if (buf[0] == '?') {
    key = true;
    put_char('?');
    im920Ser.flush();
    delay(10);
  }

  if (get_imbusy()) { // BUSYの場合はエラー終了
    return false;
  } else {
    if (!key) {
      im920Ser.print(buf);
    } else {
      im920Ser.print(&buf[1]);
    }
  }
  return true;
}


// 1行受信
// IM920sLのUART出力を行末(=CR+LFまで)1行受信する
// 戻り値：
//  受信結果を1個のStringとして返す / データが何も無い場合は""を返す
// ※通常は(受信パケット解析処理)を使用する
String IPCBox::get_line(void)
{
  String res;
  if (!kbhit()) {                         // 受信データが無い場合は何もせず終了 
    return "";
  } else {
    res = im920Ser.readStringUntil('\n');   // \nは戻り値に含まれないので注意
    get_endisp(res);                        // ゴミデータ出力防止
    _rxbuf += res;
    if (!_rxbuf.endsWith("\r")) {           // 行末に\rがなければ・・・
      return "";                              // 戻り値なし
    } else {                                // あれば・・・
      res = _rxbuf + "\n";                    // 受信文字列を返す
      _rxbuf = "";                            // 次に備えてバッファはクリア
      return res;
    }
  } 
}

// UART受信データ有無チェック
// 戻り値：
// 　IM920sLからの受信データが1文字以上ある場合はTrueを返す
int IPCBox::kbhit(void)
{
  return im920Ser.available() >= 1;  
}


// スリープ開始
// IM920sLをスリープ状態に移行しUARTを停止
void IPCBox::im_sleep(void)
{
  while (!put_line("DSRX\r\n"));  // スリープ開始
  im920Ser.flush();
  delay(10);                      // 送出完了待ち
  stop_uart();                    // UART停止
}


// スリープ解除
// UARTを再スタートしIM920sLをスリープ解除
void IPCBox::im_wakeup(void)
{
  start_uart();                   // UART再スタート
  put_char('?');                  // wake-upトリガ / これは一時wake-upの為続けてENRXが必要
  while (get_imbusy());           // BUSY解除(=コマンド受付可)待ち
  while (!put_line("ENRX\r\n"));  // スリープ解除
  im920Ser.flush();
}


// BUSY状態取得
// 戻り値：
// 　BUSY = High(=コマンド投入不可)の場合はTrueを返す
int IPCBox::get_imbusy(void)
{
  return digitalRead(def_IB_IMBUSY);
}


// RESET On 出力
void IPCBox::imrst_on(void)
{
  digitalWrite(def_IB_IMRST, LOW);  // 負論理
}


// RESET Off 出力
void IPCBox::imrst_off(void)
{
  digitalWrite(def_IB_IMRST, HIGH); // 負論理
}


/* IMBLE2 アクセス */
// 1文字送信(IMBLE2)
// 入力データをIMBLE2へ1文字送信する
// BUSYチェックしない
// 入力:
//  ch:送信する文字
void IPCBox::put_charb(char ch)
{
  imBleSer.write(ch);
}


// 1行送信(IMBLE2)
// 入力データをそのままIMBLE2へUART 1行送信する
// 主にパケット送信以外のコマンド入力時に使用
// 入力:
//  buf:送信するデータ
// 戻り値:
//  BUSY = Highの場合 又は タイムアウト等でIMBLE2へのコマンド入力に失敗した場合はFalseを返す
int IPCBox::put_lineb(String &buf)
{
  if (buf.charAt(0) == '?') {
    put_charb('?');
    imBleSer.flush();
    delay(10);
    buf.remove('?', 1);
  }

  if (get_blebusy()) {      // BUSYの場合はエラー終了
    return false;
  } else {
    imBleSer.print(buf);
  }
  return true;
}

// 1行送信(IMBLE2)
// 例：
//  iu.put_lineb("RDID\r\n");
int IPCBox::put_lineb(const char *buf)
{
  if (get_blebusy()) {      // BUSYの場合はエラー終了
    return false;
  } else {
    imBleSer.print(buf);
  }
  return true;
}


// 1行受信(IMBLE2)
// IMBLE2のUART出力を行末(=CR+LFまで)1行受信する
// 戻り値：
//  受信結果を1個のbytearrayとして返す / データが何も無い場合はNoneを返す
// ※通常は(受信パケット解析処理)を使用する
String IPCBox::get_lineb(void)
{
  String res;
  if (!kbhitb()) {                          // 受信データが無い場合は何もせず終了 
    return "";
  } else {
    res = imBleSer.readStringUntil('\n');   // \nは戻り値に含まれないので注意
    get_endisp(res);                        // ゴミデータ出力防止
    _rxbufb += res;
    if (!_rxbufb.endsWith("\r")) {          // 行末に\rがなければ・・・
      return "";                              // 戻り値なし
    } else {                                // あれば・・・
      res = _rxbufb + "\n";                   // 受信文字列を返す
      _rxbufb = "";                           // 次に備えてバッファはクリア
      return res;
    }
  } 
}


// UART受信データ有無チェック(IMBLE2)
// 戻り値：
//  IMBLE2からの受信データが1文字以上ある場合はTrueを返す
// ※True時も行末まで受信できていない場合がある為注意
int IPCBox::kbhitb(void)
{
  if (imBleSer.available()) {
    return true;
  }

  return false;
}


// スリープ開始(IMBLE2)
// IMBLE2をスリープ状態に移行しUARTを停止
void IPCBox::ble_sleep(void)
{
  while (!put_lineb("DSRX\r\n"));   // スリープ開始
  imBleSer.flush();
  delay(10);                        // 送出完了待ち
  stop_uartb();                     // UART停止 
}


// スリープ解除(IMBLE2)
// UARTを再スタートしIMBLE2をスリープ解除
void IPCBox::ble_wakeup(void)
{
  start_uartb();                // UART再スタート
  put_charb('?');               // wake-upトリガ / これは一時wake-upの為続けてENRXが必要
  imBleSer.flush();
  while (get_blebusy());        // BUSY解除(=コマンド受付可)待ち
  while (!put_lineb("ENRX"));   // スリープ解除
  imBleSer.flush();
}


// BUSY状態取得
// 戻り値：
// 　BUSY = High(=コマンド投入不可)の場合はTrueを返す
int IPCBox::get_blebusy(void)
{
  return digitalRead(def_IB_BLEBUSY);
}


// モジュールをリセットする 
void IPCBox::blerst_on(void)
{
  digitalWrite(def_IB_BLERST, LOW);  // 負論理
}


// リセットを解除する
void IPCBox::blerst_off(void)
{
  digitalWrite(def_IB_BLERST, HIGH); // 負論理
}


/* その他I/O制御 */
// LED(緑) 点灯
void IPCBox::ledg_on(void)
{
  digitalWrite(def_IB_LEDG, HIGH);
}


// LED(緑) 消灯
void IPCBox::ledg_off(void)
{
  digitalWrite(def_IB_LEDG, LOW);
}


// LED(赤) 点灯
void IPCBox::ledr_on(void)
{
  digitalWrite(def_IB_LEDR, HIGH);
}


// LED(赤) 消灯
void IPCBox::ledr_off(void)
{
  digitalWrite(def_IB_LEDR, LOW);
}


// I2C pull-up On
void IPCBox::i2cpu_on(void)
{
  digitalWrite(def_IB_I2CPU, LOW);  // 負論理
}


// I2C pull-up Off
void IPCBox::i2cpu_off(void)
{
  digitalWrite(def_IB_I2CPU, HIGH); // 負論理
}


// DIP-SW 状態取得
// 戻り値：
//  DIP-SWの状態を4bitの数値(0～F)として返す
byte IPCBox::get_dsw(void)
{
  byte res;
  res =  !digitalRead(def_IB_DSW3);
  res <<= 1;
  res |= !digitalRead(def_IB_DSW2);
  res <<= 1;
  res |= !digitalRead(def_IB_DSW1);
  res <<= 1;
  res |= !digitalRead(def_IB_DSW0);

  return res;
}


/* 補助関数 */
// UART初期化(IM920sL)
void IPCBox::start_uart(void)
{
  if (!_comrdy) {
    im920Ser.begin(19200);
    im920Ser.setTimeout(0);
    _comrdy = true;
  }
}


// UART停止(IM920sL)
void IPCBox::stop_uart(void)
{
  if (_comrdy) {
    im920Ser.end();
    _comrdy = false;
  }
}


// UART初期化(IMBLE2)
void IPCBox::start_uartb(void)
{
  if (!_comrdyb) {
    imBleSer.begin(19200);
    imBleSer.setTimeout(0);
    _comrdyb = true;
  }
}


// UART停止(IMBLE2)
void IPCBox::stop_uartb(void)
{
  if (_comrdyb) {
    imBleSer.end();
    _comrdyb = false;
  }
}


// 表示不可能文字の削除
void IPCBox::get_endisp(String &data)
{
  unsigned int i;
  char dt;
  for (i=0; i<data.length(); i++) {
    dt = data.charAt(i);
    if (!((dt=='\r') || (dt=='\n') || (' '<=dt&&dt<='~'))) {
      data.remove(i, 1);
      i--;
    }
  }
}


// 受信データがRFパケットか確認する
int IPCBox::chk_rfdata(String &rxmsg)
{
  int idx;

  idx = rxmsg.indexOf(':');
  if (idx != -1) {
    if (idx >= 10) {
      if ((rxmsg[2]==',' || rxmsg[4]==',') && rxmsg[idx-3]==',') {
        return true;
      }
    }
  }
  return false;
}

// SAMD51 interrput handlers
// Reference:https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
void SERCOM4_0_Handler()
{
  im920Ser.IrqHandler();
}
void SERCOM4_1_Handler()
{
  im920Ser.IrqHandler();
}
void SERCOM4_2_Handler()
{
  im920Ser.IrqHandler();
}
void SERCOM4_3_Handler()
{
  im920Ser.IrqHandler();
}

void SERCOM5_0_Handler()
{
  imBleSer.IrqHandler();
}
void SERCOM5_1_Handler()
{
  imBleSer.IrqHandler();
}
void SERCOM5_2_Handler()
{
  imBleSer.IrqHandler();
}
void SERCOM5_3_Handler()
{
  imBleSer.IrqHandler();
}