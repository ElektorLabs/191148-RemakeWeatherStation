#include <esp_wifi.h>
#include <esp_wps.h>
#include <WiFi.h>
#include <Ticker.h>

#include "datastore.h"
#include "webserver_base.h"
#include "webserver_map_fnc.h"
#include "webserver_sensebox_fnc.h"
#include "webserver_sdcard_fnc.h"
#include "webserver_thinkspeak_fnc.h"
#include "webserver_time_fnc.h"
#include "webserver_mqtt_fnc.h"
#include "wifi_net.h"

#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

String APSSID = "ESP32 XX:XX:XX";

String ssid ="";
String pass="";

static esp_wps_config_t wps_config;
static wifi_config_t wifi_conf;

void (*scandone_cb[4]) (void)={NULL,};

bool StarStopeMode = false;

bool WPS_Running = false;

/* Function prototypes */
String wpspin2string(uint8_t a[]);
void configureSoftAP( bool use_wifi_config );
bool connectWiFi( void );
void connectioTask( void *);
void eventConnectToApFailed( void );

void StartDelayedReconnect( void );
void configureServer( void );
String WiFiGetStatus( void );
SemaphoreHandle_t xConnectTaskDoneSem=NULL;
SemaphoreHandle_t xConTaskStartMtx=NULL;
SemaphoreHandle_t xConnections=NULL;

Ticker AutoReconnect; 

/*
* WiFi Events
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
*/





void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
    //Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
        case SYSTEM_EVENT_WIFI_READY: 
            Serial.println("WiFi interface ready");
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            //Serial.println("Completed scan for access points");
            for(uint32_t i = 0; i < ( sizeof(scandone_cb)/sizeof(scandone_cb[0]) ); i++){    
              if(scandone_cb[i]!=NULL){
                      scandone_cb[i]();
              }
            }
            break;
        case SYSTEM_EVENT_STA_START:
            Serial.println("Station Mode Started");
            break;
        case SYSTEM_EVENT_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("Connected to access point");
            AutoReconnect.detach();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
              Serial.println("Connected to :" + String(WiFi.SSID()));
              Serial.print("Got IP: ");
              Serial.println(WiFi.localIP());
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            //Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
            esp_wifi_wps_disable();
            delay(10);
            esp_wifi_get_config(WIFI_IF_STA,&wifi_conf);
            //We save the config and start the reconnectio task from here 
            credentials_t c;
            strncpy((char*)c.ssid, (char*)wifi_conf.sta.ssid,  sizeof(c.ssid) );
            strncpy((char*)c.pass, (char*)wifi_conf.sta.password,  sizeof(c.pass) );
            write_credentials(c);
            delay(1);
            //Start the WiFi Connection Task
            WiFiClientEnable(true); //Force WiFi on 
            WiFiForceAP(false); //Diable Force AP
            _ReconnectWiFi(true);
             WPS_Running=false;
            //We should be good to go now ....          
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            esp_wifi_wps_disable();
             WPS_Running=false;
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            Serial.println("WPS Timedout, retrying");
            esp_wifi_wps_disable();
            //Shutdown WiFi and Restart
            WiFiForceAP(false); //Diable Force AP
            _ReconnectWiFi(true);
            WPS_Running=false;
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            Serial.println("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
            WPS_Running=false;
            break;
            break;
        case SYSTEM_EVENT_AP_START:
            Serial.println("WiFi access point started");
            break;
        case SYSTEM_EVENT_AP_STOP:
            Serial.println("WiFi access point  stopped");
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            Serial.println("Disconnected from station, attempting reconnection");
            WiFi.reconnect();
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case SYSTEM_EVENT_GOT_IP6:
            Serial.println("IPv6 is preferred");
            break;
        case SYSTEM_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case SYSTEM_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
    }
}
//WPS Functions 
void wpsInitConfig(){
  wps_config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  wps_config.wps_type = ESP_WPS_MODE;
  strcpy(wps_config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(wps_config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(wps_config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(wps_config.factory_info.device_name, ESP_DEVICE_NAME);
}

String wpspin2string(uint8_t a[]){
  char wps_pin[9];
  for(int i=0;i<8;i++){
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

void WPS_Start(){
  //This will only work is the statio is not connected to a wifi and also not configured to do so
  WPS_Running = true;
  esp_wifi_disconnect();
  delay(10);
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_STA);

  Serial.println("Starting WPS");

  wpsInitConfig();
  esp_wifi_wps_enable(&wps_config);
  esp_wifi_wps_start(0);
  //Next is to wait what the driver will tell us....
  Serial.println("WPS started");
}




void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void ProcessWifiScan(WiFiEvent_t event, WiFiEventInfo_t info){
  /* We assume scan is run okay */
  int16_t status = WiFi.scanComplete();
  if(status < 0 ){
    /*  This indicates an error */
  } else {
    /* We need to send a new list of wifi stations */
  }
  
}




void ReScanNetworks(bool WaitForReady , bool ScanForHiddenNetworks){
  int16_t ScanResult=-1;
  WiFi.scanDelete();
  WiFi.scanNetworks(true, ScanForHiddenNetworks);
  if(true == WaitForReady ){
      while(-1==ScanResult ){
        /* We need to wait */
        Serial.printf("Wait for Scan to be done %i\n\r",ScanResult);
        ScanResult = WiFi.scanComplete();
        vTaskDelay(1);
      }
      if(ScanResult==-2){
         /*This is a poblem and shall not happen */
      }
  }
 
}

uint16_t LastScanWiFiNetworksFound(){
  int16_t status = WiFi.scanComplete();
  if(status < 0 ){
    /* No Networks found */
    return 0;
  } else {
    return status;
  }
}

char* GetEncryptioModeStr(wifi_auth_mode_t mode ){
   char* ENCRYPTION= (char*)String("NDEF").c_str();
   switch(mode){
      case WIFI_AUTH_OPEN:{
        ENCRYPTION   =  (char*)String("OPEN").c_str();
      } break;

      case WIFI_AUTH_WEP:{
          ENCRYPTION   = (char*)String("WEP").c_str();
      } break;

      case WIFI_AUTH_WPA_PSK:{
        ENCRYPTION   = (char*)String("WPA_PSK").c_str();
      } break;

      case WIFI_AUTH_WPA2_PSK:{
        ENCRYPTION = (char*)String("WPA2_PSK").c_str();
      } break;

      case WIFI_AUTH_WPA_WPA2_PSK:{
          ENCRYPTION   = (char*)String("WPA_WPA2_PSK").c_str();
      } break;

      default:{
          ENCRYPTION   = (char*)String("UNKNOWN").c_str();
      } break;
    }
  return ENCRYPTION;
}

bool GetScanResultOfIDX( wifi_scan_result_t* result, uint8_t index){
int16_t status = WiFi.scanComplete();
if( ( status <0 ) || ( status < index ) ) {
  return false;
}

  result->SSID = WiFi.SSID(index);
  result->BSSID = WiFi.BSSIDstr(index);
  result->CHANNEL = WiFi.channel(index);
  result->ENCRYPTION = GetEncryptioModeStr(WiFi.encryptionType(index) );
  result->RSSI = WiFi.RSSI(index);
  return true;
}


void WiFiStop( void  ){
    //If we are on SDK 3.2  / ESP32 Arduino 1.0.4 this will cause a kernel panic
    WiFi.mode(WIFI_OFF);
}

void WiFiStart( void ){
  WiFi.setSleep(false);
  esp_wifi_start();
}

void WiFiForceSleep( void ){
  WiFi.setSleep(true);
  delay(1);
}

void WiFiClientEnable(bool Ena){
  wifi_param_config_t settings = read_wifi_config();
  settings.WIFI_Ena = Ena;
  write_wifi_config( settings );
  
}

void WiFiForceAP( bool Ena){
   wifi_param_config_t settings = read_wifi_config();
  settings.AP_Mode = Ena;
  write_wifi_config( settings );
}

/**************************************************************************************************
 *    Function      : connectioTask
 *    Description   : Task to connect to a WiFi
 *    Input         : String 
 *    Output        : none
 *    Remarks       : initialize wifi by connecting to a wifi network or creating an accesspoint
 **************************************************************************************************/
void WiFiconnectionTask( void * voparam){
Serial.println("Connection Task");
bool connected = false;
wifi_connect_param_t* param = NULL;
if(voparam==NULL){
 
} else {
  /* Signal connection failed */
  param = ( wifi_connect_param_t* )( voparam );
 
   if (param->ssid[0] == 0) {
    param->connected = false;
  } else {
      WiFi.mode(WIFI_STA);
      WiFi.begin(( char*)param->ssid, ( char*)param->pass);
      for (int timeout = 0; timeout < 15; timeout++) { //max 15 seconds
        int status = WiFi.status();
        if ((status == WL_CONNECTED)  || (status == WL_NO_SSID_AVAIL) || (status == WL_CONNECT_FAILED)){
          break;
        }
        Serial.print(".");
      
        vTaskDelay(1000/portTICK_PERIOD_MS);
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        param->connected = false;
      } else {
        /* We need to check if a update is requiered */
        credentials_t c = read_credentials();
        if( (0 != strncmp (c.pass,param->pass,sizeof(c.pass)) )|| (0 != strncmp (c.ssid,param->ssid,sizeof(c.ssid)) ) ){
          strncpy(c.pass,param->pass,sizeof(c.pass));
          strncpy(c.ssid,param->ssid,sizeof(c.ssid));
           write_credentials(c);
        }
        configureServer();
        param->connected = true;
        //This is for debugging....(sort of )

        {
            Serial.println("");
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("ESP Mac Address: ");
            Serial.println(WiFi.macAddress());
            Serial.print("Subnet Mask: ");
            Serial.println(WiFi.subnetMask());
            Serial.print("Gateway IP: ");
            Serial.println(WiFi.gatewayIP());
            Serial.print("DNS: ");
            Serial.println(WiFi.dnsIP());


        }


      }
    }
    /* Callback to be done */
    if(param->result_cb!=NULL){
         param->result_cb(connected); 
    }

    if(param->CallerSemaphore!=NULL){
      xSemaphoreGive(param->CallerSemaphore);
    }  
    xSemaphoreGive( xConTaskStartMtx );     
    /* Distroy task */
    vTaskDelete( NULL );
    while(1==1){
      vTaskDelay(portMAX_DELAY);
    }
    /* May this can be a permant task later */
}


void StartWiFiConnect( wifi_connect_param_t* parameter ){
   xSemaphoreTake( xConTaskStartMtx, portMAX_DELAY );
   xTaskCreate(
						WiFiconnectionTask,   /* Function to implement the task */
						"WIFI_ConnectionTask",  /* Name of the task */
						8192,       /* Stack size in words */
						static_cast<void*>(parameter ),       /* Task input parameter */
						1,          /* Priority of the task */
						NULL  /* Task handle. */
					);
}



/**************************************************************************************************
 *    Function      : initWiFi
 *    Description   : initializes the WiFi
 *    Input         : String 
 *    Output        : none
 *    Remarks       : initialize wifi by connecting to a wifi network or creating an accesspoint
 **************************************************************************************************/
void initWiFi( bool forceAP, bool force_openap) {

xConnectTaskDoneSem = xSemaphoreCreateBinary();
xConTaskStartMtx=xSemaphoreCreateMutex();
xConnections = xSemaphoreCreateCounting(16,0);
xSemaphoreGive( xConTaskStartMtx );
/* Setup WiFi Events */
WiFi.onEvent(WiFiEvent);
WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
WiFi.onEvent(ProcessWifiScan, WiFiEvent_t::SYSTEM_EVENT_SCAN_DONE);

if(forceAP == true ){
    wifi_param_config_t settings = read_wifi_config();
    settings.AP_Mode = true;
    write_wifi_config( settings );
} 


if(false == force_openap ){
  ReconnectWiFi();
} else {
  configureSoftAP(false);
}

   
}

bool ConnetToAP(const char* AP_SSID, const char* AP_PASS ){
  ssid=AP_SSID;
  pass=AP_PASS;
  bool con = false;
  if(true==connectWiFi()){
    configureServer(); 
    /* Save Config */
    credentials_t c;
    strncpy(c.pass,AP_PASS,sizeof(c.pass));
    strncpy(c.ssid,AP_SSID,sizeof(c.ssid));
    write_credentials(c);
    con=true;
 
  } else {
    configureSoftAP(true);
    con = false;
  }  
  return con;
}



/**************************************************************************************************
 *    Function      : connectWiFi
 *    Description   : trys to establish a WiFi connection
 *    Input         : none 
 *    Output        : bool
 *    Remarks       : connect the esp to a wifi network, retuns false if failed
 **************************************************************************************************/
bool connectWiFi( void  ) {
  if( xConnectTaskDoneSem == NULL ){
    /* WiFi not initialized ? */
    return false;
  }
  /* We start the Task and wait till we are done */
  
  wifi_connect_param_t params;
  params.ssid = ssid.c_str();
  params.pass = pass.c_str();
  params.connected = false;
  params.result_cb = NULL;
  params.CallerSemaphore = xConnectTaskDoneSem;
  StartWiFiConnect( &params );
  xSemaphoreTake(  xConnectTaskDoneSem, portMAX_DELAY );
  return params.connected;


}

void ReconnectWiFi ( void ){
  _ReconnectWiFi();
}

/**************************************************************************************************
 *    Function      : ReconnectWiFi
 *    Description   : trys to establish a WiFi connection
 *    Input         : none 
 *    Output        : none
 *    Remarks       : connect the esp to a wifi network
 **************************************************************************************************/
void _ReconnectWiFi( bool ConnectToApOnly  ) {
  static wifi_connect_param_t params;
  if( xConnectTaskDoneSem == NULL ){
    /* WiFi not initialized ? */
    Serial.println("Wifi Sempahore missing");
    return;
  }
  /* We start the Task and wait till we are done */
  WiFiStop(); 
  wifi_param_config_t settings = read_wifi_config();
  if(settings.WIFI_Ena == false){
    Serial.println("Wifi Not Enabled");
    return;
  } else {
    WiFiStart();
  }

  if(settings.AP_Mode == false ){

    credentials_t c =  read_credentials();
    if(0==c.ssid[0]){
      Serial.println("No SSID configured");
    } else {
      ssid=String(c.ssid);
      pass=String(c.pass);
      
      params.ssid = ssid.c_str();
      params.pass = pass.c_str();
      params.connected = false;
      params.result_cb = NULL;
      params.CallerSemaphore = xConnectTaskDoneSem;
      Serial.println("Starting Reconnect");
      StartWiFiConnect( &params );
    }
  } else {
    if(false == ConnectToApOnly){
      configureSoftAP(true);
    } else {
      //Reconnect failed !
    }
  }

}





/**************************************************************************************************
 *    Function      : configureSoftAP
 *    Description   : Configures the ESP as SoftAP
 *    Input         : none 
 *    Output        : none
 *    Remarks       : configure the access point of the esp
 **************************************************************************************************/
void configureSoftAP( bool use_wifi_config ) {
  uint8_t macAddr[6];
  char stringBufferAP[33]; 
  WiFi.mode(WIFI_AP);
  WiFi.softAPmacAddress(macAddr);
  snprintf(stringBufferAP,32,"ESP32 %02x:%02x:%02x",macAddr[3],macAddr[4],macAddr[5]);
  APSSID = stringBufferAP;
  Serial.println("Configuring AP: " + String(APSSID));
  if(use_wifi_config == true ){
    wifi_param_config_t settings = read_wifi_config();
    if(( strnlen(settings.ap_pass, sizeof(settings.ap_pass) >= 8 )) && settings.AP_Enc_Ena == true ){
      Serial.printf("AP Encryption enabled pass %s \n\r",settings.ap_pass);
      WiFi.softAP(APSSID.c_str(), settings.ap_pass, 1, 0, 1);
    } else {
      WiFi.softAP(APSSID.c_str(), NULL, 1, 0, 1);
    }
  } else {
    WiFi.softAP(APSSID.c_str(), NULL, 1, 0, 1);
  }
  
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(ip);
  configureServer();
}


void eventConnectToApFailed( wifi_connect_param_t* params ){
 
 if( params->connected == false ) {
    wifi_param_config_t settings = read_wifi_config();
    if(settings.NoFallbackToAP == false ){ 
      AutoReconnect.detach();
      configureSoftAP( true );
      configureServer();
    } else {
      /* Check if autoreconnect is active and working */
      StartDelayedReconnect();
    }
 }

}


void StartDelayedReconnect( void ){

  AutoReconnect.once_ms( ( 5 * 60 *1000 ) , ReconnectWiFi );

}
/**************************************************************************************************
 *    Function      : WiFigetQuality
 *    Description   : Gets the Signalquality
 *    Input         : none 
 *    Output        : none
 *    Remarks       : Return the quality (Received Signal Strength Indicator)
                      of the WiFi network.
                      Returns a number between 0 and 100 if WiFi is connected.
                      Returns -1 if WiFi is disconnected.
 **************************************************************************************************/
int WiFigetQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

/**************************************************************************************************
 *    Function      : WiFiGetStatus
 *    Description   : Gives a string representing the WiFi status
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
String WiFiGetStatus() {
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
 *    Function      : configureServer
 *    Description   : Configures all Server we need to run
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void configureServer( void ){
  SetupWebServer();
  //We also need to register the custom function extensions
  Webserver_Map_FunctionsRegister(WebserverGetHandle());
  Webserver_Time_FunctionsRegister(WebserverGetHandle());
  Webserver_SenseBox_FunctionsRegister(WebserverGetHandle());
  Webserver_SDCard_FunctionsRegister(WebserverGetHandle());
  Webserver_Thinkspeak_FunctionsRegister(WebserverGetHandle());
  Webserver_MQTT_FunctionsRegister(WebserverGetHandle());

}

/**************************************************************************************************
 *    Function      : NetworkLoopTask
 *    Description   : Configures all Server we need to run
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NetworkLoopTask( void ){
   WebserverLoopTask();
}

uint8_t WiFiGetChannel(void){
 uint8_t primaryChan;
 wifi_second_chan_t secondChan;
 esp_wifi_get_channel(&primaryChan, &secondChan);
 return primaryChan;
}

int16_t WiFiGetRSSI(void){
  int16_t RSSI = -1;
  RSSI = WiFi.RSSI();
  return RSSI;
}

bool RegisterCbWiFiScanDone( void (*cb_ptr)(void) ){
    bool registered = false;
    for(uint32_t i = 0; i < ( sizeof(scandone_cb)/sizeof(scandone_cb[0]) ); i++){    
      if(scandone_cb[i]==NULL){
              scandone_cb[i]=cb_ptr;
              registered=true;
              break;
      }
    }
    return registered;
}

bool DeleteCbWiFiScanDone( void (*cb_ptr)(void) ){
    bool del = false;
    for(uint32_t i = 0; i < ( sizeof(scandone_cb)/sizeof(scandone_cb[0]) ); i++){    
      if(scandone_cb[i]==cb_ptr){
              scandone_cb[i]=NULL;
              del=true;
      }
    }
    return del;

}


uint8_t WiFiRSSItoPercent(int16_t dBm){
if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void GetWiFiConnectionInfo( wifi_connection_info_t* ConnectionInfo){
  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);

  bzero(ConnectionInfo, sizeof(ConnectionInfo));

  if(true ==  WPS_Running ){
    ConnectionInfo->WPS_Active = true;
  } else  {
    ConnectionInfo->WPS_Active = false;
  }

  if(mode == WIFI_MODE_STA){
    ConnectionInfo->IsStation=true;
  } else {
    ConnectionInfo->IsStation=false;
  }

  if( mode == WIFI_MODE_AP){
    ConnectionInfo->IsAP=true;
  } else {
    ConnectionInfo->IsAP=false;
  }

  if(WiFi.status()==WL_CONNECTED){
    ConnectionInfo->Connected = true;
  } else {
    ConnectionInfo->Connected = false;
  } 
  /* Next is to fetch the IP Information and WiFi Informaion */
  if( (ConnectionInfo->IsAP==true) || (ConnectionInfo->Connected==true) ){
    ConnectionInfo->IP_Info.SystemIP = WiFi.localIP();
    ConnectionInfo->IP_Info.SystemGW = WiFi.gatewayIP();
    ConnectionInfo->IP_Info.SystemSUBNET = WiFi.subnetMask();
    ConnectionInfo->IP_Info.SystemPriDNS = WiFi.dnsIP(0);
    ConnectionInfo->IP_Info.SystemSecDNS = WiFi.dnsIP(1);

    ConnectionInfo->AP_Info.SSID = WiFi.SSID();
    ConnectionInfo->AP_Info.BSSID = WiFi.BSSIDstr();
    ConnectionInfo->AP_Info.CHANNEL = WiFi.channel();

    /* Get AP Information */
    if(ConnectionInfo->IsAP==true){

      ConnectionInfo->AP_Info.RSSI = 0;
 
    } else {

      ConnectionInfo->AP_Info.RSSI = WiFi.RSSI();
    }

    

  } else {
    /* No IP Info */

  }


}


bool RequestWiFiConnection( void ){ //This will give a semaphore as the connection is needed
  //We need to also lock this sequence.....
  if( xSemaphoreGive( xConnections ) != pdTRUE ){
    //Strange 
  } 
  //We put our lock on the wifi :-)
  if( WiFi.status() != WL_CONNECTED ){
    //WiFi is not connected so we need to reconnect it if possible...
    _ReconnectWiFi(true);
    if( WiFi.status() != WL_CONNECTED ){
      if(false ==  xSemaphoreTake( xConnections, ( TickType_t ) 0 ) )
      {
        //Strange and should not happen!
      }
      return false; //Can't connect!
    }
    return true;
  }
  return true;
}

void ReleaseWiFiConnection( void ){ //This will remove a semaphore as the connection can be shut down
      
      if(false ==  xSemaphoreTake( xConnections, ( TickType_t ) 0 ) )
      {
        //Strange and should not happen!
      }
      if(uxSemaphoreGetCount( xConnections) <= 0 ){
        //We can shutdown the WiFi for now ....
        if(false != StarStopeMode ){
          WiFiStop();
          WiFiForceSleep();
        }
      }
         
}


void EnableStartStopMode( bool ena ){
  StarStopeMode=ena;
  if(false == ena) {
    ReconnectWiFi();
  } else {
    if(uxSemaphoreGetCount( xConnections) <= 0 ){
        //We can shutdown the WiFi for now ....
        WiFiStop();
        WiFiForceSleep();
    }
  }
  
}