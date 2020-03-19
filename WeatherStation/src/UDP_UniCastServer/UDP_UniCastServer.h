#ifndef __UDPUNICASTSERVER_H__
 #define __UDPUNICASTSERVER_H__

 #include "../ValueMapping/ValueMapping.h"
 
 class UDPUniCastSever{
     public:
    
     void begin( void );
     void SetDestIPAddeessV4( uint8_t A3, uint8_t A2, uint8_t A1 , uint8_t A0);
     void GetDestIPAddeessV4( uint8_t* A3, uint8_t* A2, uint8_t* A1, uint8_t* A0);
     void SetUDPPort( uint16_t Port);
     uint16_t GetUDPPort( void );
     void SetTXInervall(uint16_t Minutes);
     uint16_t GetUpdateIntervall( void );
     /* Mapping is not implemented and this is running a static configuration! */
     void RegisterMappingAccess(VALUEMAPPING* Mp);
     void SendData( void );
     private:
     
     uint8_t DestAddr[4]={255,255,255,255};
     uint16_t Port = 666;
     uint16_t Intervall = 1;
     AsyncUDP udp;
     VALUEMAPPING* Mapping=nullptr;

    static void UDPServerTask( void * task_obj);
    //This will also server for the WebServer Dashboard as they will share some kind of commocm config maybe

 };
 
#endif