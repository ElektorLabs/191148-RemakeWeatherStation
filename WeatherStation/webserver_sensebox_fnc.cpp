#include<ArduinoJson.h>
#include "datastore.h"
#include "webserver_sensebox_fnc.h"

static WebServer * SensboxWebserver=nullptr;

SenseBoxUpload* Sbxptr = nullptr;

void GetSenseboxChMapping( void );
void SetSenseboxChMapping( void );
void SetSensboxServerSettings( void );
void GetSensboxSeverSettings( void );
void GetSenseboxMapping( void );

/**************************************************************************************************
 *    Function      : Webserver_SenseBox_FunctionsRegister
 *    Description   : Registers new URL for handling
 *    Input         : WebServer* serverptr 
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void Webserver_SenseBox_FunctionsRegister(WebServer* server){

    if(server == nullptr ){
        return;
    }
    SensboxWebserver = server;

    //Register the requiered function calls
    server->on("/sensebox/settings.dat", HTTP_POST,SetSensboxServerSettings);
    server->on("/sensebox/settings.json", HTTP_GET, GetSensboxSeverSettings);
    server->on("/sensebox/mapping/{}", HTTP_GET, GetSenseboxChMapping );
    server->on("/sensebox/mapping/{}", HTTP_POST, SetSenseboxChMapping );
    server->on("/sensebox/mapping.json",HTTP_GET, GetSenseboxMapping);
}

/**************************************************************************************************
 *    Function      : Webserver_SenseBox_RegisterSensebox
 *    Description   : This will register accesss to the SenseBoxUpload Object
 *    Input         : SenseBoxUpload * Sbx 
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void Webserver_SenseBox_RegisterSensebox( SenseBoxUpload * Sbx){
   Sbxptr = Sbx;

}

/**************************************************************************************************
 *    Function      : GetSensboxSeverSettings
 *    Description   : Will response with current settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetSensboxSeverSettings(){
    WebServer * server = SensboxWebserver;
    if(server == nullptr){
        return;
    }
    if(Sbxptr == nullptr){
        server->send(500); 
        return;
    }
    String response ="";
    DynamicJsonDocument root(2048);
    root["SENSEBOX_ID"]= Sbxptr->GetSensBoxID();
    root["SENSEBOX_ENA"] =(bool)(Sbxptr->GetSensBoxEnable());
    root["SENSEBOX_TXINTERVALL"] = Sbxptr->GetSensBoxUploadInterval();
    serializeJson(root, response);
    server->send(200, "text/plain", response);

}





/**************************************************************************************************
 *    Function      : SetSensboxServerSettings
 *    Description   : Will process POST with current settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void SetSensboxServerSettings( void ){
    WebServer * server = SensboxWebserver;
    if(server == nullptr){
        return;
    }
    if(Sbxptr == nullptr){
        server->send(500); 
        return;
    }
    //We read everything we have from the post command....
    if( ! server->hasArg("SENSEBOX_ID") || server->arg("SENSEBOX_ID") == NULL ) { 
    /* we are missong something here */
    } else { 
        String value = server->arg("SENSEBOX_ID");
        Sbxptr->SetSensBoxID( value );
        #ifdef DEBUG_SERIAL
        Serial.print("New SenseboxID:");
        Serial.println(value);
       #endif
    }

    if( ! server->hasArg("SENSEBOX_ENA") || server->arg("SENSEBOX_ENA") == NULL ) { 
    /* we are missing something here */
    } else { 
        bool value = false;
        if(server->arg("SENSEBOX_ENA")=="true"){
            value = true;
            #ifdef DEBUG_SERIAL
            Serial.println("Sensebox enable");
            #endif
        } else {
            #ifdef DEBUG_SERIAL
            Serial.println("Sensebox disable");
            #endif
        }
        Sbxptr->SetSensBoxEnable( value );
    }

    if( ! server->hasArg("SENSEBOX_TXINTERVALL") || server->arg("SENSEBOX_TXINTERVALL") == NULL ) { 
            /* we are missong something here */
    } else { 
        uint32_t value = server->arg("SENSEBOX_TXINTERVALL").toInt();
        if( (value >=0 ) && ( value < UINT16_MAX )){
            Sbxptr->SetSensBoxUploadInterval( value );
            #ifdef DEBUG_SERIAL
            Serial.print("New Sensebox TX intervall:");
            Serial.println(value);
            #endif
        }
    }
     server->send(200); 
}

/**************************************************************************************************
 *    Function      : GetSenseboxChMapping
 *    Description   : Will process Request with placeholder
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetSenseboxChMapping( void ){
    WebServer * server = SensboxWebserver;
    if(server == nullptr){
        return;
    }
    if(Sbxptr == nullptr){
        server->send(500); 
        return;
    }
    SenseBoxUpload::SensBoxMapping_t Mapping;
    uint8_t max_channel = Sbxptr->GetMaxMappingChannels();
    String IDs = server->pathArg(0);//This will be the channel we like to set
    int32_t Ch = IDs.toInt();
    if( (Ch<0) ){
        Ch=0;
    }

    if(Ch>=max_channel){
        Ch=max_channel-1; 
    }
    
    Mapping =  Sbxptr->GetMapping(Ch );

    //We build a qick JSON 
    String response ="";
    DynamicJsonDocument root(2048);
    root["channel"]= Ch;
    root["Sta_Channel"]= Mapping.StationChannelIdx;
    root["Enable"] =(bool)(Mapping.enable);
    root["SensorID"] = Mapping.SenseBoxSensorID;
    serializeJson(root, response);
    server->send(200, "text/plain", response);

}

/**************************************************************************************************
 *    Function      : SetSenseboxChMapping
 *    Description   : Will process POST with mapping for new channel
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void SetSenseboxChMapping( void ){
    WebServer * server = SensboxWebserver;
    if(server == nullptr){
        return;
    }
    if(Sbxptr == nullptr){
        server->send(500); 
        return;
    }
   
    SenseBoxUpload::SensBoxMapping_t Mapping;
    String IDs = server->pathArg(0);//This will be the channel we like to set
    int32_t Ch = IDs.toInt();
    if( (Ch<0) ){
        Ch=0;
    }
    Mapping = Sbxptr->GetMapping( Ch ); //Load current Data and modify it....
    if( ! server->hasArg("SENSEBOX_CH_ENA") || server->arg("SENSEBOX_CH_ENA") == NULL ) { 
    /* we are missing something here */
    } else { 
        bool value = false;
        if(server->arg("SENSEBOX_CH_ENA")=="true"){
            value = true;
        }
        Mapping.enable = value;
    }

  
    if( ! server->hasArg("SENSEBOX_STA_CH") || server->arg("SENSEBOX_STA_CH") == NULL ) { 
        /* we are missong something here */
    } else { 
        int32_t value = server->arg("SENSEBOX_STA_CH").toInt();
        if( (value>=0) && ( value<=UINT8_MAX ) ){
            Mapping.StationChannelIdx = value;
        }
    }

    if( ! server->hasArg("SENSEBOX_CH_ID") || server->arg("SENSEBOX_CH_ID") == NULL ) { 
        /* we are missong something here */
    } else { 
        String value = server->arg("SENSEBOX_CH_ID");
        strncpy(Mapping.SenseBoxSensorID, value.c_str(),32);
    }
    #ifdef DEBUG_SERIAL
        Serial.print("Channel:");
        Serial.print(Ch);
        Serial.print(" with ID: ");
        Serial.print(Mapping.SenseBoxSensorID);
        Serial.print(" @ STA CH");
        Serial.print(Mapping.StationChannelIdx );
        Serial.print( " is ");
    #endif
    if(Mapping.enable==0){
        Serial.println( "disabled");
    } else {
        Serial.println( "enabled");
    }
    Sbxptr->SetMapping(Ch, Mapping);
    server->send(200); 

}


/**************************************************************************************************
 *    Function      : GetSenseboxMapping
 *    Description   : Will response with currrent settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetSenseboxMapping( void ){

    WebServer * server = SensboxWebserver;
    if(server == nullptr){
        return;
    }
    if(Sbxptr == nullptr){
        server->send(500); 
        return;
    }
    
    String response ="";
    DynamicJsonDocument doc(4096);
    JsonArray Map = doc.createNestedArray("Mapping");
    SenseBoxUpload::SensBoxMapping_t Mapping;
    uint8_t max_channel = Sbxptr->GetMaxMappingChannels();
    for(uint32_t i=0;i<max_channel;i++){
        Mapping =  Sbxptr->GetMapping( i );
        JsonObject Mapping_0 = Map.createNestedObject();
        Mapping_0["STA_Channel"] =  Mapping.StationChannelIdx;
        Mapping_0["SensorID"] = Mapping.SenseBoxSensorID;
        Mapping_0["Enabled"] = (bool)(Mapping.enable);
    }

    serializeJson(doc, response);
    server->send(200, "text/plain", response);
}