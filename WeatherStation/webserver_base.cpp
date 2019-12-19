#include "webserver_base.h"
#include "wifi_net.h"
#include "datastore.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

WebServer* server = NULL;

/* Function Prototypes */
String SSIDList( void );
void getWiFiSettings( void );
void setWiFiSettings( void );
void restart( void );
void getSSIDList( void );

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


/**************************************************************************************************
 *    Function      : sendFile
 *    Description   : Sends a requestedfile from SPIFFS
 *    Input         : none 
 *    Output        : none
 *    Remarks       : send a file from the SPIFFS to the connected client of the webserver
 **************************************************************************************************/
void sendFile() {
  String path = server->uri();
  Serial.println("Got request for: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  Serial.println("Content type: " + contentType);
  if (SPIFFS.exists(path)) {
    Serial.println("File " + path + " found");
    File file = SPIFFS.open(path, "r");
    server->streamFile(file, contentType);
    file.close();
  }
  else {
    Serial.println("File '" + path + "' doesn't exist");
    server->send(404, "text/plain", "The requested file doesn't exist");
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


    
    server->onNotFound(sendFile); //handle everything except the above things
    server->begin();
    Serial.println("Webserver started");
  } else {
    Serial.println("Webserver already running");
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
  Serial.println("SSID list requested");
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
  Serial.println("Scanning networks");
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
  Serial.println("WiFi settings requested");
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
  Serial.println("WiFi settings received");
  ssid = server->arg("ssid");
  pass = server->arg("pass");
  String response = "Attempting to connect to '" + ssid + "'. The WiFi module restarts and tries to connect to the network.";
  sendData(response);
  Serial.println("Saving network credentials and restart.");
  strncpy((char*)(c.ssid),(char*)(ssid.c_str()),128);
  strncpy((char*)(c.pass),(char*)(pass.c_str()),128);
  Serial.printf("write ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  write_credentials(c);
  
  c = read_credentials();
  Serial.printf("read ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  /* if we do this we end up in flashloader */
  delay(2000);
  ESP.restart();

  
}

void WebserverLoopTask( void ){
   if (server != NULL){
    server->handleClient();
   } else {
    /* This may be strange */
   }
}
