#ifndef __WIFI_NWT_H__
#define __WIFI_NWT_H__
  #include <Arduino.h>

  typedef struct{
     String SSID; 
     String BSSID;
     uint8_t CHANNEL;
     char* ENCRYPTION;
     int16_t RSSI;
  } wifi_scan_result_t;

typedef struct {
  const char* ssid;
  const char* pass;
  bool connected;
  void (*result_cb) (bool);
  SemaphoreHandle_t CallerSemaphore;
} wifi_connect_param_t;

typedef struct {
  IPAddress SystemIP;
  IPAddress SystemGW;
  IPAddress SystemSUBNET;
  IPAddress SystemPriDNS;
  IPAddress SystemSecDNS;
} v4_ip_info_t;

typedef struct {
  bool Connected;
  bool IsStation;
  bool IsAP;
  wifi_scan_result_t AP_Info;
  v4_ip_info_t IP_Info;
} wifi_connection_info_t;
   
  void initWiFi( bool forceAP = false ,bool force_openap = false);
  void WiFiClientEnable(bool Ena);
  void WiFiForceAP( bool Ena);
  void WiFiStop( void );
  void WiFiStart( void );
  void WiFiForceSleep( void );
  uint8_t WiFiGetChannel(void);
  int16_t WiFiGetRSSI(void);
  void ReScanNetworks(bool WaitForReady = false, bool ScanForHiddenNetworks=false);
  uint16_t LastScanWiFiNetworksFound(  void );
  bool GetScanResultOfIDX( wifi_scan_result_t* result, uint8_t index);
  void NetworkLoopTask( void );
  void EnableStartStopMode( bool ena );

  void ReconnectWiFi ( void );
  void _ReconnectWiFi( bool ConnectToApOnly = false  ); //Generic function to restart WiFi

  bool RegisterCbWiFiScanDone( void (*cb_ptr)(void) );
  bool DeleteCbWiFiScanDone( void (*cb_ptr)(void) );

  bool RequestWiFiConnection( void ); //This will give a semaphore as the connection is needed
  void ReleaseWiFiConnection( void ); //This will remove a semaphore as the connection can be shut down


  bool ConnetToAP(const char* AP_SSID, const char* AP_PASS ); //Blocking Connect / Reconnect to a AP 
  void StartWiFiConnect( wifi_connect_param_t* parameter );
  void GetWiFiConnectionInfo( wifi_connection_info_t* ConnectionInfo);
  uint8_t WiFiRSSItoPercent(int16_t dBm);
  int WiFigetQuality( void );



#endif
