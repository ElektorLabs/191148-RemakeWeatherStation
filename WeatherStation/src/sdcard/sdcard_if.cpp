#include "SD.h"
#include "SPI.h"

#include "sdcard_if.h"

SPIClass SDSPI(HSPI);
Timecore* TimeCorePtr=nullptr;
int16_t CS_Pin = -1;
bool card_eject = false;
VALUEMAPPING* DataMapping = nullptr;

void SDCardRegisterMappingAccess(VALUEMAPPING* Mapping){
  DataMapping = Mapping;
}

void register_timecore( Timecore* TC){
  TimeCorePtr=TC;
}

void setup_sdcard( int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin, int8_t sd_cs_pin ){
  
  SDSPI.begin(sd_sck_pin, sd_miso_pin, sd_mosi_pin, -1);
  CS_Pin=sd_cs_pin;
  card_eject = true; //Card not mounted
}

void sdcard_umount(){
    card_eject = true;
    SD.end();
}

void sdcard_mound(){
    if(false == SD.begin(CS_Pin, SDSPI) ){
    Serial.println("SD-Card mount failed");
  } else {
    Serial.println("SD-Card mounted at '\\SD' ");
  }
}


//We need a few parameter
SemaphoreHandle_t SDCfgSem;
void SDCardLoging( void* param){
  bool LoggingEnable = true ;
  uint16_t Interval = 5;

  //sainity checks
  if(SDCfgSem == nullptr){
    //We need a semaphore here !
    SDCfgSem = xSemaphoreCreateMutex();
  }

  //Config for the logging will be stored also in a json file
  //We need to read it if it is existing

  while(1 == 1){

    //We need to load the ionterval for the logging
    //We run in minutes
    if( ){
      Interval= *60*1000;
      if(Interval<1){
        Interval=60*1000; //1 Minute min Time;
      }
    } else {
      Interval = portMAX_DELAY;
    }
    if( false == xSemaphoreTake( SDCfgSem, Interval ) ){
      //No Configchange at all we can simpy upload the data 
      Serial.println("SD Card: Prepare save ( enter code here )");
    } else {
      //We have a configchange 
      Serial.println("SD Card: Config changed, apply settings");

    }


  }



}


void SDCardDataLog( void ){
  
  /* 
    The SD-Card can log at a different intervall than the Network Sinks 
    We need to impliment a mutex as also faild network write attemps are 
    stored on the sd card 
  */
  if(true == card_eject){
      return; //We won't try to mount the card!
  }
  
  bool CardMounted = false ;
  uint32_t timestamp = 0;
  datum_t Date;
  bzero((void*)(&Date),sizeof(datum_t));

  if(TimeCorePtr != nullptr){
    timestamp = TimeCorePtr->GetUTC( );
    Date = TimeCorePtr->ConvertToDatum( timestamp );
  
  }



  //File format will be CSV ( comma seperated vales as in RFC4180 )
  if( SD.cardSize() != CARD_NONE ){
    //Card seems still to be mounted
    CardMounted= true;
  } else {
    //We try to mount the card again 
    if ( false == SD.begin(CS_Pin, SDSPI) ){
      //We can't mount the card
      CardMounted=false; 
    } else {
      //Card is mounted, try to write data
      CardMounted=true;
    }
  }

  if(false == CardMounted){
    return; //That is all we can do for now 
  }

  File file;
  //Filename is : YYYY-MM-DD.CSV
  //We need to grab the timestamp if possible
  String FileName = String(Date.year) + "-" +  String(Date.month) + "-" + String(Date.day) + "-Log.csv";
  String RootPath = "\\SD\\";
  String Path = RootPath + "\\" + FileName;
  //We will write to the card till 5Mb of space is left
  //Filename should be DataLog 0 to x
  if( 5000000 > ( SD.totalBytes() - SD.usedBytes() ) ){   
    
    if(true == SD.exists(Path) ){
      file = SD.open(Path, FILE_APPEND);
    } else {
      file.print("Time ,");
      file = SD.open(Path, FILE_WRITE);
      for(uint8_t i=0;i<64;i++){
          file.print("Channel ");
          file.print(i);
          file.print(" Name");
          file.print(",");
          file.print("Value");
          file.print(",");
      }
      file.print("\n\r");
    }

    if(file){

      for(uint8_t i=0;i<64;i++){
        float value = NAN;
        if(false == DataMapping->ReadMappedValue(&value,i)){
          Serial.printf("Channel %i not mapped\n\r",i);
          file.print("");
          file.print(",");
          file.print("");
          file.print(",");
        } else {
          
          Serial.printf("Channel %i Value %f",i,value );
          Serial.print(" @ ");
          String name = DataMapping->GetSensorNameByChannel(i);
          Serial.println(name);
          file.print(name);
          file.print(",");
          file.print(value);
          file.print(",");

        }
        file.print("\n\r");
      } 
      
      file.close();
    } else {
      return; //File error 
    }
  } else {
    //Out of memory 
  }

}