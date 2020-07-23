#include<ArduinoJson.h>
#include "datastore.h"
#include "webserver_thinkspeak_fnc.h"

static WebServer * ThinkspeakWebserver=nullptr;

ThinkspeakUpload* Thsp = nullptr;

static void GetThinkspeakChMapping( void );
static void SetThinkspeakChMapping( void );
static void SetThinkspeakServerSettings( void );
static void GetThinkspeakSeverSettings( void );
void GetThinkspeakMapping( void );



/**************************************************************************************************
 *    Function      : Webserver_Thinkspeak_FunctionsRegister
 *    Description   : Registers new URL for handling
 *    Input         : WebServer* serverptr 
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void Webserver_Thinkspeak_FunctionsRegister(WebServer* server){

    if(server == nullptr ){
        return;
    }
    ThinkspeakWebserver = server;

    //Register the requiered function calls
    server->on("/thingspeak/settings.dat", HTTP_POST, SetThinkspeakServerSettings);
    server->on("/thingspeak/settings.json", HTTP_GET, GetThinkspeakSeverSettings);
    server->on("/thingspeak/mapping/{}", HTTP_GET, GetThinkspeakChMapping );
    server->on("/thingspeak/mapping/{}", HTTP_POST, SetThinkspeakChMapping );
    server->on("/thingspeak/mapping.json", HTTP_GET, GetThinkspeakMapping);
    
}

/**************************************************************************************************
 *    Function      : Webserver_Thinkspeak_RegisterThinkspeak
 *    Description   : Registers access to ThinkspeakUpload Object
 *    Input         : ThinkspeakUpload * Thptr 
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void Webserver_Thinkspeak_RegisterThinkspeak( ThinkspeakUpload * Thptr){
     Thsp = Thptr;

}


/**************************************************************************************************
 *    Function      : GetThinkspeakSeverSettings
 *    Description   : response with settings to client
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetThinkspeakSeverSettings(){
    WebServer * server = ThinkspeakWebserver;
    if(server == nullptr){
        return;
    }
    if(Thsp == nullptr){
        server->send(500); 
        return;
    }
    String response ="";
    DynamicJsonDocument root(2048);
    root["APIKEY"]= Thsp->GetThinkspeakAPIKey();
    root["THINKSPEAK_ENA"] =(bool)(Thsp->GetThinkspeakEnable());
    root["THINKSPEAK_TXINTERVALL"] = Thsp->GetThinkspeakUploadInterval();
    serializeJson(root, response);
    server->send(200, "text/plain", response);

}





/**************************************************************************************************
 *    Function      : SetThinkspeakServerSettings
 *    Description   : Process POST with new Settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void SetThinkspeakServerSettings( void ){
    WebServer * server = ThinkspeakWebserver;
    if(server == nullptr){
        return;
    }
    if(Thsp == nullptr){
        server->send(500); 
        return;
    }
    //We read everything we have from the post command....
    if( ! server->hasArg("APIKEY") || server->arg("APIKEY") == NULL ) { 
    /* we are missong something here */
    } else { 
        String value = server->arg("APIKEY");
        Thsp->SetThinkspeakAPIKey( value );
        #ifdef DEBUG_SERIAL
         Serial.print("New Thinkspeak Key:");
         Serial.println(value);
        #endif
       
    }

    if( ! server->hasArg("THINKSPEAK_ENA") || server->arg("THINKSPEAK_ENA") == NULL ) { 
    /* we are missing something here */
    } else { 
        bool value = false;
        if(server->arg("THINKSPEAK_ENA")=="true"){
            value = true;
            #ifdef DEBUG_SERIAL
             Serial.println("Thinkspeak enable");
            #endif
        } else {
            #ifdef DEBUG_SERIAL
             Serial.println("Thinkspeak disable");
            #endif
        }
        Thsp->SetThinkspeakEnable( value );
    }

    if( ! server->hasArg("THINKSPEAK_TXINTERVALL") || server->arg("THINKSPEAK_TXINTERVALL") == NULL ) { 
            /* we are missong something here */
    } else { 
        uint32_t value = server->arg("THINKSPEAK_TXINTERVALL").toInt();
        if( (value >=0 ) && ( value < UINT16_MAX )){
            Thsp->SetThinkspeakUploadInterval( value );
            #ifdef DEBUG_SERIAL
             Serial.print("New Thinkspeak TX interva:");
             Serial.println(value);
            #endif
        }
    }
     server->send(200); 
}

/**************************************************************************************************
 *    Function      : GetThinkspeakChMapping
 *    Description   : Send current Mapping to Client
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetThinkspeakChMapping( void ){
    WebServer * server = ThinkspeakWebserver;
    if(server == nullptr){
        return;
    }
    if(Thsp == nullptr){
        server->send(500); 
        return;
    }
    ThinkspeakUpload::ThinkspeakMapping_t Mapping;
    uint8_t max_channel = Thsp->GetMaxMappingChannels();
    String IDs = server->pathArg(0);//This will be the channel we like to set
    int32_t Ch = IDs.toInt();
    if( (Ch<0) ){
        Ch=0;
    }

    if(Ch>=max_channel){
        Ch=max_channel-1; 
    }
    
    Mapping =  Thsp->GetMapping(Ch );
    //We build a qick JSON 
    String response ="";
    DynamicJsonDocument root(2048);
    root["channel"]= Ch;
    root["Sta_Channel"]= Mapping.StationChannelIdx;
    root["Enable"] =(bool)(Mapping.enable);
    serializeJson(root, response);
    server->send(200, "text/plain", response);

}

/**************************************************************************************************
 *    Function      : SetThinkspeakChMapping
 *    Description   : process POST for new channel mapping
 *    Input         : void
 *    Output        : none
 *    Remarks       : Uses wildcard in URL
 **************************************************************************************************/
void SetThinkspeakChMapping( void ){
    WebServer * server = ThinkspeakWebserver;
    if(server == nullptr){
        return;
    }
    if(Thsp == nullptr){
        server->send(500); 
        return;
    }
   
    ThinkspeakUpload::ThinkspeakMapping_t Mapping;
    String IDs = server->pathArg(0);//This will be the channel we like to set
    int32_t Ch = IDs.toInt();
    if( (Ch<0) ){
        Ch=0;
    }
    Mapping =  Thsp->GetMapping(Ch );
    if( ! server->hasArg("THINKSPEAK_CH_ENA") || server->arg("THINKSPEAK_CH_ENA") == NULL ) { 
    /* we are missing something here */
    } else { 
        bool value = false;
        if(server->arg("THINKSPEAK_CH_ENA")=="true"){
            value = true;
        }
        Mapping.enable = value;
    }

    if( ! server->hasArg("THINKSPEAK_STA_CH") || server->arg("THINKSPEAK_STA_CH") == NULL ) { 
        /* we are missong something here */
    } else { 
        int32_t value = server->arg("THINKSPEAK_STA_CH").toInt();
        if( (value>=0) && ( value<=UINT8_MAX ) ){
            Mapping.StationChannelIdx = value;
        }
    }
   #ifdef DEBUG_SERIAL
    Serial.print("Channel:");
    Serial.print(Ch);
    Serial.print(" @ STA CH");
    Serial.print(Mapping.StationChannelIdx );
    Serial.print( " is ");
   #endif
    if(Mapping.enable==0){
        Serial.println( "disabled");
    } else {
        Serial.println( "enabled");
    }
    Thsp->SetMapping(Ch, Mapping);
    server->send(200); 

}

/**************************************************************************************************
 *    Function      : GetThinkspeakMapping
 *    Description   : Get mapping for one Channel
 *    Input         : void
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void GetThinkspeakMapping( void ){
    WebServer * server = ThinkspeakWebserver;
    if(server == nullptr){
        return;
    }
    if(Thsp == nullptr){
        server->send(500); 
        return;
    }
    
    String response ="";
    DynamicJsonDocument doc(4096);
    JsonArray Map = doc.createNestedArray("Mapping");
    ThinkspeakUpload::ThinkspeakMapping_t Mapping;
    uint8_t max_channel = Thsp->GetMaxMappingChannels();
    for(uint32_t i=0;i<max_channel;i++){
        Mapping = Thsp->GetMapping(i );
        JsonObject Mapping_0 = Map.createNestedObject();
        Mapping_0["STA_Channel"] =  Mapping.StationChannelIdx;
        Mapping_0["Enabled"] = (bool)(Mapping.enable);
    }

    serializeJson(doc, response);
    server->send(200, "text/plain", response);

}