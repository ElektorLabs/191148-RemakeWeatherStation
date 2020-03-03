#include <ArduinoJson.h>
#include <AsyncUDP.h>
#include "UDP_UniCastServer.h"

 void UDPUniCastSever::begin( void ){
       xTaskCreate(
                    UDPUniCastSever::UDPServerTask,          /* Task function. */
                    "UPPServerTask",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    (void*)(this),             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
 }
 

/**************************************************************************************************
 *    Function      : MQTTRegisterMappingAccess
 *    Description   : This registers a function to get access to the mapped values
 *    Input         : VALUEMAPPING* 
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void UDPUniCastSever::RegisterMappingAccess(VALUEMAPPING* Mp){
  Mapping = Mp;
}


 void UDPUniCastSever::SendData( void ){
    if(nullptr == Mapping){
        return;
    }
    DynamicJsonDocument  root(4096);
    String JsonString="";
    root.clear();
    JsonArray data = root.createNestedArray("data");
    for(uint8_t i=0;i<64;i++){
        float value = NAN;
        if(false == Mapping->ReadMappedValue(&value,i)){
            Serial.printf("UDP Channel %i not mapped\n\r",i);
        } else {
        Serial.printf("UDP Channel %i Value %f",i,value );
        String name = Mapping->GetSensorNameByChannel(i);
        Serial.println(name);
        JsonObject dataobj = data.createNestedObject();
        dataobj["channel"] = i;
        dataobj["value"] = value;
        dataobj["name"] = name;

        }
    }
    serializeJson(root,JsonString);

     //udp.sendto()
     //We use broadcast instead
     udp.broadcastTo( JsonString.c_str(), 1234);
 }
   

 void UDPUniCastSever::SetDestIPAddeessV4( uint8_t A3, uint8_t A2, uint8_t A1 , uint8_t A0){
     DestAddr[0] = A0;
     DestAddr[1] = A1;
     DestAddr[2] = A2;
     DestAddr[3] = A3;
         
 }
 
 void UDPUniCastSever::GetDestIPAddeessV4( uint8_t* A3, uint8_t* A2, uint8_t* A1, uint8_t* A0){
     *A0 = DestAddr[0];
     *A1 = DestAddr[1];
     *A2 = DestAddr[2];
     *A3 = DestAddr[3];
     
 }
 
 void UDPUniCastSever::SetUDPPort( uint16_t nPort){
            Port=nPort;
 }
 
 uint16_t UDPUniCastSever::GetUDPPort( void ){
            return Port;
 }
 
 void UDPUniCastSever::SetTXINtervall(uint16_t Minutes){
            Intervall= Minutes;
 }
 
 uint16_t UDPUniCastSever::GetUpdateIntervall( void ){
            return Intervall;
 }
     /* Mapping is not implemented and this is running a static configuration! */
void UDPUniCastSever::UDPServerTask( void * task_obj){

    Serial.println("Start UPD Server Task");
    UDPUniCastSever* pObj = (UDPUniCastSever*)(task_obj); 
    //We should now be able to access the object functions and varables with the pointer
    while ( 1 == 1 ){
        uint16_t intervall = pObj->GetUpdateIntervall();
        if( intervall < 1){
            intervall = 1;
        }
        vTaskDelay( (60*1000*intervall) ); //ticks delay 
        pObj->SendData();

    }
    

 }