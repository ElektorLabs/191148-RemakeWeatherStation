#ifndef __MQTTT_TASK_H__
 #define __MQTTT_TASK_H__

 #include "../../datastore.h" //May this also is moved to a file in JSON format on SPIFFS
 void MQTTTaskStart( void ) ;
 void MQTTRegisterMappingAccess(VALUEMAPPING* Mp); 
 mqttsettings_t GetMQTTSettings( void );
#endif