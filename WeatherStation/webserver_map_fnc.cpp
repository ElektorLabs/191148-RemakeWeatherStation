#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "./src/ValueMapping/Valuemapping.h"
#include "webserver_map_fnc.h"


extern VALUEMAPPING SensorMapping;

static  WebServer* server = nullptr;

void response_mappingdata( void );
void response_supportedsensors( void );
void response_connectedsensors( void );
void process_setmapping( void );

void Webserver_Map_FunctionsRegister( WebServer* serverptr){
server=serverptr;
server->on("/mapping/mappingdata.json", HTTP_GET, response_mappingdata);
server->on("/devices/supportedsensors.json",HTTP_GET,response_supportedsensors);
server->on("/devices/connectedsensors.json",HTTP_GET,response_connectedsensors);
server->on("/mapping/set",HTTP_POST,process_setmapping );

}

void process_setmapping( void ){
//We need here the element eqivalent for the mapping
//Also we make sure that this is a valid element
bool completedata = true;

int32_t iBus=0;
int32_t iMappedChannel=0;
int32_t iValueType=0;
int32_t iValueChannel=0;


  if( ! server->hasArg("MAPPEDCHANNEL") || server->arg("MAPPEDCHANNEL") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
    completedata = completedata & false;
  } else {
     iMappedChannel = server->arg("MAPPEDCHANNEL").toInt();
  }

  if( ! server->hasArg("BUS") || server->arg("BUS") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
    completedata = completedata & false;
  } else {
     iBus= server->arg("BUS").toInt();
    
  }

  if( ! server->hasArg("MESSURMENTVALTYPE") || server->arg("MESSURMENTVALTYPE") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
    completedata = completedata & false;
  } else {
     iValueType = server->arg("MESSURMENTVALTYPE").toInt();
  }

  if( ! server->hasArg("VALCHANNEL") || server->arg("VALCHANNEL") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
    completedata = completedata & false;
  } else {
    iValueChannel = server->arg("VALCHANNEL").toInt();
  }

  if(true == completedata){
      //SaintyCheck
      bool fault = false;
      if( (iBus<0) || ( iBus>=VALUEMAPPING::SensorBus_t::SENSORBUS_CNT) ){
          //Out of range
          Serial.println("iBus out of range");
          fault=true;
      }

      if( ( iValueType<0 ) || (iValueType >= DATAUNITS::MessurmentValueType_t::MESSUREMENTVALUE_CNT) ){
          //Out of range
          Serial.println("iValueType out of range");
          fault=true;
      }

      if( (iValueChannel<0) || (iValueChannel>255) ){
          //Out of range
          Serial.println("iValueChannel out of range");
          fault=true;
      }

      if( ( iMappedChannel <0) || (iMappedChannel>=64) ){
          Serial.println("iMappedChannel out of range");
          fault=true;
      }

      if(false == fault ){
        //We can try to buld the element
        /*
        Serial.println("Data Received");
        Serial.printf("iBus=%i ,",iBus);
        Serial.printf("iValueType=%i ,",iValueType);
        Serial.printf("iValueChannel=%i ,",iValueChannel);
        Serial.printf("iMappedChannel=%i \n\r",iMappedChannel);
        */
        VALUEMAPPING::SensorElementEntry_t Element;
        Element.Bus = (VALUEMAPPING::SensorBus_t)(iBus);
        Element.ValueType = (DATAUNITS::MessurmentValueType_t )(iValueType);
        Element.ChannelIDX = (uint8_t)iValueChannel;
        SensorMapping.SetMappingForChannel(iMappedChannel, Element);
        //Serial.println("Upadte Mapping");
      } else {

      }

  } else {

  }

  server->send(200); 

}


void response_mappingdata( void ){
    if(SPIFFS.exists("/mapping.json")){
      File file = SPIFFS.open("/mapping.json", "r");
      server->streamFile(file, "text/plain");
      file.close();
    } else {
      String data="{}";
      server->send(200, "text/plain", data);
    }
}


void response_connectedsensors( void ){
    //We need to collect a whole buch of data for this one......
    VALUEMAPPING::SensorElementEntry_t SensorList[64];
    uint8_t SensorCount = SensorMapping.GetConnectedSensors(SensorList,64);
    Serial.printf("Sensors supported: %i \n\r", SensorCount);
    String data="";
    /*
          SensorBus_t Bus;
            DATAUNITS::MessurmentValueType_t ValueType;
            uint8_t ChannelIDX;
    */
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    
    JsonArray Mapping = doc.createNestedArray("SensorList");
    for(uint32_t i=0;i<( SensorCount  );i++){
            JsonObject MappingObj = Mapping.createNestedObject();
            MappingObj["Bus"] = (uint8_t)(SensorList[i].Bus);
            MappingObj["ValueType"] = (uint8_t)(SensorList[i].ValueType);
            MappingObj["Channel"] = (uint8_t)(SensorList[i].ChannelIDX);
            MappingObj["Name"]= SensorMapping.GetSensorName( SensorList[i]);
    }
    serializeJson(doc, data);
    server->send(200, "text/plain", data);
}

void response_supportedsensors( void ){
    //We need to collect a whole buch of data for this one......
    VALUEMAPPING::SensorElementEntry_t SensorList[64];
    uint8_t SensorCount = SensorMapping.GetSensors(SensorList,64);
    Serial.printf("Sensors supported: %i \n\r", SensorCount);
    String data="";
    /*
          SensorBus_t Bus;
            DATAUNITS::MessurmentValueType_t ValueType;
            uint8_t ChannelIDX;
    */
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    
    JsonArray Mapping = doc.createNestedArray("SensorList");
    for(uint32_t i=0;i<( SensorCount  );i++){
            JsonObject MappingObj = Mapping.createNestedObject();
            MappingObj["Bus"] = (uint8_t)(SensorList[i].Bus);
            MappingObj["ValueType"] = (uint8_t)(SensorList[i].ValueType);
            MappingObj["Channel"] = (uint8_t)(SensorList[i].ChannelIDX); 
            MappingObj["Name"]= SensorMapping.GetSensorName( SensorList[i]);
    }
    serializeJson(doc, data);
    server->send(200, "text/plain", data);
}