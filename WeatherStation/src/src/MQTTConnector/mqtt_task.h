#ifndef __MQTTT_TASK_H__
 #define __MQTTT_TASK_H__

 #include "../ValueMapping/ValueMapping.h"
 void MQTTTaskStart( void ) ;
 void MQTTRegisterMappingAccess(VALUEMAPPING* Mp); 
 mqttsettings_t GetMQTTSettings( void );
 TaskHandle_t GetMQTTTaskHandle( void );
#endif