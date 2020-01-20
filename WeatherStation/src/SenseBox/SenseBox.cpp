//upload the sensor values to senseBox
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "sslCertificate.h"
#include "../../wifi_net.h"
#include "SenseBox.h"




SenseBoxUpload::SenseBoxUpload( void ){

  //For the Upload we also use a mapping table but limit the amount of data currently to 16 entrys
  //For every entry we have a internal channel no and also a senseBoxID at hand ( hopfully )

}

SenseBoxUpload::~SenseBoxUpload( void ){

}

void SenseBoxUpload::begin( bool usehttps){
  
  this->usesecure = usehttps;
  
  
  TaskData.obj=this;
  TaskData.CfgSem = xSemaphoreCreateMutex();
  
  ReadMapping();
  ReadSettings();

  client = new WiFiClient();
  clientS = new WiFiClientSecure();
  if(true == usehttps){
    clientS->setCACert(ssl_sb_certificate);
  }

  xTaskCreate(
                    UploadTaskFnc,          /* Task function. */
                    "SenseBoxUpload",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    &TaskData,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle */

}

void SenseBoxUpload::RegisterDataAccess(SenseBoxUpload::DataAccesFnc Fnc){
  DaFnc = Fnc; //Register the pointer...
}

void SenseBoxUpload::WriteMapping( void ){
  //This will just convert the Mapping to JSON and write it to the local filesystem
   File file = SPIFFS.open("/SenseBoxMapping.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    JsonArray SenseboxMapping = doc.createNestedArray("Mapping");
    for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
            JsonObject MappingObj = SenseboxMapping.createNestedObject();
            MappingObj["Enabled"] = Mapping[i].enable;
            MappingObj["Channel"] = Mapping[i].StationChannelIdx;
            if(sizeof(Mapping[i].SenseBoxSensorID)>1){
              Mapping[i].SenseBoxSensorID[(sizeof(Mapping[i].SenseBoxSensorID)-1)]=0; //Just to terminate the string propper
              MappingObj["Key"] = String(Mapping[i].SenseBoxSensorID);
            } else {
              MappingObj["Key"] = "";
            }
    }
    serializeJson(doc, file);
    file.close();
}

void SenseBoxUpload::ReadMapping( void ){

//Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/SenseBoxMapping.json")){
        File file = SPIFFS.open("/SenseBoxMapping.json");
        //We need to read the file into the ArduinoJson Parser
         /*
        ReadBufferingStream bufferingStream(file, 64);
        deserialzeJson(doc, bufferingStream);
        */
        
        const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3) + 1580;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, file);
        JsonArray SenseboxMapping = doc["Mapping"];
        for(uint8_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] ));i++){
            JsonObject MappingObj = SenseboxMapping[i];
           
            Mapping[i].enable =MappingObj["Enabled"];
            Mapping[i].StationChannelIdx = MappingObj["Channel"];
            String Key = MappingObj["Key"];

            if(Key.length()<32){
              strncpy(Mapping[i].SenseBoxSensorID,Key.c_str(),sizeof(Mapping[i].SenseBoxSensorID));
            } else {
              Mapping[i].SenseBoxSensorID[0]=0;
            }

        }
    } else {
        //We need to create a blank mapping scheme
        for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
              Mapping[i].enable;
              Mapping[i].StationChannelIdx;
            for(uint8_t i=0; i< sizeof(Mapping[i].SenseBoxSensorID);i++){
              Mapping[i].SenseBoxSensorID[i]=0;
            }
         }
         Serial.println("SenseBoxUpload: Write blank config");
         WriteMapping();
    }

}

void SenseBoxUpload::WriteSettings(){

/*
{
 "SenseboxID": "000000000000000000000000000000",
 "UploadInterval":15,
 "Enabled":false
}
*/

    File file = SPIFFS.open("/SenseBoxSetting.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    doc["SenseboxID"] = String(Settings.SenseBoxID); //SensboxID needed for the upload
    doc["UploadInterval"] = Settings.UploadInterval; //Interval in minutes for new data
    doc["Enabled"] = Settings.Enabled; //If the uplaod is enabled or not 
    serializeJson(doc, file);
    file.close();
    if(TaskData.CfgSem!=nullptr){
      xSemaphoreGive(TaskData.CfgSem);
    }


}

void SenseBoxUpload::ReadSettings(){
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);
      if(SPIFFS.exists("/SenseBoxSetting.json")){
        File file = SPIFFS.open("/SenseBoxSetting.json");
        deserializeJson(doc, file);
        
        const char* SenseboxID = doc["SenseboxID"]; 
        int UploadInterval = doc["UploadInterval"]; 
        bool Enabled = doc["Enabled"];

        //Sainity checks 
        strncpy(Settings.SenseBoxID,SenseboxID,sizeof(Settings.SenseBoxID));
        Settings.SenseBoxID[sizeof(Settings.SenseBoxID)-1]=0;

        if(UploadInterval<=0){
          UploadInterval=1;
        } 
        if(UploadInterval>UINT16_MAX){
          UploadInterval=UINT16_MAX;
        }

        
        Settings.UploadInterval=UploadInterval; //Interval in minutes for new data
        Settings.Enabled = Enabled; //If the uplaod is enabled or not 


      } else {
        //We generate an empty file
         for(uint32_t i=0;i<sizeof(Settings.SenseBoxID)-1;i++){
          Settings.SenseBoxID[i]=0;
        }
        Settings.UploadInterval=1; //Interval in minutes for new data
        Settings.Enabled=false; //If the uplaod is enabled or not 
    
      }

      

}



bool SenseBoxUpload::PostData(  SenseBoxUpload* obj ) {
  WiFiClient* clientptr=nullptr;
  if(false == obj->Settings.Enabled){
    Serial.println("SenseBox upload disabled");
    return false;
  }
  if (0 == obj->Settings.SenseBoxID[0]) {
    Serial.println("SenseBoxID not set");
    return false;
  }
  Serial.println("Start Uploading to SenseBox");
  
  //As we still use the HTTP POST we need to generate the POST String
  String csv;
  
  for(uint32_t i=0;i<( sizeof(obj->Mapping) / sizeof( obj->Mapping[0] )  );i++){
    if( (true==obj->Mapping[i].enable) && ( 0 != obj->Mapping[i].SenseBoxSensorID[0] ) ){
      //Next is a try to read the data from the sensors
      float value = NAN;
      if(obj->DaFnc != nullptr ){
        if(true == obj->DaFnc(&value, obj->Mapping[i].StationChannelIdx) ){
          //Okay we got a reading, we add the data to the sting
          csv+= String(obj->Mapping[i].SenseBoxSensorID)+","+String(value)+"\n\r";
        } else {
          //We have a mapping problem at all.....
          Serial.printf("SensBox upload: Requested Channel %i : No Mapped  for Stationchanne %i", i, obj->Mapping[i].StationChannelIdx);
        }
      } else {
        Serial.printf("SensBox upload: Requested Channel %i : no DataSource", i, obj->Mapping[i].StationChannelIdx);
      }
        
    }

  }
  //This is for debug only
  Serial.println("Current CSV String:");
  Serial.println(csv);
  Serial.println("# End of String #");
  //Sainity check if we have any data
  if (csv == "") {
    Serial.println("Sensor API: Keys not set, Channels all disabled or mapping broken");
    return false;
  }
  
  String senseBoxHost = "api.opensensemap.org";
  String senseBoxUrl = "/boxes/" + String(obj->Settings.SenseBoxID) + "/data";
  String headers = "Content-Type: text/csv\r\n";
  headers += "Connection: close\r\n";
  headers += "Content-Length: " + String(csv.length()) + "\r\n";
  if(true == obj->usesecure){
      clientptr=obj->clientS;
  } else {
     clientptr=obj->client;
  }

  if(false == RequestWiFiConnection() ){
    //We need to write the CSV to a file and log the error....
    return false;

  } else {
    String resp = SenseBoxUpload::performRequest(clientptr, senseBoxHost, senseBoxUrl, 443, "POST", headers, csv);
    //We should check the response....
    ReleaseWiFiConnection();
    return true;

  }

  

}

//request the specified url of the specified host
String SenseBoxUpload::performRequest(WiFiClient* c , String host, String url, int port , String method , String headers , String data ) {
  Serial.println("Connecting to host '" + host + "' on port " + String(port));
  if(false == c->connect(host.c_str(), port)){
    //Something went wrong.....
    return "";
  } //default ports: http: port 80, https: 443
  String request = method + " " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   headers + "\r\n";
  Serial.println("Requesting url: " + request);
  c->print(request);
  if (data != "") {
    Serial.println("Data: " + data);
    c->print(data + "\r\n");
  }
  
  unsigned long timeout = millis();
  while (c->available() == 0) {
    if (timeout + 5000 < millis()) {
      Serial.println("Client timeout");
      c->stop();
      return "";
    }
    delay(1);
  }
  //read client reply
  String response;
  while(c->available()) {
    response = c->readStringUntil('\r');
  }
  Serial.println("Response: " + response);
  c->stop();
  return response;
}



void SenseBoxUpload::UploadTaskFnc(void* params){
  uint32_t WaitTime = portMAX_DELAY;
  TaskData_t* TaskData = (TaskData_t*) params;
  Serial.println("Sensbox UploadTask started");
  while(1==1){


    //We have a config changed mutex...TickType_t is uint32 on esp32
    //this means we can wait up to 71.582 max waittime....
    // ( 2^32 -1 ) / 1000(Ticks/s) / 60(s/Minutes)  = 71582 Minutes;

    
    if(true==TaskData->obj->Settings.Enabled){
      WaitTime=TaskData->obj->Settings.UploadInterval*60*1000;
      if(WaitTime<1){
        WaitTime=60*1000; //1 Minute min Time;
      }
    } else {
      WaitTime = portMAX_DELAY;
    }
    Serial.print("Sensebox will upload in ");
    Serial.print(WaitTime);
    Serial.println(" Ticks( ms )");
    if( false == xSemaphoreTake( TaskData->CfgSem, WaitTime ) ){
      //No Configchange at all we can simpy upload the data 
      //If WiFi is in Start-Stop-mode activate it now.......
      Serial.println("SenseBox: Prepare Upload ( enter code here )");
      PostData(TaskData->obj);
    } else {
      //We have a configchange 
      Serial.println("SenseBox: Config changed, apply settings");

    }


  }



}


void SenseBoxUpload::SetSensBoxID( String ID ){
  strncpy(Settings.SenseBoxID,ID.c_str(),sizeof(Settings.SenseBoxID) );
  WriteSettings();
}

void SenseBoxUpload::SetSensBoxEnable( bool Enable ){
   Settings.Enabled = Enable;
   WriteSettings();
}

void SenseBoxUpload::SetSensBoxUploadInterval( uint16_t Interval ){
   Settings.UploadInterval = Interval;
   WriteSettings();
}

void SenseBoxUpload::SetMapping(uint8_t Channel, SensBoxMapping_t Map){
   if(  ( sizeof(Mapping) / sizeof( Mapping[0] )) <= Channel ){
     return;
   }
  
   Mapping[Channel] = Map ;
   WriteMapping();
}

String SenseBoxUpload::GetSensBoxID( void ){

  return String(Settings.SenseBoxID);
   
}

bool SenseBoxUpload::GetSensBoxEnable( void ){
  return Settings.Enabled;
   
}

uint16_t SenseBoxUpload::GetSensBoxUploadInterval( void ){
  return Settings.UploadInterval;
   
}

SenseBoxUpload::SensBoxMapping_t SenseBoxUpload::GetMapping(uint8_t Channel ){
   if(  ( sizeof(Mapping) / sizeof( Mapping[0] )) <= Channel ){
     Channel = 0;
   }
   return Mapping[Channel];
}

    
uint8_t SenseBoxUpload::GetMaxMappingChannels( void ){
  return ( sizeof(Mapping) / sizeof( Mapping[0] ));
}