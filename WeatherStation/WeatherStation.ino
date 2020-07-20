//****************************************************************************
// Firmware Version 1.3 - 20.07.2020
//
//
//                        Elektor Weatherstation 191148 
//
//  Plattform: ESP32 Pico Kit V4.0 or newer
//  
//  Pinmapping
//  IO02 : RES  ( RFM95 )
//  IO14 : SCKL ( RFM95 )
//  IO12 : MISO ( RFM95 )
//  IO13 : MOSI ( RFM95 )
//  IO15 : NSS  ( RFM95 )
//  IO32 : DIO2 ( RFM95 )
//  IO33 : DIO1 ( RFM95 )
//  IO34 : DIO0 ( RFM95 )
//  IO25 : SCL
//  IO26 : SDA
//  IO34 : WIND SPEED
//  IO38 : RAIN
//  IO37 : WIND DIR
//  IO19 : MISO ( SD-CARD )
//  IO23 ; MOSI ( SD-CARD )
//  IO18 : SCKL ( SD-CARD )
//  IO05 : CS0  ( SD-CARD )
//  IO09 : TX1 ( Particle Sensor )
//  IO10 : RX1 ( Particle Sensor )
//
//  ESP32 Arduino Core requiered
//
//  Librarys requiered:
//
//  - LiquidCrystal_I2C ( https://github.com/johnrickman/LiquidCrystal_I2C )
//  - eHaJo Absolut Pressure AddOn by Hannes Jochriem ( https://github.com/ehajo/WSEN-PADS )
//  - Adafruit Unified Sensors
//  - Adafruit BME280 Library
//  - Adafruit VEML6070 Library
//  - Adafruit VEML6075 Library
//  - Adafruit TSL2561 Library
//  - Adafruit TSL2591 Library
//  - Adafruit BusIO
//  - Time by Michael Margolis
//  - AdrduinoJson 6.x 
//  - PubSubclient by Nick o'Leary
//  - NTP Client Lib
//
//  Supported Sensors:
//  WH-SP-WS02 ( https://www.elektor.com/professional-outdoor-weather-station-wh-sp-ws02 )  
//  BME280 ( 0x77 / 0x76 ) ( https://www.elektor.com/bme280-mouser-intel-i2c-version-160109-91 )
//  Wuerth WSEN-PAD ( 0x5C / 0x5D )
//  Veml6070 ( 0x38 )
//  TSL2561 ( 0x29 / 0x39 / 0x49 )
//  VEML6075 (0x10)
//  TSL2591 ( 0x29 )
//  Honneywell HPMA115S0-XXX
//  SDS011
// 
//
//
//  Version Histroy:
//
//  Version 1.3
//   - Fixed a bug inside the particle sensors not returning the messurment vlaues, only NaN
//
//  Version 1.2
//  - Fixed a bug with VEML6075 and VEML6070 driver stack, not correctly reporting the sensors
//
//  Version 1.1
//  - Initial release
//
//****************************************************************************
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "FS.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SPIFFS.h"
#include <Update.h>


#include "./wifi_net.h"

#include "./src/TimeCore/timecore.h"
#include "./src/NTPClient/ntp_client.h"

#include "./src/I2C_Sensors/i2c_sensors.h"
#include "./src/uart_pm_sensors/uart_pm_sensors.h"

#include "./src/ValueMapping/ValueMapping.h"
#include "./src/InternalSensors/InternalSensors.h"


#include "./src/SenseBox/SenseBox.h"
#include "./src/ThingSpeak/thingspeak.h"

#include "./src/sdcard/sdcard_if.h"

#include "./datastore.h"
#include "./src/MQTTConnector/mqtt_task.h"

#include "webserver_map_fnc.h"
#include "webserver_sensebox_fnc.h"
#include "webserver_thinkspeak_fnc.h"
#include "webserver_time_fnc.h"

#include "./src/UDP_UniCastServer/UDP_UniCastServer.h"

#include "./src/lcd_menu/lcd_menu.h"

/* We define the pins used for the various components */
#define INPUT_RAIN           ( 38 )
#define INPUT_WINDSPEED      ( 34 )
#define INPUT_WINDDIR        ( 37 )

#define I2C0_SCL             ( 25 )
#define I2C0_SDA             ( 26 )

#define PARTICLESENSOR_RX    ( 10 )
#define PARTICLESENSOR_TX    (  9 )

#define SD_MISO              ( 15 )
#define SD_MOSI              ( 13 )
#define SD_SCK               ( 14 )
#define SD_CS0               ( 12 )

#define USERBTN0             ( 0 )
#define USERBTN1             ( 4 )


//Defined but currently not used
#define RFM95_MISO           ( 19 )
#define RFM95_MOSI           ( 23 )
#define RFM95_SCK            ( 18 )
#define RFM95_NSS            (  5 )
#define RFM95_RES            ( 0xFF )
#define RFM95_DIO0           ( 35 )
#define RFM95_DIO1           ( 33 )
#define RFM95_DIO2           ( 32 )

//--------------------------------------------------------------------------------------------------
I2C_Sensors TwoWireSensors(I2C0_SCL, I2C0_SDA );
UART_PM_Sensors PMSensor( PARTICLESENSOR_RX , PARTICLESENSOR_TX );
VALUEMAPPING SensorMapping;
InternalSensors IntSensors;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Timecore TimeCore;
NTP_Client NTPC;
SenseBoxUpload SenseBox;
ThinkspeakUpload ThinkSpeak;
UDPUniCastSever UDPServer;
//--------------------------------------------------------------------------------------------------


/**************************************************************************************************
 *    Function      : ReadSensorData
 *    Description   : This will access the function to read sensordata
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
bool ReadSensorData(float* data ,uint8_t ch){
  return SensorMapping.ReadMappedValue(data,ch);
}


/**************************************************************************************************
 *    Function      : setup_iopins
 *    Description   : This will enable the IO-Pins
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void setup_iopins( void ){
  // Every pin that is just IO will be defined here 
  pinMode( USERBTN0, INPUT_PULLUP );
  //If this button is read as 0 this means it is pressed
  if(USERBTN1 >= 0){
    pinMode( USERBTN1, INPUT_PULLUP );
  }

}


/**************************************************************************************************
 *    Function      : StartOTA
 *    Description   : This will prepare the OTA Service
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void StartOTA(){
  Serial.println("Start OTA Service");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

}

/**************************************************************************************************
 *    Function      : NTP_Task
 *    Description   : Task for the NTP Sync 
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void NTP_Task( void * param){
  #ifdef DEBUG_SERIAL
    Serial.println(F("Start NTP Task now"));
  #endif
  NTPC.ReadSettings();
  NTPC.begin( &TimeCore );
  NTPC.Sync();

  /* As we are in a sperate thread we can run in an endless loop */
  while( 1==1 ){
    /* This will send the Task to sleep for one second */
    vTaskDelay( 1000 / portTICK_PERIOD_MS );  
    NTPC.Tick();
    NTPC.Task();
  }
}

void StartUDPServer(){
  
  UDPServer.SetTXInervall(1);
  UDPServer.begin();

}

void StartCloudServices( void ){
  //Now we set up the connectors 
  ThinkSpeak.begin(false); 
  SenseBox.begin();
  MQTTTaskStart();
  
}

void StartNTP( void ){
    //This is a dedecated Task for the NTP Service 
  xTaskCreatePinnedToCore(
      NTP_Task,       /* Function to implement the task */
      "NTP_Task",  /* Name of the task */
      10000,          /* Stack size in words */
      NULL,           /* Task input parameter */
      1,              /* Priority of the task */
      NULL,           /* Task handle. */
      1); 
}




/**************************************************************************************************
 *    Function      : setup
 *    Description   : This will onyl run once after boot
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void setup() {
  Serial.begin(115200);
  SPIFFS.begin(); /* This can be called multiple times, allready mounted it will just return */
  datastoresetup();
  setup_iopins();

  //We also need to mount the SPIFFS
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
  }
  //This will setup the internal sensors
  IntSensors.begin(INPUT_WINDDIR, INPUT_WINDSPEED, INPUT_RAIN );
  /* Next is to collect the I²C-Zoo of supported sensor */
  TwoWireSensors.begin();
  LCDMenu( USERBTN0, USERBTN1 ,1);
  //This sould trigger autodetect
  PMSensor.begin( Serial1 , UART_PM_Sensors::SerialSensorDriver_t::NONE );
  //Next step is to setup wifi and check if the configured AP is in range
  WiFiClientEnable(true); //Force WiFi on 
  WiFiForceAP(false); //Diable Force AP
  #ifdef DEBUG_SERIAL
    Serial.println("Continue boot");
  #endif
  TimeCore.begin(true);
  //This will register the driver for the sensors we support
  SensorMapping.RegisterInternalSensors(&IntSensors);
  SensorMapping.RegisterI2CBus(&TwoWireSensors);
  SensorMapping.RegisterUartSensors(&PMSensor);
  SensorMapping.begin();
  
  SenseBox.RegisterDataAccess( ReadSensorData );
  ThinkSpeak.RegisterDataAccess( ReadSensorData );
  UDPServer.RegisterMappingAccess(&SensorMapping);

  SenseBox.InitConfig();
  ThinkSpeak.InitConfig();
  
  RegisterWiFiConnectedCB(StartOTA);
  RegisterWiFiConnectedCB(StartUDPServer);
  RegisterWiFiConnectedCB(StartCloudServices);
  RegisterWiFiConnectedCB(StartNTP);
  
  Serial.println("Initialize WiFi");
 //If the button is pressed during start we will go to the AP Mode
  if(0 == ReadButtonPressCnt() ){
    initWiFi( false, false );
  } else {
    Serial.println("Force System to AP");
    initWiFi( false , true );   
  }
  LCDMenuShow(0); 
  
  //As the system with all active componentes draws more than 500mA 
  //we first start the WIFi 

  //We start to register all the added functions for the webserver
  Webserver_Time_FunctionRegisterTimecore(&TimeCore);
  Webserver_Time_FunctionRegisterNTPClient(&NTPC);
  
  Webserver_SenseBox_RegisterSensebox(&SenseBox);
  Webserver_Thinkspeak_RegisterThinkspeak(&ThinkSpeak);
  
  SDCardRegisterMappingAccess(&SensorMapping);
  SDCardRegisterTimecore(&TimeCore);
 
  //We try to start the SD-Card and mount it 
  setup_sdcard(SD_SCK ,SD_MISO, SD_MOSI, SD_CS0 );
  sdcard_mount();

}


/**************************************************************************************************
 *    Function      : loop
 *    Description   : Main Loop
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void loop() {
  /* This will be executed in the arduino main loop */
  ArduinoOTA.handle();
  NetworkLoopTask();
}


