#include <ArduinoJson.h>
#include "datastore.h"
#include "./src/MQTTConnector/mqtt_task.h"
#include "webserver_mqtt_fnc.h"

static WebServer * MQTTWebserver=nullptr;

static void request_mqttsetting( void );
static void mqttsettings_update( void );

void Webserver_MQTT_FunctionsRegister(WebServer* server){
    if(server == nullptr){
        return;
    }
    MQTTWebserver = server;
    server->on("/mqtt/settings",HTTP_POST,mqttsettings_update);
    server->on("/mqtt/settings",HTTP_GET, request_mqttsetting);
}

void mqttsettings_update( ){
  WebServer* server = MQTTWebserver;
  if(server == nullptr){
      return;
  }
  mqttsettings_t Data = read_mqttsettings();
   
  if( ! server->hasArg("MQTT_USER") || server->arg("MQTT_USER") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_USER");
    bzero(Data.mqttusername,sizeof(Data.mqttusername));
    strncpy(Data.mqttusername, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_PASS") || server->arg("MQTT_PASS") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_PASS");
    bzero(Data.mqttpassword,sizeof(Data.mqttpassword));
    strncpy(Data.mqttpassword, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_SERVER") || server->arg("MQTT_SERVER") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_SERVER");
    bzero(Data.mqttservername,sizeof(Data.mqttservername));
    strncpy(Data.mqttservername, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_HOST") || server->arg("MQTT_HOST") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_HOST");
    bzero(Data.mqtthostname,sizeof(Data.mqtthostname));
    strncpy(Data.mqtthostname, value.c_str(),64);
  }

  if( ! server->hasArg("MQTT_PORT") || server->arg("MQTT_PORT") == NULL ) { 
    /* we are missong something here */
  } else { 
    int32_t value = server->arg("MQTT_PORT").toInt();
    if( (value>=0) && ( value<=UINT16_MAX ) ){
      Data.mqttserverport = value;
    }
  }
  
  if( ! server->hasArg("MQTT_TOPIC") || server->arg("MQTT_TOPIC") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_TOPIC");
    bzero(Data.mqtttopic,sizeof(Data.mqtttopic));
    strncpy(Data.mqtttopic, value.c_str(),500);
  }

  if( ! server->hasArg("MQTT_ENA") || server->arg("MQTT_ENA") == NULL ) { 
    /* we are missing something here */
  } else { 
    bool value = false;
    if(server->arg("MQTT_ENA")=="true"){
      value = true;
    }
    Data.enable = value;
  }

  if( ! server->hasArg("MQTT_TXINTERVALL") || server->arg("MQTT_TXINTERVALL") == NULL ) { 
    /* we are missong something here */
  } else { 
    uint32_t value = server->arg("MQTT_TXINTERVALL").toInt();
    Data.mqtttxintervall = value;
  }

  if( ! server->hasArg("MQTT_IOBROKER") || server->arg("MQTT_IOBROKER") == NULL ) { 
    /* we are missing something here */
  } else { 
    bool value = false;
    if(server->arg("MQTT_IOBROKER")=="true"){
      value = true;
    }
    Data.useIoBrokerMsgStyle = value;
  }
  
  /* write data to the eeprom */
  write_mqttsettings(Data);
  TaskHandle_t MQTTTaskHandle = GetMQTTTaskHandle();
  if(MQTTTaskHandle!=nullptr){
    xTaskNotify( MQTTTaskHandle, 0x01, eSetBits );
  } else {
    Serial.println("MQTTTask Handle is null");
  }
  server->send(200); 

}


/*
  char mqttservename[129];
  uint16_t mqttserverport;
  char mqttusername[129];
  char mqttpassword[129];
  char mqtttopic[501];
  char mqtthostname[65];
 */
void request_mqttsetting(){
  WebServer* server = MQTTWebserver; 
  if(server == nullptr){
      return;
  }
  String response ="";
  mqttsettings_t Data = read_mqttsettings();
  DynamicJsonDocument root(2048);
  root["mqttena"]= (bool)(Data.enable);
  root["mqttserver"] = String(Data.mqttservername);
  root["mqtthost"] = String(Data.mqtthostname);
  root["mqttport"] = Data.mqttserverport;
  root["mqttuser"] = String(Data.mqttusername);
  root["mqtttopic"] = String(Data.mqtttopic);
  root["mqtttxintervall"] = Data.mqtttxintervall;
  root["mqtte_iobrokermode"] = Data.useIoBrokerMsgStyle;
  if(Data.mqttpassword[0]!=0){
    root["mqttpass"] = "********";
  } else {
    root["mqttpass"] ="";
  }
  serializeJson(root, response);
  server->send(200, "text/plain", response);

}