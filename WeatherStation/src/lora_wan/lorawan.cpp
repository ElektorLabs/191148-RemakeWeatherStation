#include <Arduino.h>
#include <SPI.h>
#include "lorawan.h"
#include <lmic.h>
#include <hal/hal.h>



#define TX_INTERVAL ( 5 * 60  )

static uint8_t mydata[] = "Hello, world!";

// Pin mapping
lmic_pinmap lmic_pins_esp ;
static osjob_t sendjob;


// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x26, 0x0B, 0x02, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x7E, 0xAA, 0x68, 0xE6, 0x7A, 0xAE, 0x4A, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x21, 0x3A, 0xDE, 0x67, 0xE9, 0xDE, 0xB4, 0x02, 0x08, 0x6C, 0x83, 0xD5, 0x28, 0xD8, 0x50, 0xC6 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}
bool init_ok=false;
bool lorawan::begin( uint8_t NSS, uint8_t RXTX, uint8_t RST, uint8_t DIO0, uint8_t DIO1, uint8_t DIO2 ){
  /* We do the lmic init first, if we have no modem inside we will skip the lora part */
  
  sendjob = sendjob;
  lmic_pins_esp.nss = NSS;
  lmic_pins_esp.rxtx = RXTX;
  lmic_pins_esp.rst = RST;
  lmic_pins_esp.dio[0]=DIO0;
  lmic_pins_esp.dio[1]=DIO1;
  lmic_pins_esp.dio[2]=DIO2;
 

  if(0 == os_init_ex ( &lmic_pins_esp) ){
        Serial.println("No RFM95 / SX127x found, LoRaWAN will be disabled ");
        init_ok=false;
  } else {
        Serial.println("RFM95 found, enable LoRaWAN");
        //We need as parameter a pointer to this class to call functions
        // Reset the MAC state. Session and pending data transfers will be discarded.
        LMIC_reset();
        //Set for 2% Clock error 
        LMIC_setClockError(MAX_CLOCK_ERROR * 5 / 100);
        // Start job (sending automatically starts OTAA too)
        lorawan::do_send(&sendjob);
        init_ok=true;
  }
  return init_ok;
}



void lorawan::do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void lorawan::LoRaWAN_Task( void ){
    static uint32_t lasttime=millis();
    static uint32_t points = 0;
    if(init_ok==false){
        return;
    }
        os_runloop_once();
        if( 1000 < (millis()-lasttime) ){
            lasttime = millis();
            if(points<39){
                Serial.print(".");
                points++;
            } else {
                Serial.println(".");
                points=0;
            }
        }
    
}

//-----------------------------------------------------------------------------------------------
// end of class 
//-----------------------------------------------------------------------------------------------

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("artKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                Serial.print(artKey[i], HEX);
              }
              Serial.println("");
              Serial.print("nwkKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                Serial.print(nwkKey[i], HEX);
              }
              Serial.println("");
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), lorawan::do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}
