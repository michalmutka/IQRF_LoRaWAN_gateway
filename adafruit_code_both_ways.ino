#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//--------------------------------------- Here change your keys -------------------------------------------------

static const PROGMEM u1_t NWKSKEY[16] = { 0x19, 0x9E, 0x59, 0x6F, 0xF4, 0x59, 0xB1, 0xB6, 0xDA, 0x5B, 0xFF, 0xB3, 0xB1, 0xF1, 0x82, 0x86 };   // LoRaWAN NwkSKey, network session key, MSB
static const u1_t PROGMEM APPSKEY[16] = { 0x98, 0x8B, 0xE9, 0x3B, 0x90, 0x50, 0xBF, 0x87, 0x92, 0xF6, 0x2D, 0xE7, 0xE8, 0x1F, 0xF0, 0x42 };   // LoRaWAN AppSKey, application session key, MSB
static const u4_t DEVADDR = 0x260BDA0C;                                                                                                       // LoRaWAN end-device address (DevAddr), MSB

//---------------------------------------------------------------------------------------------------------------

#if defined(ARDUINO_SAMD_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0)      // Pin mapping for Adafruit Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,
    .spi_freq = 8000000,
};
#elif defined(ARDUINO_AVR_FEATHER32U4)                                    // Pin mapping for the Adafruit Feather 32u4 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {7, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,
    .spi_freq = 1000000,
};
#else
# error "Unknown target"
#endif

static osjob_t sendjob;    


const unsigned TX_INTERVAL = 60;          // Transmission interval in seconds
String requestStr="";


void myNewHex2(unsigned v, int i, int len) {
    if (v < 16)
        requestStr +='0';
    requestStr += String(v, HEX);
    if (i != len-1) {
    requestStr += ".";
    }
}

void onEvent (ev_t ev) {
    if(ev == EV_TXCOMPLETE) {
      Serial.println(F("LoRa TX done."));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack."));
      if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload from LoRa network."));
        requestStr = "";
        
        for (int i = LMIC.dataBeg; i < LMIC.dataBeg+LMIC.dataLen; i++) {
          myNewHex2(LMIC.frame[i], i, LMIC.dataBeg+LMIC.dataLen);
  
        }
        
        Serial.println("requestlora_" + requestStr);
        requestStr = "";

      }
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);  // Schedule next transmission
    }
}

void do_send(osjob_t* j){
    if (Serial.available() > 0) {
      String mojedatastring = Serial.readString();
      Serial.print("Found request from IQRF network sent via Serial that looks like this:");
      Serial.println(mojedatastring);
      LMIC_setTxData2(1, mojedatastring.c_str(), strlen(mojedatastring.c_str()), 0);    // Prepare upstream data transmission at the next possible time.
      Serial.println("Serial data sent from IQRF to LoRa network."); 
      Serial.flush();
      mojedatastring = "";
    }
    else {
      String mojedatastring = "0_0_0";
      LMIC_setTxData2(1, mojedatastring.c_str(), strlen(mojedatastring.c_str()), 0);    // Prepare upstream data transmission at the next possible time.
      Serial.print("No serial data found from IQRF network, sending zeros to LoRa to keep class A working."); 
      mojedatastring = "";
    }
}

void setup() {
    Serial.begin(9600);
    
    os_init();
    LMIC_reset();

    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);

    //EU868
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    LMIC_setLinkCheckMode(0);       // Disable link check validation
    LMIC.dn2Dr = DR_SF9;            // TTS uses SF9 for its RX2 window.
    LMIC_setDrTxpow(DR_SF9,14);     // Set data rate and transmit power for uplink
    LMIC_setAdrMode(0);             // Adaptive data rate disabled

    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); 

    do_send(&sendjob);     // Start sendjob
}

void loop() {
    os_runloop_once();
}