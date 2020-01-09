#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "../ValueMapping/ValueMapping.h"

/* on header as last one */
#include "mqtt_task.h"


#define hourMs 3600000 //ms (60 * 60 * 1000 ms)

//We need to check if even MQTT Secure is possible
WiFiClient espClient;                           // WiFi ESP Client  
PubSubClient mqttclient(espClient);             // MQTT Client 
TaskHandle_t MQTTTaskHandle = NULL;
VALUEMAPPING* Mapping=nullptr;
typedef enum {
  vt_u8=0,
  vt_i8,
  vt_u16,
  vt_i16,
  vt_u32,
  vt_i32,
  vt_flt,
  vt_dbl,
  vt_bool,
  vt_cnt
} valuetype_t;

typedef union  {
   uint8_t u8;
   int8_t i8;
   uint16_t u16;
   int16_t i16;
   uint32_t u32;
   int32_t i32;
   float flt;
   double dbl;
   bool bl;
} MQTT_Value_un;

typedef struct{
    MQTT_Value_un Value;
    valuetype_t Type;
}MQTT_Value_t;

void callback(char* topic, byte* payload, unsigned int length);
void SendIoBrokerSingleMSG(mqttsettings_t* settings,const char* subtopic, const MQTT_Value_t value);
void MQTT_Task( void* prarm );
void MQTTRegisterMappingAccess(VALUEMAPPING* Mp){
Mapping = Mp;
}

void MQTTTaskStart( void ){
/* This will created the MQTT task pinned to core 1 with prio 1 */
   xTaskCreatePinnedToCore(
   MQTT_Task,
   "MQTT_Task",
   10000,
   NULL,
   1,
   &MQTTTaskHandle,
   1);

}

mqttsettings_t GetMQTTSettings( void ){
  return read_mqttsettings();
}

void MQTT_Task( void* prarm ){
   DynamicJsonDocument  root(4096);
   String JsonString = "";
   uint32_t ulNotificationValue;
   int32_t last_message = millis();
   mqttsettings_t Settings = read_mqttsettings();
                         
   Serial.println("MQTT Thread Start");
   mqttclient.setCallback(callback);             // define Callback function
   bool IOBrokerMode = false;
   while(1==1){

      /* if settings have changed we need to inform this task that a reload and reconnect is requiered */ 
      if(Settings.enable != false){
        ulNotificationValue = ulTaskNotifyTake( pdTRUE, 0 );
      } else {
        Serial.println("MQTT disabled, going to sleep");
        if(true == mqttclient.connected() ){
            mqttclient.disconnect();
        }
        ulNotificationValue = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        Serial.println("MQTT awake from sleep");
      }

   if( ulNotificationValue&0x01 != 0 ){
      Serial.println("Reload MQTT Settings");
      /* we need to reload the settings and do a reconnect */
      if(true == mqttclient.connected() ){
        mqttclient.disconnect();
      }
      Settings = read_mqttsettings();
   }

   if(Settings.enable != false ) {

       if(!mqttclient.connected()) {             
            /* sainity check */
            if( (Settings.mqttserverport!=0) && (Settings.mqttservername[0]!=0) && ( Settings.enable != false ) ){
                  /* We try only every second to connect */
                  Serial.print("Connecting to MQTT...");  // connect to MQTT
                  mqttclient.setServer(Settings.mqttservername, Settings.mqttserverport); // Init MQTT     
                  if (mqttclient.connect(Settings.mqtthostname, Settings.mqttusername, Settings.mqttpassword)) {
                    Serial.println("connected");          // successfull connected  
                    mqttclient.subscribe(Settings.mqtttopic);             // subscibe MQTT Topic
                  } else {
                    Serial.println("failed");   // MQTT not connected       
                  }
            }
       } else{
            mqttclient.loop();                            // loop on client
            /* Check if we need to send data to the MQTT Topic, currently hardcode intervall */
            uint32_t intervall_end = last_message +( Settings.mqtttxintervall * 1000 );
            if( ( Settings.mqtttxintervall > 0) && ( intervall_end  <  millis() ) ){
              last_message=millis();

              if( false == IOBrokerMode)//if(false == Settings.useIoBrokerMsgStyle)
              {
              /* if we run in json mode we need to bulld the object */
                if(Mapping != nullptr){
                  Serial.println("Send JSON Payload");  
                  JsonString="";
                  root.clear();
                  JsonArray data = root.createNestedArray("data");
                  for(uint8_t i=0;i<64;i++){
                      float value = NAN;
                      if(false == Mapping->ReadMappedValue(&value,i)){
                          Serial.printf("MQTT Channel %i not mapped\n\r",i);                        
                      } else {
                        Serial.printf("MQTT Channel %i Value %f",i,value );
                        String name = Mapping->GetSensorNameByChannel(i);
                        Serial.println(name);
                        JsonObject dataobj = data.createNestedObject();
                        dataobj["channel"] = i;
                        dataobj["value"] = value;
                        dataobj["name"] = name;

                      }
                  }
                  
                  /* Every minute we send a new set of data to the mqtt channel */
                  serializeJson(root,JsonString);
                  if ( 0 == mqttclient.publish(Settings.mqtttopic, JsonString.c_str())){
                      Serial.println("MQTT pub failed");  
                  }
                }

                } else /* If we run in IO Broker mode */ {
                if(Mapping != nullptr){
                  Serial.println("Send for IOBroker");  
                  MQTT_Value_t Tx_Value ;
                  Tx_Value.Type=vt_flt;
                    for(uint8_t i=0;i<64;i++){
                      float value = NAN;
                      if(false == Mapping->ReadMappedValue(&value,i)){
                          Serial.printf("MQTT Channel %i not mapped\n\r",i);                        
                      } else {
                          Tx_Value.Value.flt = value;
                          String ChannelName = "/Weather/Channel"+String(i);
                          SendIoBrokerSingleMSG(&Settings,ChannelName.c_str(),Tx_Value);
                          mqttclient.loop();                            // loop on client
                      }
                }
              }
              
            }
       }
      
   } 
  }
  vTaskDelay( 100/portTICK_PERIOD_MS );
 }
}

/***************************
 * callback - MQTT message
 ***************************/
void callback(char* topic, byte* payload, unsigned int length) {

}

void SendIoBrokerSingleMSG(mqttsettings_t* settings,const char* subtopic, const MQTT_Value_t value) {
  /* Topic can be up to 500 cahrs long and we need also to add a 
   * name for each subtopic 
   */
  char mqttcombinedtopic[1024] = {0,};
  char valuestr[64]={0,};
  int16_t error = 0;
  switch( value.Type){
      case vt_u8:{
        error = snprintf(valuestr,sizeof(valuestr),"%u",value.Value.u8);
      } break;

      case vt_i8:{
        error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.u8);
      } break;

      case vt_u16:{
      error = snprintf(valuestr,sizeof(valuestr),"%u",value.Value.u8);
      } break;

    case vt_i16:{
      error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.u8);
      } break;

    case vt_u32:{
      error = snprintf(valuestr,sizeof(valuestr),"%u",value.Value.u8);
      } break;

    case vt_i32:{
        error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.dbl);
      } break;

    case vt_flt:{
        error = snprintf(valuestr,sizeof(valuestr),"%f",value.Value.flt);
      } break;

    case vt_dbl:{
        error = snprintf(valuestr,sizeof(valuestr),"%lf",value.Value.dbl);
    } break;

    case vt_bool:{
      if(true != value.Value.bl){
          strncpy (valuestr,"false", sizeof(valuestr));
      } else {
          strncpy (valuestr,"true",sizeof(valuestr));
      }
    }break;

    default:{
        error = -66;
    } break;

  }
  
  if( (error < 0)  || ( error > sizeof(valuestr ) ) ) {
    /* value is truncated ! */
    Serial.printf("value string error: %i\n\r",error );  
  } else {
    strncpy(mqttcombinedtopic, settings->mqtttopic, 1023);
    /* This is 'save' as if we have a size of zero the compile will / shall fail */
    mqttcombinedtopic[sizeof(mqttcombinedtopic)-1]=0;
    uint32_t len = strnlen( mqttcombinedtopic,sizeof(mqttcombinedtopic) );
    /* We need to get the lenght of the current string to limit the strncoy*/
    strncat(mqttcombinedtopic, subtopic, sizeof(mqttcombinedtopic)-len );
    if ( 0 == mqttclient.publish(mqttcombinedtopic,valuestr, true) ){ 
        Serial.println("MQTT pub failed");  
    } else {
         Serial.print("Published in "); 
         Serial.print(mqttcombinedtopic);
         Serial.print(" the Message:");
         Serial.println( valuestr );
    }
  }
}