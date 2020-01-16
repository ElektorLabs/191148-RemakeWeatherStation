//upload the sensor values to senseBox
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "sslCertificate.h"
#include "../../wifi_net.h"
#include "thinkspeak.h"



ThinkspeakUpload::ThinkspeakUpload( void ){

  //For the Upload we also use a mapping table but limit the amount of data currently to 16 entrys
  //For every entry we have a internal channel no and also a senseBoxID at hand ( hopfully )


}

ThinkspeakUpload::~ThinkspeakUpload( void ){

}

void ThinkspeakUpload::begin(  bool usehttps ){
  usesecure = usehttps;
  TaskData.obj=this;
  TaskData.CfgSem = xSemaphoreCreateMutex();
  
  ReadMapping();
  ReadSettings();

  client = new WiFiClient();
  clientS = new WiFiClientSecure();
  clientS->setCACert(ssl_ts_certificate);
  
  xTaskCreate(
                    UploadTaskFnc,          /* Task function. */
                    "ThinkspeakUpload",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    &TaskData,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle */

}

void ThinkspeakUpload::RegisterDataAccess(ThinkspeakUpload::DataAccesFnc Fnc){
  DaFnc = Fnc; //Register the pointer...
}


void ThinkspeakUpload::WriteMapping( void ){
  //This will just convert the Mapping to JSON and write it to the local filesystem
   File file = SPIFFS.open("/ThinkspeakMapping.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    JsonArray SenseboxMapping = doc.createNestedArray("Mapping");
    for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
            JsonObject MappingObj = SenseboxMapping.createNestedObject();
            MappingObj["Enabled"] = Mapping[i].enable;
            MappingObj["Channel"] = Mapping[i].StationChannelIdx;            
    }
    serializeJson(doc, file);
}

void ThinkspeakUpload::ReadMapping( void ){

//Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/ThinkspeakMapping.json")){
        File file = SPIFFS.open("/ThinkspeakMapping.json");
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
        }
    } else {
        //We need to create a blank mapping scheme
        for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
              Mapping[i].enable;
              Mapping[i].StationChannelIdx;
         }
         Serial.println("ThinkspeakUpload: Write blank config");
         WriteMapping();
    }

}

void ThinkspeakUpload::WriteSettings(){

/*
{
 "SenseboxID": "000000000000000000000000000000",
 "UploadInterval":15,
 "Enabled":false
}
*/

    File file = SPIFFS.open("/ThinkspeakSetting.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    doc["ThinkspeakAPIKey"] = String(Settings.ThingspealAPIKey); //SensboxID needed for the upload
    doc["UploadInterval"] = Settings.UploadInterval; //Interval in minutes for new data
    doc["Enabled"] = Settings.Enabled; //If the uplaod is enabled or not 
    serializeJson(doc, file);


}

void ThinkspeakUpload::ReadSettings(){
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

      if(SPIFFS.exists("/ThinkspeakSetting.json")){
        File file = SPIFFS.open("/ThinkspeakSetting.json");
        deserializeJson(doc, file);
        
        const char* SenseboxID = doc["ThinkspeakAPIKey"]; 
        int UploadInterval = doc["UploadInterval"]; 
        bool Enabled = doc["Enabled"];

        //Sainity checks 
        strncpy(Settings.ThingspealAPIKey,SenseboxID,sizeof(Settings.ThingspealAPIKey));
        Settings.ThingspealAPIKey[sizeof(Settings.ThingspealAPIKey)-1]=0;

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
         for(uint32_t i=0;i<sizeof(Settings.ThingspealAPIKey)-1;i++){
          Settings.ThingspealAPIKey[i]=0;
        }
        Settings.UploadInterval=1; //Interval in minutes for new data
        Settings.Enabled=false; //If the uplaod is enabled or not 
    
      }

      

}



bool ThinkspeakUpload::PostData(  ThinkspeakUpload* obj ) {
  WiFiClient* clientptr = nullptr;
  String thingspeakApi = String(obj->Settings.ThingspealAPIKey);

  if(false == obj->Settings.Enabled){
    Serial.println("Thinkspeak upload disabled");
    return false;
  }
  if (0 == obj->Settings.ThingspealAPIKey[0]) {
    Serial.println("Thinkspeak API Key not set");
    return false;
  } 
  Serial.println("Start Uploading to Thinkspeak");
  
  //As we still use the HTTP POST we need to generate the POST String
  String thingspeakHost = "api.thingspeak.com";
  String thingspeakUrl = "/update";
  thingspeakUrl += "?api_key=" + thingspeakApi;
  uint8_t added_fields=0;
  for(uint32_t i=0;i<( sizeof(obj->Mapping) / sizeof( obj->Mapping[0] )  );i++){
    if( (true==obj->Mapping[i].enable) ){
      //Next is a try to read the data from the sensors
      float value = NAN;
      if(obj->DaFnc != nullptr ){
        if(true == obj->DaFnc(&value, obj->Mapping[i].StationChannelIdx) ){
          //Okay we got a reading, we add the data to the sting
           thingspeakUrl += "&field" + String(i) + "=" + String(value); 
           added_fields++;
        } else {
          //We have a mapping problem at all.....
          Serial.printf(" Thinkspeak upload: Requested Channel %i : No Mapped  for Stationchanne %i", i, obj->Mapping[i].StationChannelIdx);
        }
      } else {
        Serial.printf(" Thinkspeak upload: Requested Channel %i : no DataSource", i, obj->Mapping[i].StationChannelIdx);
      }
        
    }

  }
  //This is for debug only
  Serial.println("Current URL String:");
  Serial.println(thingspeakUrl);
  Serial.println("# End of String #");
  //Sainity check if we have any data
  if (0 ==added_fields) {
    Serial.println("Thinkspeak API: Channels all disabled or mapping broken");
    return false;
  }
  
  Serial.println("Uploading to thingspeak");  
 
  if(true == obj->usesecure){
      clientptr=obj->clientS;
  } else {
     clientptr=obj->client;
  }

  if(false == RequestWiFiConnection() ){
    //We need to write the CSV to a file and log the error....
    return false;
  } else {

    String resp ="";
    if(true==obj->usesecure){
      resp = performRequest(clientptr, thingspeakHost, thingspeakUrl,443);
    } else {
      resp = performRequest(clientptr, thingspeakHost, thingspeakUrl);
    }
    ReleaseWiFiConnection();
    return (resp != "" && !resp.startsWith("0"));
  }

}

//request the specified url of the specified host
String ThinkspeakUpload::performRequest(WiFiClient* c , String host, String url, int port , String method , String headers , String data ) {
  
  Serial.println("Connecting to host '" + host + "' on port " + String(port));
  c->connect(host.c_str(), port); //default ports: http: port 80, https: 443
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

void ThinkspeakUpload::UploadTaskFnc(void* params){
  uint32_t WaitTime = portMAX_DELAY;
  TaskData_t* TaskData = (TaskData_t*) params;
  Serial.println("Thinkspeak UploadTask started");
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
    if( false == xSemaphoreTake( TaskData->CfgSem, WaitTime ) ){
      //No Configchange at all we can simpy upload the data 
      Serial.println("Thinkspeak: Prepare Upload ( insert code here )");
      PostData(TaskData->obj);
    } else {
      //We have a configchange 
      Serial.println("Thinkspeak: Config changed, apply settings");

    }


  }



}

uint8_t ThinkspeakUpload::GetMaxMappingChannels( void ){
  return ( sizeof(Mapping) / sizeof( Mapping[0] )  );
}

void ThinkspeakUpload::SetThinkspeakAPIKey( String ID ) {
  
  strncpy(Settings.ThingspealAPIKey, ID.c_str(), sizeof(Settings.ThingspealAPIKey));

}

void ThinkspeakUpload::SetThinkspeakEnable( bool Enable ) {

  Settings.Enabled=Enable;

}

void ThinkspeakUpload::SetThinkspeakUploadInterval( uint16_t Interval ) {

  Settings.UploadInterval=Interval;

}

void ThinkspeakUpload::SetMapping(uint8_t Channel, ThinkspeakMapping_t Map){
  if( Channel >= ( sizeof(Mapping) / sizeof( Mapping[0] )  )  ){
    Channel=0;
  }

  Mapping[Channel] = Map;


}

String ThinkspeakUpload::GetThinkspeakAPIKey( void ) {
  return String(Settings.ThingspealAPIKey);
}
bool ThinkspeakUpload::GetThinkspeakEnable( void ) {
  return  Settings.Enabled;

}
uint16_t ThinkspeakUpload::GetThinkspeakUploadInterval( void ) {
  return  Settings.UploadInterval;

}
ThinkspeakUpload::ThinkspeakMapping_t ThinkspeakUpload::GetMapping(uint8_t Channel ){
   if( Channel >= ( sizeof(Mapping) / sizeof( Mapping[0] )  )  ){
    Channel= ( sizeof(Mapping) / sizeof( Mapping[0] )  ) - 1;
  }

  return Mapping[Channel];

}














