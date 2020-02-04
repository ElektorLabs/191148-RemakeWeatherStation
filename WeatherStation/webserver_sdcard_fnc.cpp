
#include <ArduinoJson.h>
#include "./src/sdcard/sdcard_if.h"
#include "webserver_sdcard_fnc.h"

WebServer * SDCardWebServer = nullptr;
void UmountCard( void );
void MountCard(void);
void GetMountStatus( void );
void GetSettings( void );
void UpdateSettings( void );

/**************************************************************************************************
 *    Function      : Webserver_SDCard_FunctionsRegister
 *    Description   : Registers new URL for handling
 *    Input         : WebServer* serverptr 
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void Webserver_SDCard_FunctionsRegister(WebServer* server){
    SDCardWebServer = server;
    server->on("/sdlog/settings.dat", HTTP_POST, UpdateSettings );
    server->on("/sdlog/settings.json", HTTP_GET, GetSettings );
    server->on("/sdlog/sd/mount", MountCard);
    server->on("/sdlog/sd/umount", UmountCard);
    server->on("/sdlog/sd/status", GetMountStatus);
}

/**************************************************************************************************
 *    Function      : UmountCard
 *    Description   : Will unmount SD-Card
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void UmountCard( void ){
    WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    //We try to unmount the card
    sdcard_umount();
    server->send(200);
}

/**************************************************************************************************
 *    Function      : MountCard
 *    Description   : Will mount SD-Card
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void MountCard( void ){
 WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    //We will try to mount the card....
    sdcard_mount();
    server->send(200);

}

/**************************************************************************************************
 *    Function      : GetMountStatus
 *    Description   : Will report the SD-Card status
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetMountStatus( void ){
    bool mounted = sdcard_getmounted();
    //We return a json for this, also some other infos
    WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    
    String response ="";
    DynamicJsonDocument root(2048);
    root["SDCardMounted"] =(bool)(mounted);
    root["SDCardSize"] =(uint32_t)( sdcard_GetCapacity() );
    root["SDCardFree"] =(uint32_t)( sdcard_GetFreeSpace() );  
    serializeJson(root, response);
    server->send(200, "text/plain", response);


}


/**************************************************************************************************
 *    Function      : UpdateSettings
 *    Description   : Will process POST with new settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void UpdateSettings( void ){
    WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    if( ! server->hasArg("SDLOG_ENA") || server->arg("SDLOG_ENA") == NULL ) { 
    /* we are missing something here */
    } else { 
        bool value = false;
        if(server->arg("SDLOG_ENA")=="true"){
            value = true;
            Serial.print("New SDCard Log  enabled");
        } else {
             Serial.print("New SDCard Log  disabled");
        }
        sdcard_log_enable( value );
       
    }

    if( ! server->hasArg("SDLOG_INT") || server->arg("SDLOG_INT") == NULL ) { 
            /* we are missong something here */
    } else { 
        uint32_t value = server->arg("SDLOG_INT").toInt();
        if( (value >=0 ) && ( value < UINT16_MAX )){
               sdcard_log_int( value );
            Serial.print("New SDCard Log  interval:");
            Serial.println(value);
        }
    }
    server->send(200); 
}

/**************************************************************************************************
 *    Function      : GetSettings
 *    Description   : Will report settings
 *    Input         : void
 *    Output        : none
 *    Remarks       : None
 **************************************************************************************************/
void GetSettings( void ){

    //We just have log enable and also the interval
    bool enable = sdcard_log_getenable();
    uint16_t interval = sdcard_log_getinterval();

    WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    
    String response ="";
    DynamicJsonDocument root(2048);
    root["Enable"] =(bool)(enable);
    root["LogInterval"] = interval;
    serializeJson(root, response);
    server->send(200, "text/plain", response);


}


