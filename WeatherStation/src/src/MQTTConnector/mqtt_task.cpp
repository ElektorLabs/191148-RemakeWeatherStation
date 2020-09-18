#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "../ValueMapping/ValueMapping.h"
#include "../../datastore.h" //May this also is moved to a file in JSON format on SPIFFS
#include "../../version.h"


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


/**************************************************************************************************
 *    Function      : callback
 *    Description   : none
 *    Input         : char* topic, byte* payload, unsigned int length
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void callback(char* topic, byte* payload, unsigned int length);


/**************************************************************************************************
 *    Function      : SendIoBrokerSingleMSG
 *    Description   : Will send messages compatibel with the ioBroker 
 *    Input         : void* prarm 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void SendIoBrokerSingleMSG(mqttsettings_t* settings,const char* subtopic, const MQTT_Value_t value);

void MQTT_Task( void* prarm );

/**************************************************************************************************
 *    Function      : MQTTRegisterMappingAccess
 *    Description   : This registers a function to get access to the mapped values
 *    Input         : VALUEMAPPING* 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void MQTTRegisterMappingAccess(VALUEMAPPING* Mp){
  Mapping = Mp;
}

/**************************************************************************************************
 *    Function      : MQTTTaskStart
 *    Description   : This will start the MQTT Task
 *    Input         : void 
 *    Output        : TaskHandle_t
 *    Remarks       : none
 **************************************************************************************************/
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

/**************************************************************************************************
 *    Function      : GetMQTTTaskHandle
 *    Description   : This will read the MQTT Handle
 *    Input         : void 
 *    Output        : TaskHandle_t
 *    Remarks       : none
 **************************************************************************************************/
TaskHandle_t GetMQTTTaskHandle( void ){
   return MQTTTaskHandle;
}

/**************************************************************************************************
 *    Function      : GetMQTTSettings
 *    Description   : This will read the MQTT Settings from datastore
 *    Input         : void* prarm 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
mqttsettings_t GetMQTTSettings( void ){
  return read_mqttsettings();
}

/**************************************************************************************************
 *    Function      : MQTT_Task
 *    Description   : Task to handle MQTT 
 *    Input         : void* prarm 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void MQTT_Task( void* prarm ){
   DynamicJsonDocument  root(4096);
   String JsonString = "";
   uint32_t ulNotificationValue;
   int32_t last_message = millis();
   mqttsettings_t Settings = read_mqttsettings();
   #ifdef DEBUG_SERIAL                      
    Serial.println("MQTT Thread Start");
   #endif
   mqttclient.setCallback(callback);             // define Callback function
   bool IOBrokerMode = false;
   while(1==1){
       vTaskDelay( 100/portTICK_PERIOD_MS );
      /* if settings have changed we need to inform this task that a reload and reconnect is requiered */ 
      if(Settings.enable != false){
        ulNotificationValue = ulTaskNotifyTake( pdTRUE, 0 );
      } else {
        #ifdef DEBUG_SERIAL
          Serial.println("MQTT disabled, going to sleep");
        #endif
        if(true == mqttclient.connected() ){
            mqttclient.disconnect();
        }
        ulNotificationValue = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        #ifdef DEBUG_SERIAL
          Serial.println("MQTT awake from sleep");
        #endif
      }

   if( (ulNotificationValue&0x01) != 0 ){
      #ifdef DEBUG_SERIAL
        Serial.println("Reload MQTT Settings");
      #endif
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
                  #ifdef DEBUG_SERIAL
                    Serial.print("Connecting to MQTT...");  // connect to MQTT
                  #endif
                  mqttclient.setServer(Settings.mqttservername, Settings.mqttserverport); // Init MQTT     
                  if (mqttclient.connect(Settings.mqtthostname, Settings.mqttusername, Settings.mqttpassword)) {
                    #ifdef DEBUG_SERIAL
                      Serial.println("connected");          // successfull connected  
                    #endif
                    mqttclient.subscribe(Settings.mqtttopic);             // subscibe MQTT Topic
                  } else {
                     #ifdef DEBUG_SERIAL
                      Serial.println("failed");   // MQTT not connected       
                    #endif
                  }
            }
            mqttclient.loop();                            // loop on client
       } else{
            mqttclient.loop();                            // loop on client
            IOBrokerMode = Settings.useIoBrokerMsgStyle;
            /* Check if we need to send data to the MQTT Topic, currently hardcode intervall */
            uint32_t intervall_end = last_message +( Settings.mqtttxintervall * 1000 * 60 );
            if( ( Settings.mqtttxintervall > 0) && ( intervall_end  <  millis() ) ){
              last_message=millis();

              if( false == IOBrokerMode)//if(false == Settings.useIoBrokerMsgStyle)
              {
              /* if we run in json mode we need to bulld the object */
                if(Mapping != nullptr){
                  #ifdef DEBUG_SERIAL
                    Serial.println("Send JSON Payload");  
                  #endif
                  JsonString="";
                  root.clear();
                  JsonArray data = root.createNestedArray("data");
                  for(uint8_t i=0;i<64;i++){
                      float value = NAN;
                      if(false == Mapping->ReadMappedValue(&value,i)){
                          #ifdef DEBUG_SERIAL
                            Serial.printf("MQTT Channel %i not mapped\n\r",i);
                          #endif                        
                      } else {
                        String name = Mapping->GetSensorNameByChannel(i);
                        #ifdef DEBUG_SERIAL
                          Serial.printf("MQTT Channel %i Value %f , Name:",i,value );
                          Serial.println(name);
                        #endif
                        JsonObject dataobj = data.createNestedObject();
                        dataobj["channel"] = i;
                        dataobj["value"] = value;
                        dataobj["name"] = name;

                      }
                  }
                  
                  /* Every minute we send a new set of data to the mqtt channel */
                  serializeJson(root,JsonString);
                  if ( 0 == mqttclient.publish(Settings.mqtttopic, JsonString.c_str())){
                      #ifdef DEBUG_SERIAL
                        Serial.println("MQTT pub failed"); 
                      #endif 
                  }
                } else {
                   #ifdef DEBUG_SERIAL
                        Serial.println("MQTT not registred to Mapper"); 
                    #endif 
                }

                } else /* If we run in IO Broker mode */ {
                if(Mapping != nullptr){
                  #ifdef DEBUG_SERIAL
                    Serial.println("Send for IOBroker");  
                  #endif
                  MQTT_Value_t Tx_Value ;
                  Tx_Value.Type=vt_flt;
                    for(uint8_t i=0;i<64;i++){
                      float value = NAN;
                      if(false == Mapping->ReadMappedValue(&value,i)){
                            #ifdef DEBUG_SERIAL
                             Serial.printf("MQTT Channel %i not mapped\n\r",i);
                            #endif                        
                      } else {
                          Tx_Value.Value.flt = value;
                          String ChannelName = "/Weather/Channel"+String(i);
                          SendIoBrokerSingleMSG(&Settings,ChannelName.c_str(),Tx_Value);
                          mqttclient.loop();                            // loop on client
                      }
                }
              } else {
                   #ifdef DEBUG_SERIAL
                        Serial.println("MQTT not registred to Mapper"); 
                    #endif 
              }
              
            }
       }
      
   } 
  }
 }
}



/**************************************************************************************************
 *    Function      : SendIoBrokerSingleMSG
 *    Description   : Will send messages compatibel with the ioBroker 
 *    Input         : void* prarm 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
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
        error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.i8);
      } break;

      case vt_u16:{
      error = snprintf(valuestr,sizeof(valuestr),"%u",value.Value.u16);
      } break;

    case vt_i16:{
      error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.i16);
      } break;

    case vt_u32:{
      error = snprintf(valuestr,sizeof(valuestr),"%u",value.Value.u32);
      } break;

    case vt_i32:{
        error = snprintf(valuestr,sizeof(valuestr),"%i",value.Value.i32);
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
    #ifdef DEBUG_SERIAL
      Serial.printf("value string error: %i\n\r",error );
    #endif  
  } else {
    strncpy(mqttcombinedtopic, settings->mqtttopic, 1023);
    /* This is 'save' as if we have a size of zero the compile will / shall fail */
    mqttcombinedtopic[sizeof(mqttcombinedtopic)-1]=0;
    uint32_t len = strnlen( mqttcombinedtopic,sizeof(mqttcombinedtopic) );
    /* We need to get the lenght of the current string to limit the strncoy*/
    strncat(mqttcombinedtopic, subtopic, sizeof(mqttcombinedtopic)-len );
    if ( 0 == mqttclient.publish(mqttcombinedtopic,valuestr, true) ){ 
        #ifdef DEBUG_SERIAL
          Serial.println("MQTT pub failed");
        #endif  
    } else {
        #ifdef DEBUG_SERIAL
         Serial.print("Published in "); 
         Serial.print(mqttcombinedtopic);
         Serial.print(" the Message:");
         Serial.println( valuestr );
      #endif
    }
  }
}

/***************************
 * callback - MQTT message
 ***************************/
/**************************************************************************************************
 *    Function      : callback
 *    Description   : none
 *    Input         : char* topic, byte* payload, unsigned int length
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void callback(char* topic, byte* payload, unsigned int length) {

}