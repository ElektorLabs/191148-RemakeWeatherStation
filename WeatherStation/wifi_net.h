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
  bool WPS_Active;
  wifi_scan_result_t AP_Info;
  v4_ip_info_t IP_Info;
} wifi_connection_info_t;


  /**************************************************************************************************
 *    Function      : initWiFi
 *    Description   : Will initialize the WiFi
 *    Input         : bool , bool 
 *    Output        : none
 *    Remarks       : User can force WiFi On or AP Mode
 **************************************************************************************************/ 
  void initWiFi( bool forceAP = false ,bool force_openap = false);

  /**************************************************************************************************
 *    Function      : WPS_Start
 *    Description   : Will start a WPS Join ( Button Mode )
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
  void WPS_Start( void );

  /**************************************************************************************************
 *    Function      : WiFiClientEnable
 *    Description   : This will force the WiFi in ClientMode
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void WiFiClientEnable(bool Ena);

  /**************************************************************************************************
 *    Function      : WiFiForceAP
 *    Description   : This will force the WiFi into AP Config
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void WiFiForceAP( bool Ena);

  /**************************************************************************************************
 *    Function      : WiFiStop
 *    Description   : This will disable WiFI
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void WiFiStop( void );

  /**************************************************************************************************
 *    Function      : WiFiStart
 *    Description   : This will enable WiFi
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
  void WiFiStart( void );

  /**************************************************************************************************
 *    Function      : WiFiForceSleep
 *    Description   : This will put WiFi into SleepMode 
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void WiFiForceSleep( void );

  /**************************************************************************************************
 *    Function      : WiFiGetChannel
 *    Description   : Return the currently used WiFi Channel
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  uint8_t WiFiGetChannel(void);

  /**************************************************************************************************
 *    Function      : WiFiGetRSSI
 *    Description   : This will return the RSSI in dBm
 *    Input         : none
 *    Output        : int16_t
 *    Remarks       : none
 **************************************************************************************************/
  int16_t WiFiGetRSSI(void);

  /**************************************************************************************************
 *    Function      : ReScanNetworks
 *    Description   : this will scan for WiFi Networks
 *    Input         : bool, bool
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
  void ReScanNetworks(bool WaitForReady = false, bool ScanForHiddenNetworks=false);

  /**************************************************************************************************
 *    Function      : LastScanWiFiNetworksFound
 *    Description   : Will return the result for the last WiFi Scan
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  uint16_t LastScanWiFiNetworksFound(  void );

  /**************************************************************************************************
 *    Function      : GetScanResultOfIDX
 *    Description   : This will give a result of the Scanlist
 *    Input         : wifi_scan_result_t* , uint8_t
 *    Output        : bool
 *    Remarks       : none
 **************************************************************************************************/
  bool GetScanResultOfIDX( wifi_scan_result_t* result, uint8_t index);

  /**************************************************************************************************
 *    Function      : NetworkLoopTask
 *    Description   : Keeps the Network services going
 *    Input         : none
 *    Output        : none
 *    Remarks       : needs to be called as often as possible
 **************************************************************************************************/
  void NetworkLoopTask( void );

  /**************************************************************************************************
 *    Function      : EnableStartStopMode
 *    Description   : StartStop Mode for WiFi
 *    Input         : bool
 *    Output        : none
 *    Remarks       : If enabled WiFi will be switched off if no longer needed
 *                    also Web access won't work any longer if WiFi is disabled
 **************************************************************************************************/
  void EnableStartStopMode( bool ena );

/**************************************************************************************************
 *    Function      : ReconnectWiFi
 *    Description   : Trys to reestablish a WiFi connection  
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void ReconnectWiFi ( void );

  /**************************************************************************************************
 *    Function      : _ReconnectWiFi
 *    Description   : Try a reconnect and can optinally not fall back to AP mode 
 *    Input         : none
 *    Output        : none
 *    Remarks       : internal function, but sometimes handy
 **************************************************************************************************/
  void _ReconnectWiFi( bool ConnectToApOnly = false  ); //Generic function to restart WiFi

/**************************************************************************************************
 *    Function      : RegisterCbWiFiScanDone
 *    Description   : Will take a Function to be called if a WiFi Scan is done
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
  bool RegisterCbWiFiScanDone( void (*cb_ptr)(void) );

  /**************************************************************************************************
 *    Function      : DeleteCbWiFiScanDone
 *    Description   : This can delete a callback for WiFi Scan done
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  bool DeleteCbWiFiScanDone( void (*cb_ptr)(void) );

/**************************************************************************************************
 *    Function      : RequestWiFiConnection
 *    Description   : If in StartStop mode this will try to establis a WiFi Connection again
 *    Input         : none
 *    Output        : none
 *    Remarks       : Used to inform the system that someone is using the WiFi
 **************************************************************************************************/
  bool RequestWiFiConnection( void ); //This will give a semaphore as the connection is needed

  /**************************************************************************************************
 *    Function      : ReleaseWiFiConnection
 *    Description   : Informs the System that someone won't need WiFi anymore
 *    Input         : none
 *    Output        : none
 *    Remarks       : This must be called after WiFi is claimed with RequestWiFiConnection
 **************************************************************************************************/
  void ReleaseWiFiConnection( void ); //This will remove a semaphore as the connection can be shut down

/**************************************************************************************************
 *    Function      : ConnetToAP
 *    Description   : This will connect to a given SSID
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  bool ConnetToAP(const char* AP_SSID, const char* AP_PASS ); //Blocking Connect / Reconnect to a AP

  /**************************************************************************************************
 *    Function      : StartWiFiConnect
 *    Description   : none
 *    Input         : wifi_connect_param_t*
 *    Output        : none
 *    Remarks       : This will start a connection process with given parameter
 **************************************************************************************************/ 
  void StartWiFiConnect( wifi_connect_param_t* parameter );

  /**************************************************************************************************
 *    Function      : GetWiFiConnectionInfo
 *    Description   : Get the current connection information
 *    Input         : wifi_connection_info_t*
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  void GetWiFiConnectionInfo( wifi_connection_info_t* ConnectionInfo);

  /**************************************************************************************************
 *    Function      : WiFiRSSItoPercent
 *    Description   : Will calcualte a connection status in % from RSSI value
 *    Input         : int16_t
 *    Output        : nouint8_t
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
  uint8_t WiFiRSSItoPercent(int16_t dBm);

  /**************************************************************************************************
 *    Function      : WiFigetQuality
 *    Description   : Returns a quality indeicator from 0-4 with -1 meaning not connected
 *    Input         : none
 *    Output        : int
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
  int WiFigetQuality( void );

/**************************************************************************************************
 *    Function      : RegisterWiFiConnectedCB
 *    Description   : Will take a Function to be called if a WiFi Scan is done
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
  bool RegisterWiFiConnectedCB( void (*cb_ptr)(void) );

  /**************************************************************************************************
 *    Function      : DeleteWiFiConnectedCB
 *    Description   : This can delete a callback for WiFi Scan done
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
  bool DeleteWiFiConnectedCB( void (*cb_ptr)(void) );


#endif
