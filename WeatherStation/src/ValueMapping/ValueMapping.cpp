#include "Valuemapping.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "../I2C_Sensors/i2c_sensors.h"


VALUEMAPPING::VALUEMAPPING( void ){
    
    for(uint8_t i=0;i< ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) ; i++   ){
         MappingTable[i].Bus=NOTMAPPED;
    }
}

VALUEMAPPING::~VALUEMAPPING( void ){

}

void VALUEMAPPING::RegisterI2CBus(I2C_Sensors* Sensors ){
    I2CSensorBus = Sensors;
}

void VALUEMAPPING::RegisterUartSensors(UART_PM_Sensors* Sensors){
    PMSensors=Sensors;
}

void VALUEMAPPING::RegisterOneWire( void){
    //Not supported yet
}



void VALUEMAPPING::RegisterInternalSensors(InternalSensors* Sensors){
    IntSensors = Sensors;
}

void VALUEMAPPING::begin( void ){

    //We collect all know bus devices here
    //All connected busses must be register in bevore

    ReadConfig();

}

uint8_t VALUEMAPPING::GetConnectedSensors( SensorElementEntry_t* List, uint8_t capacity ){
    InternalSensors::SensorUnitInfo_t IntSensorList[15];
    I2C_Sensors::SensorUnitInfo_t I2CSensorList[30];
    UART_PM_Sensors::SensorUnitInfo_t PMSensorList[6];
    uint8_t IntSensorListElementCnt=0;
    uint8_t I2CSensorListElementCnt=0;
    uint8_t PMSensorListElementCnt=0;
    
    uint8_t used_capacity = 0;


    //We collect all sensors we can get
    if(IntSensors != nullptr ){
        
        if(true == IntSensors->GetConnectedSensorList(IntSensorList, ( sizeof( IntSensorList ) / sizeof( IntSensorList[0] ) )  ,&IntSensorListElementCnt, true)){
            Serial.printf("Collected %i INTERNAL Sensors\n\r",IntSensorListElementCnt);
        } else {
            //OutOfMemory
            Serial.println("INTERNAL SENSORLIST OUT OF MEMORY");
        }
    }
    
    if(I2CSensorBus != nullptr){
        //Collect all I2C Sensors
        if(true == I2CSensorBus->GetConnectedSensorList(I2CSensorList, ( sizeof( I2CSensorList ) / sizeof( I2CSensorList[0] ) ), &I2CSensorListElementCnt, true ) ){
            //Calculate the remaining capacity
           Serial.printf("Collected %i I2C Sensors\n\r",I2CSensorListElementCnt);
        } else {
           //OutOfMemory
           Serial.println("I2C SENSORLIST OUT OF MEMORY");
        }
    }

    if(PMSensors != nullptr){
        if(true == PMSensors->GetConnectedSensorList(PMSensorList, ( sizeof( PMSensorList ) / sizeof( PMSensorList[0] ) ), &PMSensorListElementCnt ,true) ){
            //Calculate the remaining capacity
            Serial.printf("Collected %i UART Sensors\n\r",PMSensorListElementCnt);
           
        } else {
           //OutOfMemory
           Serial.println("UART SENSORLIST OUT OF MEMORY");
        }
    }


    //Next is Element Assembly 
    for(uint8_t i=0;i<IntSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::INTERNAL;
        List[used_capacity].ValueType=IntSensorList[i].Type;
        List[used_capacity].ChannelIDX=IntSensorList[i].ChannelID;
        used_capacity++;
    }

    for(uint8_t i=0;i<I2CSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::I2C;
        List[used_capacity].ValueType=I2CSensorList[i].Type;
        List[used_capacity].ChannelIDX=I2CSensorList[i].ChannelID;
        used_capacity++;
    }

      for(uint8_t i=0;i<PMSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::UART_SERIAL;
        List[used_capacity].ValueType=PMSensorList[i].Type;
        List[used_capacity].ChannelIDX=PMSensorList[i].ChannelID;
        used_capacity++;
    }

    return used_capacity;
}


uint8_t VALUEMAPPING::GetSensors( SensorElementEntry_t* List, uint8_t capacity){

 
    InternalSensors::SensorUnitInfo_t IntSensorList[15];
    I2C_Sensors::SensorUnitInfo_t I2CSensorList[30];
    UART_PM_Sensors::SensorUnitInfo_t PMSensorList[6];
    uint8_t IntSensorListElementCnt=0;
    uint8_t I2CSensorListElementCnt=0;
    uint8_t PMSensorListElementCnt=0;
    
    uint8_t used_capacity = 0;


    //We collect all sensors we can get
    if(IntSensors != nullptr ){
        
        if(true == IntSensors->GetSensorList(IntSensorList, ( sizeof( IntSensorList ) / sizeof( IntSensorList[0] ) )  ,&IntSensorListElementCnt)){
            Serial.printf("Collected %i INTERNAL Sensors\n\r",IntSensorListElementCnt);
        } else {
            //OutOfMemory
            Serial.println("INTERNAL SENSORLIST OUT OF MEMORY");
        }
    }
    
    if(I2CSensorBus != nullptr){
        //Collect all I2C Sensors
        if(true == I2CSensorBus->GetSensorList(I2CSensorList, ( sizeof( I2CSensorList ) / sizeof( I2CSensorList[0] ) ), &I2CSensorListElementCnt ) ){
            //Calculate the remaining capacity
           Serial.printf("Collected %i I2C Sensors\n\r",I2CSensorListElementCnt);
        } else {
           //OutOfMemory
           Serial.println("I2C SENSORLIST OUT OF MEMORY");
        }
    }

    if(PMSensors != nullptr){
        if(true == PMSensors->GetSensorList(PMSensorList, ( sizeof( PMSensorList ) / sizeof( PMSensorList[0] ) ), &PMSensorListElementCnt ) ){
            //Calculate the remaining capacity
            Serial.printf("Collected %i UART Sensors\n\r",PMSensorListElementCnt);
           
        } else {
           //OutOfMemory
           Serial.println("UART SENSORLIST OUT OF MEMORY");
        }
    }


    //Next is Element Assembly 
    for(uint8_t i=0;i<IntSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::INTERNAL;
        List[used_capacity].ValueType=IntSensorList[i].Type;
        List[used_capacity].ChannelIDX=IntSensorList[i].ChannelID;
        used_capacity++;
    }

    for(uint8_t i=0;i<I2CSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::I2C;
        List[used_capacity].ValueType=I2CSensorList[i].Type;
        List[used_capacity].ChannelIDX=I2CSensorList[i].ChannelID;
        used_capacity++;
    }

      for(uint8_t i=0;i<PMSensorListElementCnt;i++){
        if(used_capacity>=capacity){
            return used_capacity;
        }
        List[used_capacity].Bus=VALUEMAPPING::UART_SERIAL;
        List[used_capacity].ValueType=PMSensorList[i].Type;
        List[used_capacity].ChannelIDX=PMSensorList[i].ChannelID;
        used_capacity++;
    }

    return used_capacity;
}


bool VALUEMAPPING::ReadMappedValue( float* Value, uint8_t Channel ){
    float value = NAN;
    if(Channel>= ( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
        return false;
    } 
    //If we are here the channel is mapped and we need to fetch data
    switch(MappingTable[Channel].Bus){
        case NOTMAPPED:{
            return false;
        } break;

        case INTERNAL:{
            if(nullptr!=IntSensors){
                value = IntSensors->GetValue(MappingTable[Channel].ValueType, MappingTable[Channel].ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("Internal Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        case I2C:{
            if(nullptr!=I2CSensorBus){
                value = I2CSensorBus->GetValue(MappingTable[Channel].ValueType, MappingTable[Channel].ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("I2C Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        case UART_SERIAL:{
            if(nullptr!=PMSensors){
                value = PMSensors->GetValue(MappingTable[Channel].ValueType, MappingTable[Channel].ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("UART Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        default:{
            return false;
        } break;
    }

    *Value = value;
    return true;

}

void VALUEMAPPING::PrintElementData( SensorElementEntry_t Element ){
    Serial.printf("Element: Bus=%i, ValueType=%i, Channel=%i\n\r",Element.Bus,Element.ValueType,Element.ChannelIDX);
}

void VALUEMAPPING::SetMappingForChannel( uint8_t MappedChannelIndex, SensorElementEntry_t Element ){

     if(MappedChannelIndex<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
        //PrintElementData(Element);
        memcpy((void*)(&MappingTable[MappedChannelIndex]), (void*)&Element, sizeof(Element));
        //Serial.printf("Mapped to channel %i", MappedChannelIndex);
        //PrintElementData(MappingTable[MappedChannelIndex]);
        WriteConfig();
    } else {
        Serial.println("Mapping Channel out of Range");
    }
}

VALUEMAPPING::SensorElementEntry_t VALUEMAPPING::GetMappingForChannel( uint8_t MappedChannelIndex){
    VALUEMAPPING::SensorElementEntry_t Element;
    Element.Bus=NOTMAPPED;
    if(MappedChannelIndex<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
              memcpy((void*)&Element,(void*)(&MappingTable[MappedChannelIndex]),  sizeof(Element));
    }
    return Element;
}

//This will fetch the corresponding Sensorname for better human identificaion
String VALUEMAPPING::GetSensorName( VALUEMAPPING::SensorElementEntry_t Element){
    String Sensorname;
    String BusName;
    String ValueTyeName;
    String ChannelID;
    String ValueName;

    BusName = BusNames[Element.Bus];
    ValueTyeName = DATAUNITS::GetValueTypeName( Element.ValueType);
    ChannelID = String(Element.ChannelIDX);
    switch(Element.Bus){
        case INTERNAL:{
            if(IntSensors != nullptr){
                ValueName = IntSensors->GetChannelName(Element.ValueType,Element.ChannelIDX);
            } else {
                ValueName = "?";
            }
        } break;

        case I2C:{
            if(I2CSensorBus != nullptr){
                ValueName = I2CSensorBus->GetChannelName(Element.ValueType,Element.ChannelIDX);
            } else {
                ValueName = "?";
            }
        } break;

        case UART_SERIAL:{
            if(PMSensors != nullptr){
                ValueName = PMSensors->GetChannelName(Element.ValueType,Element.ChannelIDX);
            } else {
                ValueName = "?";
            }
        } break;
    }
    
    //Sensorname = BusName+"."+ValueTyeName+"."+ChannelID+"."+ValueName;
    Sensorname = ValueName + "." +ChannelID + "." + ValueTyeName+ "." + BusName;
    return Sensorname;


}

String VALUEMAPPING::GetSensorNameByChannel(uint8_t Channel){
     String Name;
     VALUEMAPPING::SensorElementEntry_t Element;
    
     if(Channel<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
              memcpy((void*)&Element,(void*)(&MappingTable[Channel]),  sizeof(Element));
              Name = GetSensorName( Element );
     
     } else {
         Name = BusNames[0];
      
     }

   return Name;
}

 bool VALUEMAPPING::GetSensorValue( float* Value, VALUEMAPPING::SensorElementEntry_t Element ){
    float value = NAN;
    switch(Element.Bus){
        case NOTMAPPED:{
            return false;
        } break;

        case INTERNAL:{
            if(nullptr!=IntSensors){
                value = IntSensors->GetValue(Element.ValueType, Element.ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("Internal Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        case I2C:{
            if(nullptr!=I2CSensorBus){
                value = I2CSensorBus->GetValue(Element.ValueType, Element.ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("I2C Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        case UART_SERIAL:{
            if(nullptr!=PMSensors){
                value = PMSensors->GetValue(Element.ValueType, Element.ChannelIDX);
            } else {
                //This is an error !
                Serial.printf("UART Sensor, no driver registred\n\r");
                return false;
            }
        } break;

        default:{
            return false;
        } break;
    }

    *Value = value;
    return true;


}

void VALUEMAPPING::ReadConfig( void ){

    //Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/mapping.json")){
        File file = SPIFFS.open("/mapping.json");
        //We need to read the file into the ArduinoJson Parser
         /*
        ReadBufferingStream bufferingStream(file, 64);
        deserialzeJson(doc, bufferingStream);
        */
        //Arraysize
        uint32_t elementcount = ( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) );
        const size_t capacity = elementcount*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + elementcount*JSON_OBJECT_SIZE(3) + 1580;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, file);
        JsonArray Mapping = doc["Mapping"];
        for(uint8_t i=0;i<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) );i++){
            JsonObject Entry = Mapping[i];
            int Bus = Entry["Bus"]; // 0
            int Type = Entry["ValueType"]; // 0
            int Channel = Entry["Channel"]; // 0

            if(Bus<0){
                Bus=0;
            }
            if(Type<0){
                Type=0;
            }
            if(Channel<0){
                Channel=0;
            }
            
            MappingTable[i].Bus=(VALUEMAPPING::SensorBus_t)(Bus);
            MappingTable[i].ValueType=(DATAUNITS::MessurmentValueType_t)(Type);
            MappingTable[i].ChannelIDX= (uint8_t)(Channel);
        }
        file.close();
    } else {
        //We need to create a blank mapping scheme
         for(uint8_t i=0;i< ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) ; i++   ){
            MappingTable[i].Bus=NOTMAPPED;
         }
         Serial.println("Write new Mapping");
         WriteConfig();

    }


}

void VALUEMAPPING::WriteConfig( void ){

    File file = SPIFFS.open("/mapping.json", FILE_WRITE);
     uint32_t elementcount = ( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) );
    const size_t capacity = elementcount*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + elementcount*JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    JsonArray Mapping = doc.createNestedArray("Mapping");
    for(uint32_t i=0;i<( sizeof(MappingTable) / sizeof( MappingTable[0] )  );i++){
            JsonObject MappingObj = Mapping.createNestedObject();
            MappingObj["Bus"] = (uint8_t)(MappingTable[i].Bus);
            MappingObj["ValueType"] = (uint8_t)(MappingTable[i].ValueType);
            MappingObj["Channel"] = (uint8_t)(MappingTable[i].ChannelIDX);;
    }
    serializeJson(doc, file);
    file.close();

}

uint8_t VALUEMAPPING::GetMaxMappedChannels( void ){
    return ( sizeof(MappingTable) / sizeof( MappingTable[0] )  );
}
