#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "./src/ValueMapping/Valuemapping.h"

//requiered for timekeeping, old but working 
#include "./src/TimeCore/timecore.h"
#include "datastore.h"
//
#include "webserver_fnc.h"


extern VALUEMAPPING SensorMapping;

static  WebServer* server = nullptr;

void response_mappingdata( void );
void response_supportedsensors( void );
void response_connectedsensors( void );
void process_setmapping( void );

void timesettings_send( void );
void settime_update(void );
void ntp_settings_update( void );
void timezone_update( void );
void timezone_overrides_update( void );

void WebserverFunctionsRegister( WebServer* serverptr){
server=serverptr;

// server->on("/notes.dat",HTTP_GET,read_notes);
// server->on("/notes.dat",HTTP_POST,update_notes);

// server->on("/mqtt/settings",HTTP_POST,mqttsettings_update);
// server->on("/mqtt/settings",HTTP_GET,read_mqttsetting);

server->on("/mapping/mappingdata.json", HTTP_GET, response_mappingdata);
server->on("/devices/supportedsensors.json",HTTP_GET,response_supportedsensors);
server->on("/devices/connectedsensors.json",HTTP_GET,response_connectedsensors);
server->on("/mapping/set",HTTP_POST,process_setmapping ); //Sets a single mapping for one channel

//As we have a kind of time service with NTP arround we also need to reimplement the functions to hanlde it

 server->on("/timesettings", HTTP_GET, timesettings_send);
 server->on("/settime.dat", HTTP_POST, settime_update); /* needs to process date and time */
 server->on("/ntp.dat",HTTP_POST,ntp_settings_update); /* needs to process NTP_ON, NTPServerName and NTP_UPDTAE_SPAN */
 server->on("/timezone.dat",timezone_update); /*needs to handel timezoneid */
 server->on("/overrides.dat",timezone_overrides_update); /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */



}

/* This sectionion if for the time settings  */


/**************************************************************************************************
*    Function      : timesettings_send
*    Description   : Sends the timesettings as json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void timesettings_send(){
StaticJsonDocument<350> root;
char strbuffer[129];
String response="";  
  
  memset(strbuffer,0,129);
  datum_t d = timec.GetLocalTimeDate();
  snprintf(strbuffer,64,"%02d:%02d:%02d",d.hour,d.minute,d.second);
  
  root["time"] = strbuffer;
 
  memset(strbuffer,0,129);
  snprintf(strbuffer,64,"%04d-%02d-%02d",d.year,d.month,d.day);
  root["date"] = strbuffer;


  root["tzidx"] = (int32_t)timec.GetTimeZone();
  root["gpsena"] = true;
  root["zoneoverride"]=timec.GetTimeZoneManual();;
  root["gmtoffset"]=timec.GetGMT_Offset();;
  root["dlsdis"]=!timec.GetAutomacitDLS();
  root["dlsmanena"]=timec.GetManualDLSEna();
  uint32_t idx = timec.GetDLS_Offset();
  root["dlsmanidx"]=idx;
  root["gps_sync"]=gps_config.sync_on_gps;
  serializeJson(root, response);
  sendData(response);
}


/**************************************************************************************************
*    Function      : settime_update
*    Description   : Parses POST for new local time
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void settime_update( ){ /* needs to process date and time */
  datum_t d;
  d.year=2000;
  d.month=1;
  d.day=1;
  d.hour=0;
  d.minute=0;
  d.second=0;

  bool time_found=false;
  bool date_found=false;
  
  if( ! server->hasArg("date") || server->arg("date") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
   
    Serial.printf("found date: %s\n\r",server->arg("date").c_str());
    uint8_t d_len = server->arg("date").length();
    Serial.printf("datelen: %i\n\r",d_len);
    if(server->arg("date").length()!=10){
      Serial.println("date len failed");
    } else {   
      String year=server->arg("date").substring(0,4);
      String month=server->arg("date").substring(5,7);
      String day=server->arg("date").substring(8,10);
      d.year = year.toInt();
      d.month = month.toInt();
      d.day = day.toInt();
      date_found=true;
    }   
  }

  if( ! server->hasArg("time") || server->arg("time") == NULL ) { // If the POST request doesn't have username and password data
    
  } else {
    if(server->arg("time").length()!=8){
      Serial.println("time len failed");
    } else {
    
      String hour=server->arg("time").substring(0,2);
      String minute=server->arg("time").substring(3,5);
      String second=server->arg("time").substring(6,8);
      d.hour = hour.toInt();
      d.minute = minute.toInt();
      d.second = second.toInt();     
      time_found=true;
    }
     
  } 
  if( (time_found==true) && ( date_found==true) ){
    Serial.printf("Date: %i, %i, %i ", d.year , d.month, d.day );
    Serial.printf("Time: %i, %i, %i ", d.hour , d.minute, d.second );
    timec.SetLocalTime(d);
  }
  
  server->send(200);   
 
 }


/**************************************************************************************************
*    Function      : timezone_update
*    Description   : Parses POST for new timezone settings
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
void timezone_update( ){ /*needs to handel timezoneid */
  if( ! server->hasArg("timezoneid") || server->arg("timezoneid") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
   
    Serial.printf("New TimeZoneID: %s\n\r",server->arg("timezoneid").c_str());
    uint32_t timezoneid = server->arg("timezoneid").toInt();
    timec.SetTimeZone( (TIMEZONES_NAMES_t)timezoneid );   
  }
  timec.SaveConfig();
  server->send(200);    

 }

 
/**************************************************************************************************
*    Function      : timezone_overrides_update
*    Description   : Parses POST for new timzone overrides
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
 void timezone_overrides_update( ){ /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */

  bool DLSOverrid=false;
  bool ManualDLS = false;
  bool ZONE_OVERRRIDE = false;
  int32_t gmt_offset = 0;
  DLTS_OFFSET_t dls_offsetidx = DLST_OFFSET_0;
  if( ! server->hasArg("dlsdis") || server->arg("dlsdis") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    DLSOverrid=true;  
  }

  if( ! server->hasArg("dlsmanena") || server->arg("dlsmanena") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ManualDLS=true;  
  }

  if( ! server->hasArg("ZONE_OVERRRIDE") || server->arg("ZONE_OVERRRIDE") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ZONE_OVERRRIDE=true;  
  }

  if( ! server->hasArg("gmtoffset") || server->arg("gmtoffset") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    gmt_offset = server->arg("gmtoffset").toInt();
  }

  if( ! server->hasArg("dlsmanidx") || server->arg("dlsmanidx") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    dls_offsetidx = (DLTS_OFFSET_t) server->arg("dlsmanidx").toInt();
  }
  timec.SetGMT_Offset(gmt_offset);
  timec.SetDLS_Offset( (DLTS_OFFSET_t)(dls_offsetidx) );
  timec.SetAutomaticDLS(!DLSOverrid);
  timec.SetManualDLSEna(ManualDLS);
  timec.SetTimeZoneManual(ZONE_OVERRRIDE);

 
  timec.SaveConfig();
  server->send(200);    

  
 }



/* ------------------------------------------- */

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
            MappingObj["Connected"] = 
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
            MappingObj["Connected"] = 
            MappingObj["Name"]= SensorMapping.GetSensorName( SensorList[i]);
    }
    serializeJson(doc, data);
    server->send(200, "text/plain", data);
}