#include <Preferences.h>
#include "datastore.h"

/* function prototypes */
void nvswrite_struct(void* data_in, uint32_t e_size, const char* key  );
bool nvsread_struct( void* element, uint32_t e_size, const char* key   );


/* This will be used as keys for the NVS */
#define WIFI_CONFIG_START "WIFICONF"

#define CREDENTIALS_START "CREDENTIALS"

#define NTP_START "NTPSETTINGS"

#define NOTES_START "NOTES"

#define MQTT_START "MQTTSETTINGS"

#define TEMP_SENSOR_START "ONEWIRE"

#define TIMECORECONFIG_START "TIMECORECONFIG"


Preferences prefs;
/**************************************************************************************************
 *    Function      : datastoresetup
 *    Description   : Gets the EEPROM Emulation set up
 *    Input         : none 
 *    Output        : none
 *    Remarks       : We use 4096 byte for EEPROM 
 **************************************************************************************************/
void datastoresetup() {
  uint8_t buffer[514];
  /* We emulate 4096 byte here */
  prefs.begin("DATASTORE"); // use "DATASTORE" namespace

  read_wifi_config();
  read_ntp_config();
  read_credentials();
  read_notes(buffer,512);
  read_mqttsettings();
  
  

}

/**************************************************************************************************
 *    Function      : write_wifi_config
 *    Description   : writes the ntp config
 *    Input         : ntp_config_t c 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void write_wifi_config(wifi_param_config_t c){
 nvswrite_struct( ( (void*)(&c) ), sizeof(wifi_param_config_t) , WIFI_CONFIG_START );    
}


/**************************************************************************************************
 *    Function      : read_wifi_config
 *    Description   : writes the ntp config
 *    Input         : none
 *    Output        : ntp_config_t
 *    Remarks       : none
 **************************************************************************************************/
wifi_param_config_t read_wifi_config( void ){
  wifi_param_config_t retval;
  if(false ==  nvsread_struct( (void*)(&retval), sizeof(wifi_param_config_t) , WIFI_CONFIG_START ) ){
    Serial.println("WIFI CONF");
    bzero((void*)&retval,sizeof( wifi_param_config_t ));
    write_wifi_config(retval);
  } 
  return retval;
}

/**************************************************************************************************
 *    Function      : write_ntp_config
 *    Description   : writes the ntp config
 *    Input         : ntp_config_t c 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void write_ntp_config(ntp_config_t c){
  nvswrite_struct( ( (void*)(&c) ), sizeof(ntp_config_t) , NTP_START );    
}

/**************************************************************************************************
 *    Function      : read_ntp_config
 *    Description   : writes the ntp config
 *    Input         : none
 *    Output        : ntp_config_t
 *    Remarks       : none
 **************************************************************************************************/
ntp_config_t read_ntp_config( void ){
  ntp_config_t retval;
  if(false ==  nvsread_struct( (void*)(&retval), sizeof(ntp_config_t) , NTP_START ) ){
    Serial.println("NTP CONF");
    bzero((void*)&retval,sizeof( ntp_config_t ));
    write_ntp_config(retval);
  } 
  return retval;
}


/**************************************************************************************************
 *    Function      : write_credentials
 *    Description   : writes the wifi credentials
 *    Input         : credentials_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_credentials(credentials_t c){
   nvswrite_struct( ( (void*)(&c) ), sizeof(credentials_t) , CREDENTIALS_START );
}

/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the wifi credentials
 *    Input         : none
 *    Output        : credentials_t
 *    Remarks       : none
 **************************************************************************************************/
credentials_t read_credentials( void ){
  credentials_t retval;
  if(false == nvsread_struct( (void*)(&retval), sizeof(credentials_t) , CREDENTIALS_START ) ){ 
    Serial.println("WIFI CONF");
    bzero((void*)&retval,sizeof( credentials_t ));
    write_credentials(retval);
  }
  return retval;
}



/**************************************************************************************************
 *    Function      : nvsread_struct
 *    Description   : reads a given block from flash / eeprom 
 *    Input         : void* element, uint32_t e_size, uint32_t startaddr  
 *    Output        : bool ( true if read was okay )
 *    Remarks       : Reads a given datablock into flash and checks the the crc32 
 **************************************************************************************************/
bool nvsread_struct( void* element, uint32_t e_size, const char* key  ){
  bool done = false;
  Serial.println("Read NVS");
  if( e_size != prefs.getBytes(key, element, e_size) ){
    done = false;
  } else {
    done = true;
  }
  return done;
}

/**************************************************************************************************
 *    Function      : nvswrite_struct
 *    Description   : writes the display settings
 *    Input         : void* data, uint32_t e_size, uint32_t address 
 *    Output        : bool
 *    Remarks       : Writes a given datablock into flash and adds a crc32 
 **************************************************************************************************/
void nvswrite_struct(void* data_in, uint32_t e_size,const char* key ){
  uint32_t data_written = prefs.putBytes(key, data_in, e_size);
  if( e_size !=  data_written ){
    /* Write has failed ! */
    Serial.printf("Write Error NVS, Bytes written %u, Bytes to write %u \n\r", data_written, e_size);
  } else {
    /* Write done */
    Serial.println("Write NVS");
  }

}


/**************************************************************************************************
 *    Function      : write_notes
 *    Description   : writes the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_notes(uint8_t* data, uint32_t size){
  
  nvswrite_struct(data,size,NOTES_START);
}


/**************************************************************************************************
 *    Function      : read_notes
 *    Description   : reads the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void read_notes(uint8_t* data, uint32_t size){
  if(size>501){
    size=501;
  }
  
  if( false == (nvsread_struct( data, size, NOTES_START  ) ) ){
   Serial.println("Notes corrutp");
   bzero(data,size);
   write_notes(data,size);
  }
   
  return;
}


/**************************************************************************************************
 *    Function      : write_mqttsettings
 *    Description   : write the mqtt settings
 *    Input         : mqttsettings_t data
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_mqttsettings(mqttsettings_t data){
  nvswrite_struct( ( (void*)(&data) ), sizeof(mqttsettings_t) , MQTT_START );
}

/**************************************************************************************************
 *    Function      : read_mqttsettings
 *    Description   : reads the mqtt settings
 *    Input         : none
 *    Output        : mqttsettings_t
 *    Remarks       : none
 **************************************************************************************************/
mqttsettings_t read_mqttsettings( void ){
  
  mqttsettings_t retval;
  if(false == nvsread_struct( (void*)(&retval), sizeof(mqttsettings_t) , MQTT_START ) ){ 
    Serial.println("MQTT CONF");
    bzero((void*)&retval,sizeof( mqttsettings_t ));
    write_mqttsettings(retval);
  }
  return retval;

}


/**************************************************************************************************
 *    Function      : write_temp_sensor_settings
 *    Description   : reads the mqtt settings
 *    Input         : temp_sensorconfig_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void  write_temp_sensor_settings( temp_sensorconfig_t data ){
  nvswrite_struct( ( (void*)(&data) ), sizeof(temp_sensorconfig_t) , TEMP_SENSOR_START );
}

/**************************************************************************************************
 *    Function      : read_temp_sensor_settings
 *    Description   : reads the mqtt settings
 *    Input         : none
 *    Output        : temp_sensorconfig_t
 *    Remarks       : none
 **************************************************************************************************/
temp_sensorconfig_t read_temp_sensor_settings( void ){

  temp_sensorconfig_t retval;
  if(false == nvsread_struct( (void*)(&retval), sizeof(temp_sensorconfig_t) , TEMP_SENSOR_START ) ){ 
    Serial.println("TEMP SENSOR CONF");
    bzero((void*)&retval,sizeof( temp_sensorconfig_t ));
    write_temp_sensor_settings(retval);
  }
  return retval;

}

/**************************************************************************************************
 *    Function      : write_timecoreconf
 *    Description   : writes the time core config
 *    Input         : timecoreconf_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_timecoreconf(timecoreconf_t c){
  nvswrite_struct( ( (void*)(&c) ), sizeof(timecoreconf_t) , TIMECORECONFIG_START );  
}


/**************************************************************************************************
 *    Function      : read_timecoreconf
 *    Description   : reads the time core config
 *    Input         : none
 *    Output        : timecoreconf_t
 *    Remarks       : none
 **************************************************************************************************/
timecoreconf_t read_timecoreconf( void ){
  timecoreconf_t retval;
  if(false == nvsread_struct( (void*)(&retval), sizeof(timecoreconf_t) , TIMECORECONFIG_START ) ){ 
    Serial.println("TIME CONF");
    retval = Timecore::GetDefaultConfig();
    write_timecoreconf(retval);
  }
  return retval;
}

/**************************************************************************************************
 *    Function      : datastore_erase
 *    Description   : writes the whole EEPROM with 0xFF  
 *    Input         : none
 *    Output        : none
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
void datastore_erase( void ){
  prefs.clear();
}


