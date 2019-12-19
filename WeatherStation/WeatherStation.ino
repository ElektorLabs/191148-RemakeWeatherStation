//****************************************************************************
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
//  Librarys requiered:
// 
//****************************************************************************
#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SPIFFS.h"

#include "./src/lora_wan/lorawan.h"
#include "./src/lora_raw/loraraw.h"

#include "./src/I2C_Sensors/i2c_sensors.h"
#include "./src/uart_pm_sensors/uart_pm_sensors.h"

#include "./src/ValueMapping/ValueMapping.h"
#include "./src/InternalSensors/InternalSensors.h"
#include "./src/TimeCore/timecore.h"
#include "./wifi_net.h"

#include "./datastore.h"

#include "webserver_fnc.h"

/* We define the pins used for the various components */

#define INPUT_RAIN           ( 38 )
#define INPUT_WINDSPEED      ( 34 )
#define INPUT_WINDDIR        ( 37 )

#define I2C0_SCL             ( 21 )
#define I2C0_SDA             ( 33 )

#define PARTICLESENSOR_RX    ( 10 )
#define PARTICLESENSOR_TX    (  9 )

#define SD_MISO              ( 12 )
#define SD_MOSI              ( 13 )
#define SD_SCK               ( 14 )
#define SD_CS0               ( 15 )

#define RFM95_MISO           ( 19 )
#define RFM95_MOSI           ( 23 )
#define RFM95_SCK            ( 18 )
#define RFM95_NSS            (  5 )
#define RFM95_RES            ( 0xFF )
#define RFM95_DIO0           ( 35 )
#define RFM95_DIO1           ( 33 )
#define RFM95_DIO2           ( 32 )

#define USERBTN0             ( 0 )


 SPIClass SDSPI(HSPI);
 lorawan LORAWAN;


 I2C_Sensors TwoWireSensors(I2C0_SCL, I2C0_SDA );
 UART_PM_Sensors PMSensor( PARTICLESENSOR_RX , PARTICLESENSOR_TX );
 VALUEMAPPING SensorMapping;
 InternalSensors IntSensors;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Timecore TimeCore;

void DataLoggingTask(void* param);

void setup_iopins( void ){
  // Every pin that is just IO will be defined here 
  pinMode( USERBTN0, INPUT_PULLUP );
  //If this button is read as 0 this means it is pressed

}

void setup_sdcard( void ){
  
  SDSPI.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
  if(false == SD.begin(SD_CS0, SDSPI) ){
    Serial.println("SD-Card mount failed");
  } else {
    Serial.println("SD-Card mounted at '\\SD' ");
  }
}





void setup() {
  Serial.begin(115200);
  datastoresetup();
  setup_iopins();
  setup_sdcard();
  //We also need to mount the SPIFFS
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
  }
  //This will setup the internal sensors
  IntSensors.begin(INPUT_WINDDIR, INPUT_WINDSPEED, INPUT_RAIN );
  /* Next is to collect the I²C-Zoo of supported sensor */
  TwoWireSensors.begin();
  
  PMSensor.begin( Serial1 , UART_PM_Sensors::SerialSensorDriver_t::SENSOR_HPM115S0 );
  vTaskDelay(2000);
  if ( false == LORAWAN.begin( RFM95_NSS, 0xFF , 0xFF, RFM95_DIO0, RFM95_DIO1, RFM95_DIO2 ) ){
    //LoRa Module not found, we won't schedule any transmission
  } else {
    //LoRa WAN Enabled
  }
  //Next step is to setup wifi and check if the configured AP is in range
  /* We check if the boot button is pressed ( IO00 ) */
  delay(2000);
  WiFiClientEnable(true); //Force WiFi on 
  WiFiForceAP(false); //Diable Force AP
  if(0 != digitalRead(USERBTN0 )){
    initWiFi( false, false );
  } else {
    Serial.println("Force System to AP");
    initWiFi( false , true );   
  }
  TimeCore.begin(true);
  //We need to register the drivers
  SensorMapping.RegisterInternalSensors(&IntSensors);
  SensorMapping.RegisterI2CBus(&TwoWireSensors);
  SensorMapping.RegisterUartSensors(&PMSensor);

  //
  SensorMapping.begin();


    xTaskCreatePinnedToCore(
                    DataLoggingTask,   /* Function to implement the task */
                    "DataLogging",  /* Name of the task */
                    16000,       /* Stack size in words */
                    NULL,       /* Task input parameter */
                    5,          /* Priority of the task */
                    NULL,  /* Task handle. */
                    0);         /* Core where the task should run */



}



void loop() {
  float value =0;
  static uint32_t last_ms = 0;
  uint32_t milli = millis();
  /* This will be executed in the arduino main loop */
  LORAWAN.LoRaWAN_Task();
  //The Rain and WindSensors are now processed by its own tasks
  //We can now take care for the important stuff
  if( (milli - last_ms) > 1000 ){
    last_ms = milli;
    //Once a second to be called 
  }
  NetworkLoopTask();
}


/* Datalogging intervall can for the network only set global */
void DataLoggingTask(void* param){
  //We will force a mapping for the internal sensors here , just for testing
  VALUEMAPPING::SensorElementEntry_t Element;
  Element.Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
  Element.ValueType = DATAUNITS::SPEED;
  Element.ChannelIDX = 0;
  SensorMapping.SetMappingForChannel(0, Element);


  Element.Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
  Element.ValueType = DATAUNITS::DIRECTION;
  Element.ChannelIDX = 0;
  
  SensorMapping.SetMappingForChannel(1, Element);
  
  Element.Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
  Element.ValueType = DATAUNITS::RAINAMOUNT;
  Element.ChannelIDX = 0;
  
  SensorMapping.SetMappingForChannel(2, Element);
  //This will grab every 10 seconds new values from the mapped sensors and display them as test
  while(1==1){
    for(uint8_t i=0;i<64;i++){
      float value = NAN;
      if(false == SensorMapping.ReadMappedValue(&value,i)){
        Serial.printf("Channel %i not mapped\n\r",i);
      } else {
        Serial.printf("Channel %i Value %f",i,value );
        Serial.print(" @ ");
        String name = SensorMapping.GetSensorNameByChannel(i);
        Serial.println(name);
      }

    }
  vTaskDelay(10000); //10s delay
  }



}



void NetworkDataLogging( void ){

  //This will do the data logging to the configured channels
  /*Currently we have 
    - MQTT
    - Sensbox
    - ThingsNetwork
    - TTN ( LoRaWAN , RFM95W )
  */
  /* Logging will be called for all possible network sinks at the same time */

  /* We need to grab the configured channel mapping 
     Channels are 0-255 and mapped to a sensror value 




}

void SDCardDataLogging( void ){
  /* 
    The SD-Card can log at a different intervall than the Network Sinks 
    We need to impliment a mutex as also faild network write attemps are 
    stored on the sd card 
  */
  bool CardMounted = false ;
  uint32_t timestamp = TimeCore.GetUTC( );
  datum_t Date = TimeCore.ConvertToDatum( timestamp );

  //File format will be CSV ( comma seperated vales as in RFC4180 )
  if( SD.cardSize() != CARD_NONE ){
    //Card seems still to be mounted
    CardMounted= true;
  } else {
    //We try to mount the card again 
    if ( false == SD.begin(SD_CS0, SDSPI) ){
      //We can't mount the card
      CardMounted=false; 
    } else {
      //Card is mounted, try to write data
      CardMounted=false;
    }
  }

  if(false == CardMounted){
    return; //That is all we can do for now 
  }

  File file;
  //Filename is : YYYY-MM-DD.CSV
  //We need to grab the timestamp if possible
  String FileName = "";//String(Date.year) + "-" String(Date.month) +"-" + String(Date.day) + "-Log.csv";
  String RootPath = "\\SD\\";
  String Path = RootPath + "\\" + FileName;
  //We will write to the card till 5Mb of space is left
  //Filename should be DataLog 0 to x
  if( 5000000 > ( SD.totalBytes() - SD.usedBytes() ) ){   
    
    if(true == SD.exists(Path) ){
      file = SD.open("\\SD\\DataLog.csv", FILE_APPEND);
      if(file){
        //Okay we can write
      } else {
        return; 
      }
    } else {

      file = SD.open("\\SD\\", FILE_WRITE);
      if(file){
        //Okay we can write
      } else {
        return;
      }
    }
  } else {
    return; // Out of space 
  }
}