//upload the sensor values to senseBox
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "sslCertificate.h"
#include "../../wifi_net.h"
#include "../../datastore.h"
#include "thingspeak.h"

#define DEBUG_SERIAL
/**************************************************************************************************
 *    Function      : ThinkspeakUpload
 *    Description   : Constructor
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
ThinkspeakUpload::ThinkspeakUpload( void ){
  //For the Upload we also use a mapping table but limit the amount of data currently to 16 entrys
  //For every entry we have a internal channel no and also a senseBoxID at hand ( hopfully )
}

/**************************************************************************************************
 *    Function      : ThinkspeakUpload
 *    Description   : Destructor
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
ThinkspeakUpload::~ThinkspeakUpload( void ){

}


/**************************************************************************************************
 *    Function      : begin
 *    Description   : Sets up the Thingspeak Task
 *    Input         : bool usehttps
 *    Output        : void
 *    Remarks       : Thingspeak don't linke HTTPS for now
 **************************************************************************************************/
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

/**************************************************************************************************
 *    Function      : InitConfig
 *    Description   : Reads the current config
 *    Input         : void
 *    Output        : void
 *    Remarks       : Read the config without starting any tasks
 **************************************************************************************************/
void ThinkspeakUpload::InitConfig(void){
   ReadMapping();
   ReadSettings();
}

/**************************************************************************************************
 *    Function      : RegisterDataAccess
 *    Description   : Register a function to access the mapped sensor values
 *    Input         : ThinkspeakUpload::DataAccesFnc Fnc
 *    Output        : void
 *    Remarks       : Thingspeak don't linke HTTPS for now
 **************************************************************************************************/
void ThinkspeakUpload::RegisterDataAccess(ThinkspeakUpload::DataAccesFnc Fnc){
  DaFnc = Fnc; //Register the pointer...
}

/**************************************************************************************************
 *    Function      : WriteMapping
 *    Description   : Writes the Mapping to a JSON file
 *    Input         : void
 *    Output        : void
 *    Remarks       : void
 **************************************************************************************************/
void ThinkspeakUpload::WriteMapping( void ){
  //This will just convert the Mapping to JSON and write it to the local filesystem
    String JSONData="";
    File file = SPIFFS.open("/ThingSpeakMapping.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);
    JsonArray ThingSpeakMapping = doc.createNestedArray("Mapping");
    for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
            JsonObject MappingObj = ThingSpeakMapping.createNestedObject();
            MappingObj["Enabled"] = Mapping[i].enable;
            MappingObj["Channel"] = Mapping[i].StationChannelIdx;            
    }
    serializeJson(doc, JSONData);
    file.print(JSONData);
    file.close();
    #ifdef DEBUG_SERIAL
    Serial.print("JSONData");
    Serial.println(JSONData);
    //We check the file content....
    file = SPIFFS.open("/ThingSpeakMapping.json");
    Serial.print("File content:");
    while(file.available()){
      char ch = file.read();
      Serial.print(ch);
    }
    file.close();
    Serial.println("End of File");
    #endif
    xSemaphoreGive(TaskData.CfgSem);

}


/**************************************************************************************************
 *    Function      : ReadMapping
 *    Description   : Read the Mapping from a JSON file
 *    Input         : void
 *    Output        : void
 *    Remarks       : void
 **************************************************************************************************/
void ThinkspeakUpload::ReadMapping( void ){
const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3) + 1580;
DynamicJsonDocument doc(capacity);
String JSONData="";        
//Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/ThingSpeakMapping.json")){
        File file = SPIFFS.open("/ThingSpeakMapping.json");
        #ifdef DEBUG_SERIAL
          Serial.print("Filesize:");
          Serial.print(file.available());
        #endif
          while(file.available()){
            char ch = file.read();
            #ifdef DEBUG_SERIAL
             Serial.print(ch);
            #endif
            JSONData=JSONData+ch;
          }
          #ifdef DEBUG_SERIAL
           Serial.println("");
           Serial.print("JSON:");
           Serial.println(JSONData);

          #endif
        DeserializationError err = deserializeJson(doc, JSONData);
        if(err) {
          #ifdef DEBUG_SERIAL
            Serial.print("ThingSpeakMapping ");
            Serial.print(F(" deserializeJson() failed with code "));
            Serial.println(err.c_str());         
          #endif
        } else {
          
        }
        JsonArray ThingspeakMapping = doc["Mapping"];
        if(ThingspeakMapping.isNull()==true){
          #ifdef DEBUG_SERIAL
            Serial.println("Thingspeak dezerilize failed");
          #endif
        }
        for(uint8_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] ));i++){

            Mapping[i].enable =ThingspeakMapping[i]["Enabled"];
            Mapping[i].StationChannelIdx = ThingspeakMapping[i]["Channel"];
            #ifdef DEBUG_SERIAL
              Serial.print("Read form File for Thingspeak Channel");
              Serial.println(i);
              Serial.print("Mapped to Station Channel:");
              Serial.println(Mapping[i].StationChannelIdx);
              if(Mapping[i].enable==true){
                Serial.println("Channel is Enabled");
              } else {
              Serial.println("Channel is Disabled");
              }
            #endif
        }
    } else {
        //We need to create a blank mapping scheme
        for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
              Mapping[i].enable=false;
              Mapping[i].StationChannelIdx=0;
         }
         #ifdef DEBUG_SERIAL
           Serial.println("ThingspeakUpload: Write blank config");
         #endif
         WriteMapping();
    }

}



/**************************************************************************************************
 *    Function      : WriteSettings
 *    Description   : Write the Settings to a JSON file
 *    Input         : void
 *    Output        : void
 *    Remarks       : void
 **************************************************************************************************/
void ThinkspeakUpload::WriteSettings(){
   #ifdef DEBUG_SERIAL
    Serial.println("Write to /ThingSpeakSetting.json");
   #endif
    File file = SPIFFS.open("/ThingSpeakSetting.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    doc["ThinkspeakAPIKey"] = String(Settings.ThingspealAPIKey); //SensboxID needed for the upload
    doc["UploadInterval"] = Settings.UploadInterval; //Interval in minutes for new data
    doc["Enabled"] = Settings.Enabled; //If the uplaod is enabled or not 
    serializeJson(doc, file);
    file.close();
    xSemaphoreGive(TaskData.CfgSem);

}


/**************************************************************************************************
 *    Function      : ReadSettings
 *    Description   : Read the Settings from a JSON file
 *    Input         : void
 *    Output        : void
 *    Remarks       : void
 **************************************************************************************************/
void ThinkspeakUpload::ReadSettings(){
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

      if(SPIFFS.exists("/ThingSpeakSetting.json")){
        File file = SPIFFS.open("/ThingSpeakSetting.json");
        DeserializationError err = deserializeJson(doc, file);
        if(err) {
          #ifdef DEBUG_SERIAL
            Serial.print("/ThingSpeakSetting.json ");
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
          #endif
        }
        
        
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
        file.close();

      } else {
        //We generate an empty file
         for(uint32_t i=0;i<sizeof(Settings.ThingspealAPIKey)-1;i++){
          Settings.ThingspealAPIKey[i]=0;
        }
        Settings.UploadInterval=1; //Interval in minutes for new data
        Settings.Enabled=false; //If the uplaod is enabled or not 
    
      }

      

}



/**************************************************************************************************
 *    Function      : PostData
 *    Description   : This will prepare data to be send and try to 
 *    Input         : ThinkspeakUpload* obj
 *    Output        : bool
 *    Remarks       : void
 **************************************************************************************************/
bool ThinkspeakUpload::PostData(  ThinkspeakUpload* obj ) {
  WiFiClient* clientptr = nullptr;
  String thingspeakApi = String(obj->Settings.ThingspealAPIKey);

  if(false == obj->Settings.Enabled){
    #ifdef DEBUG_SERIAL
      Serial.println("ThingSpeak upload disabled");
    #endif
    return false;
  }
  if (0 == obj->Settings.ThingspealAPIKey[0]) {
    #ifdef DEBUG_SERIAL
      Serial.println("ThingSpeak API Key not set");
    #endif
    return false;
  } 
  Serial.println("Start Uploading to ThingSpeak");
  
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
           thingspeakUrl += "&field" + String((i+1)) + "=" + String(value); 
           added_fields++;
        } else {
          //We have a mapping problem at all.....
          #ifdef DEBUG_SERIAL
            Serial.printf(" ThigSpeak upload: Requested Channel %i : No Mapped  for Stationchanne %i", i, obj->Mapping[i].StationChannelIdx);
          #endif
        }
      } else {
        #ifdef DEBUG_SERIAL
          Serial.printf(" ThingSpeak upload: Requested Channel %i : no DataSource", i);
        #endif
      }
        
    }

  }
  //This is for debug only
  #ifdef DEBUG_SERIAL
  Serial.println("Current URL String:");
  Serial.println(thingspeakUrl);
  Serial.println("# End of String #");
  //Sainity check if we have any data
  #endif
  if (0 ==added_fields) {
    #ifdef DEBUG_SERIAL
      Serial.println("ThingSpeak API: Channels all disabled or mapping broken");
    #endif
    return false;
  }
  
  #ifdef DEBUG_SERIAL
    Serial.println("Uploading to ThingSpeak");  
  #endif
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
      #ifdef DEBUG_SERIAL
        Serial.println("Try to use HTTPS@443 for Thingspeak");
      #endif
    } else {
      resp = performRequest(clientptr, thingspeakHost, thingspeakUrl,80);
      #ifdef DEBUG_SERIAL
        Serial.println("Try to use HTTP@80 for Thingspeak");
      #endif
    }
    ReleaseWiFiConnection();
    return (resp != "" && !resp.startsWith("0"));
  }

}

/**************************************************************************************************
 *    Function      : performRequest
 *    Description   : This will perfrom a request to thingspeak
 *    Input         : WiFiClient* c , String host, String url, int port , String method , String headers , String data
 *    Output        : String
 *    Remarks       : void
 **************************************************************************************************/
String ThinkspeakUpload::performRequest(WiFiClient* c , String host, String url, int port , String method , String headers , String data ) {
  #ifdef DEBUG_SERIAL
    Serial.println("Connecting to host '" + host + "' on port " + String(port));
  #endif
  c->connect(host.c_str(), port); //default ports: http: port 80, https: 443
  String request = method + " " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   headers + "\r\n";
  #ifdef DEBUG_SERIAL
    Serial.println("Requesting url: " + request);
  #endif
  c->print(request);
  if (data != "") {
    #ifdef DEBUG_SERIAL
      Serial.println("Data: " + data);
    #endif
    c->print(data + "\r\n");
  }
  
  unsigned long timeout = millis();
  while (c->available() == 0) {
    if (timeout + 5000 < millis()) {
    #ifdef DEBUG_SERIAL
      Serial.println("Client timeout");
    #endif
      c->stop();
      return "";
    }
    delay(1);
  }
  //read client reply
  String response;
  while(c->available()) {
    char data = c->read();
    response  = response + data;
    if(data=='\r'){
      break;
    }
    delay(1);

  }
  #ifdef DEBUG_SERIAL
    Serial.println("Response: " + response);
  #endif
  c->stop();
  return response;
}

/**************************************************************************************************
 *    Function      : UploadTaskFnc
 *    Description   : Task for the 
 *    Input         : void* params
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void ThinkspeakUpload::UploadTaskFnc(void* params){
  uint32_t WaitTime = portMAX_DELAY;
  TaskData_t* TaskData = (TaskData_t*) params;
  #ifdef DEBUG_SERIAL
    Serial.println("Thinkspeak UploadTask started");
  #endif
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
    #ifdef DEBUG_SERIAL
      Serial.print("Thingspeak will upload in ");
      Serial.print(WaitTime);
      Serial.println(" Ticks( ms )");
    #endif
    if( false == xSemaphoreTake( TaskData->CfgSem, WaitTime ) ){
      //No Configchange at all we can simpy upload the data 
      #ifdef DEBUG_SERIAL
        Serial.println("Thingspeak: Prepare Upload ( no config changes )");
      #endif
      PostData(TaskData->obj);
    } else {
      //We have a configchange 
      #ifdef DEBUG_SERIAL
        Serial.println("Thingspeak: Config changed, apply settings");
      #endif
    }


  }



}


/**************************************************************************************************
 *    Function      : GetMaxMappingChannels
 *    Description   : Returns the max amount of channels supported by this driver
 *    Input         : void
 *    Output        : uint8_t
 *    Remarks       : none
 **************************************************************************************************/
uint8_t ThinkspeakUpload::GetMaxMappingChannels( void ){
  return ( sizeof(Mapping) / sizeof( Mapping[0] )  );
}


/**************************************************************************************************
 *    Function      : SetThinkspeakAPIKey
 *    Description   : Sets the Key used for the Upload 
 *    Input         : ID
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void ThinkspeakUpload::SetThinkspeakAPIKey( String ID ) {
  
  strncpy(Settings.ThingspealAPIKey, ID.c_str(), sizeof(Settings.ThingspealAPIKey));
  WriteSettings();
}

/**************************************************************************************************
 *    Function      : SetThinkspeakEnable
 *    Description   : Enables or disbales the Upload
 *    Input         : bool
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void ThinkspeakUpload::SetThinkspeakEnable( bool Enable ) {

  Settings.Enabled=Enable;
  WriteSettings();

}

/**************************************************************************************************
 *    Function      : SetThinkspeakUploadInterval
 *    Description   : Sets the intervall in minutes for the upload
 *    Input         : bool
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void ThinkspeakUpload::SetThinkspeakUploadInterval( uint16_t Interval ) {

  Settings.UploadInterval=Interval;
  WriteSettings();

}


/**************************************************************************************************
 *    Function      : SetMapping
 *    Description   : Sets the mapping for one channel
 *    Input         : uint8_t Channel, ThinkspeakMapping_t Map
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void ThinkspeakUpload::SetMapping(uint8_t Channel, ThinkspeakMapping_t Map){
  if( Channel >= ( sizeof(Mapping) / sizeof( Mapping[0] )  )  ){
    Channel=0;
  }

  //We output the mappingdata gere.....

  Mapping[Channel] = Map;
  WriteMapping();

}

/**************************************************************************************************
 *    Function      : GetThinkspeakAPIKey
 *    Description   : Gets the used API Key
 *    Input         : void
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String ThinkspeakUpload::GetThinkspeakAPIKey( void ) {
  return String(Settings.ThingspealAPIKey);
}

/**************************************************************************************************
 *    Function      : GetThinkspeakEnable
 *    Description   : Returns if the Upload is enabled
 *    Input         : void
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
bool ThinkspeakUpload::GetThinkspeakEnable( void ) {
  return  Settings.Enabled;

}

/**************************************************************************************************
 *    Function      : GetThinkspeakUploadInterval
 *    Description   : Gets the current upload intervall
 *    Input         : void
 *    Output        : uint16_t
 *    Remarks       : none
 **************************************************************************************************/
uint16_t ThinkspeakUpload::GetThinkspeakUploadInterval( void ) {
  return  Settings.UploadInterval;

}


/**************************************************************************************************
 *    Function      : GetThinkspeakUploadInterval
 *    Description   : Gets the current mapping for one channel
 *    Input         : uint8_t Channel
 *    Output        : ThinkspeakUpload::ThinkspeakMapping_t
 *    Remarks       : none
 **************************************************************************************************/
ThinkspeakUpload::ThinkspeakMapping_t ThinkspeakUpload::GetMapping(uint8_t Channel ){
   if( Channel >= ( sizeof(Mapping) / sizeof( Mapping[0] )  )  ){
    Channel = ( sizeof(Mapping) / sizeof( Mapping[0] )  ) - 1;
  }
  #ifdef DEBUG_SERIAL
    Serial.print("Return Thingspeak Channel");
    Serial.println(Channel);
    Serial.print("Mapped to Station Channel:");
    Serial.println(Mapping[Channel].StationChannelIdx);
    if(Mapping[Channel].enable==true){
      Serial.print("Channel is Enabled");
    } else {
    Serial.print("Channel is Disabled");
    }
  #endif
  return Mapping[Channel];

}
#ifdef DEBUG_SERIAL
 #undef DEBUG_SERIAL
#endif














