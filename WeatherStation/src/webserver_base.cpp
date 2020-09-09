#include "webserver_base.h"
#include "wifi_net.h"
#include "datastore.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Update.h>
#include "version.h"

WebServer* server = NULL;

/* Function Prototypes */
String SSIDList( void );
void getWiFiSettings( void );
void setWiFiSettings( void );
void restart( void );
void getSSIDList( void );
void getInfo( void );

//This will even work if the spiffs is not mounatable or empty
const char* serverFirmwareIndex = "<form method='POST' action='/update/fimrware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
const char* serverSPIFFSIndex = "<form method='POST' action='/update/spiffs' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


/**************************************************************************************************
 *    Function      : getContentType
 *    Description   : Gets the contenttype depending on a filename 
 *    Input         : String
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String getContentType(String filename) {
  if (server->hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


void sendError404( void ){
   #ifdef DEBUG_SERIAL
    Serial.println("File doesn't exist (404)");
   #endif
   server->send(404, "text/plain", "The requested file doesn't exist");
}

/**************************************************************************************************
 *    Function      : sendFile
 *    Description   : Sends a requestedfile from SPIFFS
 *    Input         : none 
 *    Output        : none
 *    Remarks       : send a file from the SPIFFS to the connected client of the webserver
 **************************************************************************************************/
void sendFile() {
  String path = server->uri();
  #ifdef DEBUG_SERIAL
    Serial.println("Got request for: " + path);
  #endif
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  #ifdef DEBUG_SERIAL
    Serial.println("Content type: " + contentType);
  #endif
  if (SPIFFS.exists(path)) {
    #ifdef DEBUG_SERIAL
      Serial.println("File " + path + " found");
    #endif
    File file = SPIFFS.open(path, "r");
    server->streamFile(file, contentType);
    file.close();
  }
  else {
    if(path=="/index.html"){
      //If we are here the index html is missing so no webconten at all
      server->sendHeader("Connection", "close");
      server->send(200, "text/html", serverSPIFFSIndex);
    } else {
      Serial.println("File '" + path + "' doesn't exist");
      sendError404();
    }
  }
  

}

/**************************************************************************************************
 *    Function      : sendFile
 *    Description   : Sends a requestedfile from SPIFFS
 *    Input         : String 
 *    Output        : none
 *    Remarks       : send data to the connected client of the webserver
 **************************************************************************************************/
void sendData(String data) {
  server->send(200, "text/plain", data);
}


void UpdateUploadDone( void ){
  server->sendHeader("Connection", "close");
  server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}

void UpdateFirmwareProcess( void ){
  HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN,U_FLASH,-1,LOW )) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
}

void UpdateSPIFFSProcess( void ){
  HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN,U_SPIFFS  ,-1,LOW )) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
}


/**************************************************************************************************
 *    Function      : configureServer
 *    Description   : Setup for the Webserver
 *    Input         : none 
 *    Output        : none
 *    Remarks       : initialize the webserver on port 80
 **************************************************************************************************/
void SetupWebServer() {
  if(server == NULL) {
    SPIFFS.begin(); /* This can be called multiple times, allready mounted it will just return */
    server = new WebServer (80);
  /* This is the bare minimum we will have inside the webserver */
  server->on("/setWiFiSettings", HTTP_GET, setWiFiSettings);
  server->on("/getWiFiSettings", HTTP_GET, getWiFiSettings);
  server->on("/getSSIDList", HTTP_GET, getSSIDList);
  server->on("/restart", HTTP_GET, restart);
  server->on("/info", HTTP_GET, getInfo);


  //This is for the Webupdate for the firmware
   server->on("/update/fimrware.html", HTTP_GET, []() {
      server->sendHeader("Connection", "close");
      server->send(200, "text/html", serverFirmwareIndex);
   });
   server->on("/update/fimrware", HTTP_POST, UpdateUploadDone, UpdateFirmwareProcess);
     server->on("/update/spiffs.html", HTTP_GET, []() {
      server->sendHeader("Connection", "close");
      server->send(200, "text/html", serverSPIFFSIndex);
   });
   server->on("/update/spiffs", HTTP_POST, UpdateUploadDone, UpdateSPIFFSProcess);

  //access blacklist
  server->on("/mapping.json", HTTP_GET, sendError404);

  //Webupdate functions

    
    server->onNotFound(sendFile); //handle everything except the above things
    server->begin();
    Serial.println("Webserver started at port 80");
  } else {
  #ifdef DEBUG_SERIAL    
    Serial.println("Webserver already running");
  #endif
  }
}

WebServer* WebserverGetHandle( void ){
  return server;
}

/**************************************************************************************************
 *    Function      : setWiFiSettings
 *    Description   : Applies the WiFi settings 
 *    Input         : none
 *    Output        : none
 *    Remarks       : restart the esp as requested on the webpage
 **************************************************************************************************/
void restart() {
  sendData("The ESP will restart and you will be disconnected from the '" + WiFi.SSID() + "' network.");
  delay(1000);
  /* Any fixes for ESP8266 may be here */
  ESP.restart();
}


/**************************************************************************************************
 *    Function      : WiFiStatusToString
 *    Description   : Gives a string representing the WiFi status
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
String WiFiStatusToString() {
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:     return "IDLE"; break;
    case WL_NO_SSID_AVAIL:   return "NO SSID AVAIL"; break;
    case WL_SCAN_COMPLETED:  return "SCAN COMPLETED"; break;
    case WL_CONNECTED:       return "CONNECTED"; break;
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED"; break;
    case WL_CONNECTION_LOST: return "CONNECTION LOST"; break;
    case WL_DISCONNECTED:    return "DISCONNECTED"; break;
    case WL_NO_SHIELD:       return "NO SHIELD"; break;
    default:                 return "Undefined: " + String(WiFi.status()); break;
  }
}

/**************************************************************************************************
 *    Function      : getSSIDList
 *    Description   : Sends a SSID List to the client 
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void getSSIDList() {
    #ifdef DEBUG_SERIAL
      Serial.println("SSID list requested");
    #endif
  sendData(SSIDList());
}


/**************************************************************************************************
 *    Function      : SSIDList
 *    Description   : Returns the SSID List 
 *    Input         : String separator 
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String SSIDList( void ) {
  String Result ="";
  #ifdef DEBUG_SERIAL
  Serial.println("Scanning networks");
  #endif
  ReScanNetworks(true);
  uint16_t n = LastScanWiFiNetworksFound();
  
  size_t capacity = 3*(JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(3) + n*JSON_OBJECT_SIZE(5));
  DynamicJsonDocument doc(capacity);
  
  doc["ScanCount"] = n;
  doc["HiddenSSID"] = false;
  
  JsonArray Networks = doc.createNestedArray("Networks");
  for(uint32_t i=0; i<n;i++){
    JsonObject Networks_0 = Networks.createNestedObject();
    Networks_0["SSID"] = WiFi.SSID(i);
    Networks_0["BSSID"] = WiFi.BSSIDstr(i);
    Networks_0["Ch"] = WiFi.channel(i);
    
    switch(WiFi.encryptionType(i)){
      case WIFI_AUTH_OPEN:{
        Networks_0["Enc"] = "OPEN";
      } break;

      case WIFI_AUTH_WEP:{
          Networks_0["Enc"] = "WEP";
      } break;

      case WIFI_AUTH_WPA_PSK:{
        Networks_0["Enc"] = "WPA_PSK";
      } break;

      case WIFI_AUTH_WPA2_PSK:{
        Networks_0["Enc"] = "WPA2_PSK";
      } break;

      case WIFI_AUTH_WPA_WPA2_PSK:{
          Networks_0["Enc"] = "WPA_WPA2_PSK";
      } break;

      default:{
          Networks_0["Enc"] = "UNKNOW";
      } break;
    }
   
    Networks_0["RSSI"] = WiFi.RSSI(i);
  
  }
  serializeJson(doc, Result);
   
  return Result;

/*
 * {
 "ScanCount":1,
  "HiddenSSID":false,
  "Networks" :[
        { "SSID" : "Fu", "Channel":6 , "Encryption":true, "RSSI":20 }, 
        { "SSID" : "Fu", "Channel":6 , "Encryption":true, "RSSI":20 },
        { "SSID" : "Fu", "Channel":6 , "Encryption":true, "RSSI":20 }
   ]
}
 */
  
}

//send the wifi settings to the connected client of the webserver
void getWiFiSettings() {
  #ifdef DEBUG_SERIAL
    Serial.println("WiFi settings requested");
  #endif
  const size_t capacity = 500+ JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(capacity);
  credentials_t c = read_credentials();  
  String response;
  int16_t RSSI = WiFiGetRSSI();
  String ssid(c.ssid);
  doc["SSID"] = ssid; 
  doc["CHANNEL"]=WiFiGetChannel();
  doc["RSSI"]=RSSI;
 
  serializeJson(doc, response);
  sendData(response);
}


/**************************************************************************************************
 *    Function      : setWiFiSettings
 *    Description   : Applies the WiFi settings 
 *    Input         : none
 *    Output        : none
 *    Remarks       : Store the wifi settings configured on the webpage and restart the esp to connect to this network
 **************************************************************************************************/
void setWiFiSettings( void ) {
  credentials_t c;  
  String ssid ="";
  String pass="";
  #ifdef DEBUG_SERIAL
    Serial.println("WiFi settings received");
  #endif
  ssid = server->arg("ssid");
  pass = server->arg("pass");
  String response = "Attempting to connect to '" + ssid + "'. The WiFi module restarts and tries to connect to the network.";
  sendData(response);
  #ifdef DEBUG_SERIAL
  Serial.println("Saving network credentials and restart.");
  #endif 
  strncpy((char*)(c.ssid),(char*)(ssid.c_str()),128);
  strncpy((char*)(c.pass),(char*)(pass.c_str()),128);
  #ifdef DEBUG_SERIAL
    Serial.printf("write ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  #endif
  write_credentials(c);
  
  c = read_credentials();
  #ifdef DEBUG_SERIAL
    Serial.printf("read ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  #endif
  /* if we do this we end up in flashloader */
  delay(2000);
  ESP.restart();

  
}

/**************************************************************************************************
 *    Function      : getInfo
 *    Description   : Will read the firmware infromation 
 *    Input         : none
 *    Output        : none
 *    Remarks       : Send the curren Fimrwareinformation arround
 **************************************************************************************************/
void getInfo( void ){

  const size_t capacity = 250+ JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(capacity);
  String response;
  doc["Version_Major"] = (uint8_t)( FW_VERSION_MAJOR ); 
  doc["Version_Minor"] = (uint8_t)( FW_VERSION_MINOR ); 
  serializeJson(doc, response);
  sendData(response);

}

/**************************************************************************************************
 *    Function      : WebserverLoopTask
 *    Description   : Peridically to be called task for Webserver
 *    Input         : none
 *    Output        : none
 *    Remarks       : needs to be called frequently
 **************************************************************************************************/
void WebserverLoopTask( void ){
   if (server != NULL){
    server->handleClient();
   } else {
    /* This may be strange */
   }
}
