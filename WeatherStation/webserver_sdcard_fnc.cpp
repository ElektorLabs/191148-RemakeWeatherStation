
#include <ArduinoJson.h>
#include "./src/sdcard/sdcard_if.h"
#include "webserver_sdcard_fnc.h"

WebServer * SDCardWebServer = nullptr;
void UmountCard( void );
void MountCard(void);
void GetMountStatus( void );
void GetSettings( void );
void UpdateSettungs( void );
void Webserver_SDCard_FunctionsRegister(WebServer* server){
    SDCardWebServer = server;
    server->on("/sdlog/settings.dat", HTTP_GET, UpdateSettungs );
    server->on("/sdlog/settings.json", HTTP_POST, GetSettings );
    server->on("/sdlog/sd/mount", MountCard);
    server->on("/sdlog/sd/umount", UmountCard);
    server->on("/sdlog/sd/status", GetMountStatus);
}

//We need to get a mount status 
//also a mount and unmount


void UmountCard( void ){
    WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    //We try to unmount the card
    sdcard_umount();
    server->send(200);
}

void MountCard( void ){
 WebServer * server = SDCardWebServer;
    if(server == nullptr){
        return;
    }
    //We will try to mount the card....
    sdcard_mount();
    server->send(200);

}

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

void UpdateSettungs( void ){
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

    if( ! server->hasArg("SENSEBOX_TXINTERVALL") || server->arg("SENSEBOX_TXINTERVALL") == NULL ) { 
            /* we are missong something here */
    } else { 
        uint32_t value = server->arg("SENSEBOX_TXINTERVALL").toInt();
        if( (value >=0 ) && ( value < UINT16_MAX )){
               sdcard_log_int( value );
            Serial.print("New SDCard Log  interval:");
            Serial.println(value);
        }
    }
     server->send(200); 



  

}

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


