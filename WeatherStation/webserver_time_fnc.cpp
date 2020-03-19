
#include <ArduinoJson.h>


#include "datastore.h"

#include "webserver_base.h"
#include "webserver_time_fnc.h"


NTP_Client * NTPCPtr=nullptr;
Timecore* TimeCorePtr=nullptr;

WebServer* WebTimeSvr=nullptr;

void timesettings_send(void);
void settime_update(void);
void ntp_settings_update(void);
void timezone_update(void);
void timezone_overrides_update(void);

//As we have a kind of time service with NTP arround we also need to reimplement the functions to hanlde it
void Webserver_Time_FunctionsRegister(WebServer* server){
  WebTimeSvr=server;
  if(WebTimeSvr==nullptr){
    abort();
  }
  WebTimeSvr->on("/timesettings", HTTP_GET, timesettings_send);
  WebTimeSvr->on("/settime.dat", HTTP_POST, settime_update); /* needs to process date and time */
  WebTimeSvr->on("/ntp.dat",HTTP_POST,ntp_settings_update); /* needs to process NTP_ON, NTPServerName and NTP_UPDTAE_SPAN */
  WebTimeSvr->on("/timezone.dat",timezone_update); /*needs to handel timezoneid */
  WebTimeSvr->on("/overrides.dat",timezone_overrides_update); /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */

}

void Webserver_Time_FunctionRegisterTimecore(Timecore* TcPtr){
  TimeCorePtr=TcPtr;
}
void Webserver_Time_FunctionRegisterNTPClient(NTP_Client* NtpCPtr){
  NTPCPtr=NtpCPtr;
}



/**************************************************************************************************
*    Function      : response_settings
*    Description   : Sends the timesettings as json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void timesettings_send(){

char strbuffer[129];
String response="";  

if( (TimeCorePtr==nullptr) || (NTPCPtr==nullptr) ){
  WebTimeSvr->send(500);
  return;
}

  DynamicJsonDocument root(350);
  memset(strbuffer,0,129);
  datum_t d = TimeCorePtr->GetLocalTimeDate();
  snprintf(strbuffer,64,"%02d:%02d:%02d",d.hour,d.minute,d.second);
  
  root["time"] = strbuffer;
 
  memset(strbuffer,0,129);
  snprintf(strbuffer,64,"%04d-%02d-%02d",d.year,d.month,d.day);
  root["date"] = strbuffer;

  memset(strbuffer,0,129);
  snprintf(strbuffer,129,"%s",NTPCPtr->GetServerName());
  root["ntpname"] = strbuffer;
  root["tzidx"] = (int32_t)TimeCorePtr->GetTimeZone();
  root["ntpena"] = NTPCPtr->GetNTPSyncEna();
  root["ntp_update_span"]=NTPCPtr->GetSyncInterval();
  root["zoneoverride"]=TimeCorePtr->GetTimeZoneManual();;
  root["gmtoffset"]=TimeCorePtr->GetGMT_Offset();;
  root["dlsdis"]=!TimeCorePtr->GetAutomacitDLS();
  root["dlsmanena"]=TimeCorePtr->GetManualDLSEna();
  uint32_t idx = TimeCorePtr->GetDLS_Offset();
  root["dlsmanidx"]=idx;
  serializeJson(root,response);
  WebTimeSvr->send(200, "text/plain", response);
}


/**************************************************************************************************
*    Function      : settime_update
*    Description   : Parses POST for new local time
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void settime_update( ){ /* needs to process date and time */
  datum_t d;
  d.year=2000;
  d.month=1;
  d.day=1;
  d.hour=0;
  d.minute=0;
  d.second=0;

  bool time_found=false;
  bool date_found=false;
  
  if( (TimeCorePtr==nullptr) || (NTPCPtr==nullptr) ){
    WebTimeSvr->send(500);
    return;
  }

  if( ! WebTimeSvr->hasArg("date") || WebTimeSvr->arg("date") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
   
    Serial.printf("found date: %s\n\r",WebTimeSvr->arg("date").c_str());
    uint8_t d_len = WebTimeSvr->arg("date").length();
    Serial.printf("datelen: %i\n\r",d_len);
    if(WebTimeSvr->arg("date").length()!=10){
      Serial.println("date len failed");
    } else {   
      String year=WebTimeSvr->arg("date").substring(0,4);
      String month=WebTimeSvr->arg("date").substring(5,7);
      String day=WebTimeSvr->arg("date").substring(8,10);
      d.year = year.toInt();
      d.month = month.toInt();
      d.day = day.toInt();
      date_found=true;
    }   
  }

  if( ! WebTimeSvr->hasArg("time") || WebTimeSvr->arg("time") == NULL ) { // If the POST request doesn't have username and password data
    
  } else {
    if(WebTimeSvr->arg("time").length()!=8){
      Serial.println("time len failed");
    } else {
    
      String hour=WebTimeSvr->arg("time").substring(0,2);
      String minute=WebTimeSvr->arg("time").substring(3,5);
      String second=WebTimeSvr->arg("time").substring(6,8);
      d.hour = hour.toInt();
      d.minute = minute.toInt();
      d.second = second.toInt();     
      time_found=true;
    }
     
  } 
  if( (time_found==true) && ( date_found==true) ){
    Serial.printf("Date: %i, %i, %i ", d.year , d.month, d.day );
    Serial.printf("Time: %i, %i, %i ", d.hour , d.minute, d.second );
    TimeCorePtr->SetLocalTime(d);
  }
  
  WebTimeSvr->send(200);   
 
 }


/**************************************************************************************************
*    Function      : ntp_settings_update
*    Description   : Parses POST for new ntp settings
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void ntp_settings_update( ){ /* needs to process NTP_ON, NTPWebTimeSvrName and NTP_UPDTAE_SPAN */

    if( (TimeCorePtr==nullptr) || (NTPCPtr==nullptr) ){
      WebTimeSvr->send(500);
      return;
    }

  if( ! WebTimeSvr->hasArg("NTP_ON") || WebTimeSvr->arg("NTP_ON") == NULL ) { // If the POST request doesn't have username and password data
    NTPCPtr->SetNTPSyncEna(false);  
  } else {
    NTPCPtr->SetNTPSyncEna(true);  
  }

  if( ! WebTimeSvr->hasArg("NTPServerName") || WebTimeSvr->arg("NTPServerName") == NULL ) { // If the POST request doesn't have username and password data
      
  } else {
     NTPCPtr->SetServerName( WebTimeSvr->arg("NTPServerName") );
  }

  if( ! WebTimeSvr->hasArg("ntp_update_delta") || WebTimeSvr->arg("ntp_update_delta") == NULL ) { // If the POST request doesn't have username and password data
     
  } else {
    NTPCPtr->SetSyncInterval( WebTimeSvr->arg("ntp_update_delta").toInt() );
  }
  NTPCPtr->SaveSettings();
  WebTimeSvr->send(200);   
  
 }

/**************************************************************************************************
*    Function      : timezone_update
*    Description   : Parses POST for new timezone settings
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
void timezone_update( ){ /*needs to handel timezoneid */
  if( (TimeCorePtr==nullptr) || (NTPCPtr==nullptr) ){
    WebTimeSvr->send(500);
    return;
  }

  if( ! WebTimeSvr->hasArg("timezoneid") || WebTimeSvr->arg("timezoneid") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
    #ifdef DEBUG_SERIAL
    Serial.printf("New TimeZoneID: %s\n\r",WebTimeSvr->arg("timezoneid").c_str());
    #endif
    uint32_t timezoneid = WebTimeSvr->arg("timezoneid").toInt();
    TimeCorePtr->SetTimeZone( (TIMEZONES_NAMES_t)timezoneid );   
  }
  TimeCorePtr->SaveConfig();
  WebTimeSvr->send(200);    

 }

 
/**************************************************************************************************
*    Function      : timezone_overrides_update
*    Description   : Parses POST for new timzone overrides
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
 void timezone_overrides_update( ){ /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */

  if( (TimeCorePtr==nullptr) || (NTPCPtr==nullptr) ){
    WebTimeSvr->send(500);
    return;
  }


  bool DLSOverrid=false;
  bool ManualDLS = false;
  bool ZONE_OVERRRIDE = false;
  int32_t gmt_offset = 0;
  DLTS_OFFSET_t dls_offsetidx = DLST_OFFSET_0;
  if( ! WebTimeSvr->hasArg("dlsdis") || WebTimeSvr->arg("dlsdis") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    DLSOverrid=true;  
  }

  if( ! WebTimeSvr->hasArg("dlsmanena") || WebTimeSvr->arg("dlsmanena") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ManualDLS=true;  
  }

  if( ! WebTimeSvr->hasArg("ZONE_OVERRRIDE") || WebTimeSvr->arg("ZONE_OVERRRIDE") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ZONE_OVERRRIDE=true;  
  }

  if( ! WebTimeSvr->hasArg("gmtoffset") || WebTimeSvr->arg("gmtoffset") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    gmt_offset = WebTimeSvr->arg("gmtoffset").toInt();
  }

  if( ! WebTimeSvr->hasArg("dlsmanidx") || WebTimeSvr->arg("dlsmanidx") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    dls_offsetidx = (DLTS_OFFSET_t) WebTimeSvr->arg("dlsmanidx").toInt();
  }
  TimeCorePtr->SetGMT_Offset(gmt_offset);
  TimeCorePtr->SetDLS_Offset( (DLTS_OFFSET_t)(dls_offsetidx) );
  TimeCorePtr->SetAutomaticDLS(!DLSOverrid);
  TimeCorePtr->SetManualDLSEna(ManualDLS);
  TimeCorePtr->SetTimeZoneManual(ZONE_OVERRRIDE);

 
  TimeCorePtr->SaveConfig();
  WebTimeSvr->send(200);    

  
 }




