#pragma once

#include <Arduino.h>
#include <wiring_private.h>

#define def_NOT_RFDT      0xFFFF  // RF受信ではないときのrxnodeの値 


#define def_IB_IMTXD   IM_TXD  // CPU -> IM920sL
#define def_IB_IMRXD   IM_RXD  // CPU <- IM920sL
#define def_IB_IMBUSY  IM_BUSY
#define def_IB_IMRST   IM_RST

#define def_IB_BLETXD  BLE_TXD // CPU -> IMBLE2
#define def_IB_BLERXD  BLE_RXD // CPU <- IMBLE2
#define def_IB_BLESCL  BLE_SCL // CPU -> IMBLE2
#define def_IB_BLESDA  BLE_SDA // CPU <-> IMBLE2
#define def_IB_BLEBUSY BLE_BUSY
#define def_IB_BLERST  BLE_RST
#define def_IB_BLEIRQ  BLE_IRQ
#define def_IMADR      0x30    // I2Cアドレス

#define def_IB_LEDG  LEDG
#define def_IB_LEDR  LEDR
#define def_IB_DSW0  DSW0
#define def_IB_DSW1  DSW1
#define def_IB_DSW2  DSW2
#define def_IB_DSW3  DSW3
#define def_IB_I2CPU I2C_PUEN

class IPCBox 
{
  public:
  int rfdata;
    byte rssi;
    String payload; 
    unsigned short node,mi,rt[6];
    char payload_chr[256];
    
    void init(void);
    
    String encode_pkt(unsigned short, String&);
    void decode_pkt(String&);

    void put_char(char);
    int put_line(const char*);
    int put_line(String&);
    String get_line(void);
    int kbhit(void);
    void im_sleep(void);
    void im_wakeup(void);
    int get_imbusy(void);
    void imrst_on(void);
    void imrst_off(void);

    void put_charb(char);
    int put_lineb(String&);
    int put_lineb(const char*);
    String get_lineb(void);
    int kbhitb(void);
    void ble_sleep(void);
    void ble_wakeup(void);
    int get_blebusy(void);
    void blerst_on(void);
    void blerst_off(void);

    void ledg_on(void);
    void ledg_off(void);
    void ledr_on(void);
    void ledr_off(void);
    void i2cpu_on(void);
    void i2cpu_off(void);
    byte get_dsw(void);

    void start_uart(void);
    void stop_uart(void);
    void start_uartb(void);
    void stop_uartb(void);
    void get_endisp(String&);
    int chk_rfdata(String&);

  private:
    int _comrdy, _comrdyb, _i2crdyb;
    byte _adr;
    String _rxbuf, _rxbufb;
};
