#include "SD.h"
#include "SPI.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>

#include "sdcard_if.h"

SPIClass SDSPI(HSPI);
Timecore* SDTimeCorePtr=nullptr;
int16_t CS_Pin = -1;
bool card_eject = false;
VALUEMAPPING* DataMapping = nullptr;

//We need a few parameter
SemaphoreHandle_t SDCfgSem;

SemaphoreHandle_t SDCardAccessSem;


bool LogEnable = false;
uint16_t LogInterval = 15;

void SDCardDataLog( void );
void SDCardLoging( void* param);
void SdCardLog_WriteConfig( void );
void SDCardLog_ReadConfig( void );

void SDCardRegisterMappingAccess(VALUEMAPPING* Mapping){
  DataMapping = Mapping;
}

void SDCardRegisterTimecore( Timecore* TC){
  SDTimeCorePtr=TC;
}

void setup_sdcard( int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin, int8_t sd_cs_pin ){
  
  SDSPI.begin(sd_sck_pin, sd_miso_pin, sd_mosi_pin, -1);
  CS_Pin=sd_cs_pin;
  card_eject = true; //Card not mounted
  SDCardAccessSem  = xSemaphoreCreateBinary();
  if(nullptr == SDCardAccessSem){
    abort();
  }
  xSemaphoreGive(SDCardAccessSem);
  SDCfgSem = xSemaphoreCreateBinary();
  if( nullptr == SDCfgSem){
    abort();
  }
  SDCardLog_ReadConfig();
  //We also start the Logging Task if requiered.....
  xTaskCreate(
                    SDCardLoging,          /* Task function. */
                    "SDCardLogging",        /* String with name of task. */
                    20000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle */


}

void sdcard_umount(){
    if (false == xSemaphoreTake(SDCardAccessSem, portMAX_DELAY ) ){
      return;
    } 
    card_eject = true;
    SD.end();
     Serial.println("SD-Card ejected");
    xSemaphoreGive(SDCardAccessSem);

}

void sdcard_mount(){
    if (false == xSemaphoreTake(SDCardAccessSem, portMAX_DELAY )){
      return;
    } 
    SD.end();
  if(false == SD.begin(CS_Pin, SDSPI) ){
    Serial.println("SD-Card mount failed");
     card_eject = true;
  } else {
    Serial.println("SD-Card mounted at '\\SD' ");
    card_eject = false;
  }
  xSemaphoreGive(SDCardAccessSem);
}

bool sdcard_getmounted( void ){
  if(  card_eject == true){
    return false;
  } else {
    return true;
  }
}

void sdcard_log_enable( bool ena){
      LogEnable = ena;
      SdCardLog_WriteConfig();
}

void sdcard_log_int( uint16_t interval){
      LogInterval = interval;
      SdCardLog_WriteConfig();
}

bool sdcard_log_getenable( void ){
  return LogEnable;
}

uint16_t sdcard_log_getinterval( void ){
  return LogInterval;
}

void sdcard_log_writesettings(bool ena, uint16_t interval){
   LogEnable = ena;
   LogInterval = interval;
   SdCardLog_WriteConfig();
}

void SdCardLog_WriteConfig( void ){
    //This will just convert the Mapping to JSON and write it to the local filesystem
    Serial.println("Write /sdcardlog.json ");
    File file = SPIFFS.open("/sdcardlog.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);
    
    doc["enabled"] = LogEnable;
    doc["interval"] = LogInterval;

    serializeJson(doc, file);
    file.close();
    if(nullptr != SDCfgSem){
      xSemaphoreGive(SDCfgSem);
    }
}

void SDCardLog_ReadConfig( void ){
String JSONData="";
//Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/sdcardlog.json")){
        File file = SPIFFS.open("/sdcardlog.json");
        
        Serial.print("Filesize:");
        Serial.println(file.available());
        while(file.available()){
          char ch = file.read();
          Serial.print(ch);
          JSONData=JSONData+ch;
        }
        Serial.println("");
        Serial.print("JSON:");
        Serial.println(JSONData);
     
        const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3) + 1580;
        DynamicJsonDocument doc(capacity);
        DeserializationError err =  deserializeJson(doc, JSONData);
        if(err) {
          Serial.print("/sdcardlog.json ");
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(err.c_str());
        }

        LogEnable = doc["enabled"];
        LogInterval = doc["interval"];
        file.close();
        
    } else {
        Serial.println("Read for /sdcardlog.json failed");
        LogEnable = false;
        LogInterval = 1;
        SdCardLog_WriteConfig();
      //We need to create a default config
    }

}

//Capacity in MB
uint32_t sdcard_GetCapacity( void ){
  uint32_t size_mb=0;
  if(SDCardAccessSem==nullptr){
    return 0;
  }
  if (false == xSemaphoreTake(SDCardAccessSem, 5)){
    Serial.println("SD Card Semaphore locked");
    return 0;
  } 
  if(true == sdcard_getmounted()){
      uint64_t bytesfree = SD.totalBytes();
      size_mb = bytesfree / 1024 / 1024 ;
  }
 


  xSemaphoreGive(SDCardAccessSem);
  return size_mb;
} 

//FreeSpace in MB
uint32_t sdcard_GetFreeSpace( void  ){
  if(SDCardAccessSem==nullptr){
    return 0;
  }
  if (false == xSemaphoreTake(SDCardAccessSem, 5)){
    Serial.println("SD Card Semaphore locked");
    return 0;
  } 
  uint64_t BytesFree = SD.totalBytes() - SD.usedBytes() ;
  uint32_t MBFree = ( BytesFree / 1024 / 1024 ); 
  xSemaphoreGive(SDCardAccessSem);
  return MBFree;
}


void SDCardLoging( void* param){
  bool LoggingEnable = true ;
  uint32_t Interval = portMAX_DELAY;

  //sainity checks
  if(SDCfgSem == nullptr){
    //We need a semaphore here !
    SDCfgSem = xSemaphoreCreateBinary();
  }

  //Config for the logging will be stored also in a json file
  //We need to read it if it is existing
  xSemaphoreGive(SDCfgSem);
  
  while(1 == 1){

    //We need to load the ionterval for the logging
    //We run in minutes
    if( LoggingEnable == true ){
      Interval= LogInterval*60*1000; 
      if(Interval<1){
        Interval=10*1000; //1 Minute min Time;
      }
    } else {
      Interval = portMAX_DELAY;
      Serial.println("SDLog: Entering Sleep");
    }
    Serial.print("Next SDLog in ");
    Serial.print(Interval);
    Serial.println(" Ticks (ms)");
    if( false == xSemaphoreTake( SDCfgSem, Interval ) ){
      //No Configchange at all we can simpy upload the data 
      SDCardDataLog();
    } else {
      //We have a configchange 
      Serial.println("SD Card: Config changed, apply settings");
      SDCardLog_ReadConfig();

    }


  }



}


void SDCardDataLog( void ){
  
  /* 
    The SD-Card can log at a different intervall than the Network Sinks 
    We need to impliment a mutex as also faild network write attemps are 
    stored on the sd card 
  */
  if(nullptr == DataMapping){
    Serial.println("No Datasource registred");
    return;
  }

  if (false == xSemaphoreTake(SDCardAccessSem, 5)){
    Serial.println("SD Card Semaphore locked");
    return;
  } 
 
  if(true == card_eject){
      xSemaphoreGive(SDCardAccessSem);
      Serial.println("SD Card not mounted");
      return; //We won't try to mount the card!
  }
  
  bool CardMounted = false ;
  uint32_t timestamp = 0;
  datum_t Date;
  bzero((void*)(&Date),sizeof(datum_t));

  if(SDTimeCorePtr != nullptr){
    timestamp = SDTimeCorePtr->GetUTC( );
    Date = SDTimeCorePtr->ConvertToDatum( timestamp );
  
  }

  if(true == card_eject){
    CardMounted=false;
  } else {
    CardMounted=true;
  }

  if(false == CardMounted){
    xSemaphoreGive(SDCardAccessSem);
    Serial.println("SD Card not connected, leave");
    return; //That is all we can do for now 
  }

  File file;
  //Filename is : YYYY-MM-DD.CSV
  //We need to grab the timestamp if possible
  char DateString[11]; 
  char TimeString[10];
  snprintf(DateString, sizeof(DateString),"%4d-%02d-%02d",Date.year,Date.month,Date.day);
  snprintf(TimeString, sizeof(TimeString),"%02d:%02d:%02d", Date.hour, Date.minute, Date.second);
  String FileName = String(DateString) + "-Log.csv";
  String RootPath = "";
  String Path = RootPath + "/" + FileName;
  
  //We will write to the card till 5Mb of space is left
  //Filename should be DataLog 0 to x
  uint64_t BytesFree = SD.totalBytes() - SD.usedBytes() ;
  uint32_t MBFree = ( BytesFree / 1024 / 1024 ); 
  if( MBFree > 5 ){   
    Serial.print("SD Card start writing @");
    Serial.println(TimeString);
    Serial.print("Path:");
    Serial.print(Path);
    if(true == SD.exists(Path) ){
      file = SD.open(Path, FILE_APPEND);
      Serial.println(" - File exist, append data");
    } else {
      file = SD.open(Path, FILE_WRITE);
      if(file){
        file.print("Time ,");
        for(uint8_t i=0;i<64;i++){
            file.print("Channel ");
            file.print(i);
            file.print(" Name");
            file.print(",");
            file.print("Value");
            if(i<63){
              file.print(",");
            } else {
              file.print("\r\n");
    
            }
        }       
      } else {
        Serial.println("Can't open file for write");
        xSemaphoreGive(SDCardAccessSem);
        sdcard_umount();
        return;    
      }
    }

    if(file){
      //We need to output the current time if possible....
      file.print(TimeString);
      file.print(",");
      for(uint8_t i=0;i<64;i++){
        float value = NAN;
        if(false == DataMapping->ReadMappedValue(&value,i)){
          file.print("");
          file.print(",");
          file.print("");
          
        } else {
          String name = DataMapping->GetSensorNameByChannel(i);
          file.print(name);
          file.print(",");
          file.print(value);
        }
        if(i<63){
            file.print(",");
        } else {
           file.print("\n");
        }
      } 
      file.close();
    } else {
      xSemaphoreGive(SDCardAccessSem);
       Serial.println("SD Card file error");
      return; //File error 
    }
  } else {
    //Out of memory 
    Serial.println("SD Card no space left on device");
    Serial.print("MB Free:");
    Serial.println(MBFree);

  }
  xSemaphoreGive(SDCardAccessSem);
}