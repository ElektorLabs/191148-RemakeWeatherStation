#include "ValueMapping.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "../I2C_Sensors/i2c_sensors.h"

 /**************************************************************************************************
 *    Function      : VALUEMAPPING
 *    Description   : Constructor
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
VALUEMAPPING::VALUEMAPPING( void ){
    
    for(uint8_t i=0;i< ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) ; i++   ){
         MappingTable[i].Bus=NOTMAPPED;
    }
}

 /**************************************************************************************************
 *    Function      : VALUEMAPPING
 *    Description   : Destructor
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
VALUEMAPPING::~VALUEMAPPING( void ){

}

/**************************************************************************************************
 *    Function      : RegisterI2CBus
 *    Description   : Will add a I2C Bus to the Mapper
 *    Input         : I2C_Sensors* Sensors
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::RegisterI2CBus(I2C_Sensors* Sensors ){
    I2CSensorBus = Sensors;
}

/**************************************************************************************************
 *    Function      : RegisterUartSensors
 *    Description   : Will add a UART Sensors to the Mapper
 *    Input         : UART_PM_Sensors* Sensors
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::RegisterUartSensors(UART_PM_Sensors* Sensors){
    PMSensors=Sensors;
}

/**************************************************************************************************
 *    Function      : RegisterOneWire
 *    Description   : Will add a OneWire-Bus to the Mapper
 *    Input         : void
 *    Output        : void
 *    Remarks       : Not supported yet
 **************************************************************************************************/
void VALUEMAPPING::RegisterOneWire( void){
    //Not supported yet
}


/**************************************************************************************************
 *    Function      : RegisterInternalSensors
 *    Description   : Will add internal sensors to the mapper
 *    Input         : InternalSensors* Sensors
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::RegisterInternalSensors(InternalSensors* Sensors){
    IntSensors = Sensors;
}

/**************************************************************************************************
 *    Function      : begin
 *    Description   : Will read and apply the sored config
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::begin( void ){

    //We collect all know bus devices here
    //All connected busses must be register in bevore

    ReadConfig();

}

/**************************************************************************************************
 *    Function      : GetConnectedSensors
 *    Description   :  Will return the connected sensors at the system
 *    Input         : SensorElementEntry_t* List, uint8_t capacity
 *    Output        : uint8_t
 *    Remarks       : None
 **************************************************************************************************/
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
            //Serial.printf("Collected %i INTERNAL Sensors\n\r",IntSensorListElementCnt);
        } else {
            //OutOfMemory
            //Serial.println("INTERNAL SENSORLIST OUT OF MEMORY");
        }
    }
    
    if(I2CSensorBus != nullptr){
        //Collect all I2C Sensors
        if(true == I2CSensorBus->GetConnectedSensorList(I2CSensorList, ( sizeof( I2CSensorList ) / sizeof( I2CSensorList[0] ) ), &I2CSensorListElementCnt, true ) ){
            //Calculate the remaining capacity
           //Serial.printf("Collected %i I2C Sensors\n\r",I2CSensorListElementCnt);
        } else {
           //OutOfMemory
           //Serial.println("I2C SENSORLIST OUT OF MEMORY");
        }
    }

    if(PMSensors != nullptr){
        if(true == PMSensors->GetConnectedSensorList(PMSensorList, ( sizeof( PMSensorList ) / sizeof( PMSensorList[0] ) ), &PMSensorListElementCnt ,true) ){
            //Calculate the remaining capacity
            //Serial.printf("Collected %i UART Sensors\n\r",PMSensorListElementCnt);
           
        } else {
           //OutOfMemory
           //Serial.println("UART SENSORLIST OUT OF MEMORY");
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

/**************************************************************************************************
 *    Function      : GetSensors
 *    Description   :  Will return the supported sensors at the system
 *    Input         : SensorElementEntry_t* List, uint8_t capacity
 *    Output        : uint8_t
 *    Remarks       : None
 **************************************************************************************************/
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
            //Serial.printf("Collected %i INTERNAL Sensors\n\r",IntSensorListElementCnt);
        } else {
            //OutOfMemory
            //Serial.println("INTERNAL SENSORLIST OUT OF MEMORY");
        }
    }
    
    if(I2CSensorBus != nullptr){
        //Collect all I2C Sensors
        if(true == I2CSensorBus->GetSensorList(I2CSensorList, ( sizeof( I2CSensorList ) / sizeof( I2CSensorList[0] ) ), &I2CSensorListElementCnt ) ){
            //Calculate the remaining capacity
           //Serial.printf("Collected %i I2C Sensors\n\r",I2CSensorListElementCnt);
        } else {
           //OutOfMemory
           //Serial.println("I2C SENSORLIST OUT OF MEMORY");
        }
    }

    if(PMSensors != nullptr){
        if(true == PMSensors->GetSensorList(PMSensorList, ( sizeof( PMSensorList ) / sizeof( PMSensorList[0] ) ), &PMSensorListElementCnt ) ){
            //Calculate the remaining capacity
            //Serial.printf("Collected %i UART Sensors\n\r",PMSensorListElementCnt);
           
        } else {
           //OutOfMemory
           //Serial.println("UART SENSORLIST OUT OF MEMORY");
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

/**************************************************************************************************
 *    Function      : ReadMappedValue
 *    Description   : Will return a mapped value
 *    Input         : float* Value, uint8_t Channel
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
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
                #ifdef DEBUG_SERIAL
                Serial.printf("Internal Sensor, no driver registred\n\r");
                #endif
                return false;
            }
        } break;

        case I2C:{
            if(nullptr!=I2CSensorBus){
                value = I2CSensorBus->GetValue(MappingTable[Channel].ValueType, MappingTable[Channel].ChannelIDX);
            } else {
                //This is an error 
                #ifdef DEBUG_SERIAL
                Serial.printf("I2C Sensor, no driver registred\n\r");
                #endif
                return false;
            }
        } break;

        case UART_SERIAL:{
            if(nullptr!=PMSensors){
                value = PMSensors->GetValue(MappingTable[Channel].ValueType, MappingTable[Channel].ChannelIDX);
            } else {
                //This is an error !
                #ifdef DEBUG_SERIAL
                Serial.printf("UART Sensor, no driver registred\n\r");
                #endif
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

/**************************************************************************************************
 *    Function      : PrintElementData
 *    Description   : Debug function to display element content 
 *    Input         : SensorElementEntry_t Element 
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::PrintElementData( SensorElementEntry_t Element ){
    #ifdef DEBUG_SERIAL
        Serial.printf("Element: Bus=%i, ValueType=%i, Channel=%i\n\r",Element.Bus,Element.ValueType,Element.ChannelIDX);
    #endif
}

/**************************************************************************************************
 *    Function      : SetMappingForChannel
 *    Description   : Will set a mapping for an internal channel
 *    Input         : uint8_t MappedChannelIndex, SensorElementEntry_t Element
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void VALUEMAPPING::SetMappingForChannel( uint8_t MappedChannelIndex, SensorElementEntry_t Element ){

     if(MappedChannelIndex<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
        //PrintElementData(Element);
        memcpy((void*)(&MappingTable[MappedChannelIndex]), (void*)&Element, sizeof(Element));
        //Serial.printf("Mapped to channel %i", MappedChannelIndex);
        //PrintElementData(MappingTable[MappedChannelIndex]);
        WriteConfig();
    } else {
        #ifdef DEBUG_SERIAL
        Serial.println("Mapping Channel out of Range");
        #endif
    }
}

/**************************************************************************************************
 *    Function      : GetMappingForChannel
 *    Description   : Will return the mapping for a given channel
 *    Input         : uint8_t MappedChannelIndex 
 *    Output        : VALUEMAPPING::SensorElementEntry_t
 *    Remarks       : None
 **************************************************************************************************/
VALUEMAPPING::SensorElementEntry_t VALUEMAPPING::GetMappingForChannel( uint8_t MappedChannelIndex){
    VALUEMAPPING::SensorElementEntry_t Element;
    Element.Bus=NOTMAPPED;
    if(MappedChannelIndex<( ( sizeof(MappingTable) / sizeof( MappingTable[0] )  ) )){
              memcpy((void*)&Element,(void*)(&MappingTable[MappedChannelIndex]),  sizeof(Element));
    }
    return Element;
}

/**************************************************************************************************
 *    Function      : GetSensorName
 *    Description   : Will return the frinedly name for a sensor 
 *    Input         : VALUEMAPPING::SensorElementEntry_t Element
 *    Output        : String
 *    Remarks       : None
 **************************************************************************************************/
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

	default:{
		ValueName = "?";
	}break;
    }
    
    //Sensorname = BusName+"."+ValueTyeName+"."+ChannelID+"."+ValueName;
    Sensorname = ValueName + "." +ChannelID + "." + ValueTyeName+ "." + BusName;
    return Sensorname;


}

/**************************************************************************************************
 *    Function      : GetSensorNameByChannel
 *    Description   : Will return the frinedly name for a sensor 
 *    Input         : uint8_t Channel
 *    Output        : String
 *    Remarks       : None
 **************************************************************************************************/
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

/**************************************************************************************************
 *    Function      : GetSensorValue
 *    Description   : Will return value of a sensor if mapped correctly
 *    Input         : float* Value, VALUEMAPPING::SensorElementEntry_t Element
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
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
                #ifdef DEBUG_SERIAL
                Serial.printf("Internal Sensor, no driver registred\n\r");
                #endif
                return false;
            }
        } break;

        case I2C:{
            if(nullptr!=I2CSensorBus){
                value = I2CSensorBus->GetValue(Element.ValueType, Element.ChannelIDX);
            } else {
                //This is an error !
                #ifdef DEBUG_SERIAL
                Serial.printf("I2C Sensor, no driver registred\n\r");
                #endif
                return false;
            }
        } break;

        case UART_SERIAL:{
            if(nullptr!=PMSensors){
                value = PMSensors->GetValue(Element.ValueType, Element.ChannelIDX);
            } else {
                //This is an error !
                #ifdef DEBUG_SERIAL
                    Serial.printf("UART Sensor, no driver registred\n\r");
                #endif
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

/**************************************************************************************************
 *    Function      : ReadConfig
 *    Description   : Will read and apply config from SPIFFS
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
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
         //Next is to apply a default scheme.....
       
        MappingTable[0].Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
        MappingTable[0].ValueType = DATAUNITS::SPEED;
        MappingTable[0].ChannelIDX = 0;
       
        MappingTable[1].Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
        MappingTable[1].ValueType = DATAUNITS::DIRECTION;
        MappingTable[1].ChannelIDX = 0;
        
        
        MappingTable[2].Bus = VALUEMAPPING::SensorBus_t::INTERNAL;
        MappingTable[2].ValueType = DATAUNITS::RAINAMOUNT;
        MappingTable[2].ChannelIDX = 0;
        
      
        MappingTable[3].Bus = VALUEMAPPING::SensorBus_t::I2C;
        MappingTable[3].ValueType = DATAUNITS::TEMPERATURE;
        MappingTable[3].ChannelIDX = 0;
        
        
        MappingTable[4].Bus = VALUEMAPPING::SensorBus_t::I2C;
        MappingTable[4].ValueType = DATAUNITS::HUMIDITY;
        MappingTable[4].ChannelIDX = 0;

        MappingTable[5].Bus = VALUEMAPPING::SensorBus_t::I2C;
        MappingTable[5].ValueType = DATAUNITS::PRESSURE;
        MappingTable[5].ChannelIDX = 0;
        #ifdef DEBUG_SERIAL
            Serial.println("Write new Mapping");
        #endif
        WriteConfig();

    }


}


/**************************************************************************************************
 *    Function      : WriteConfig
 *    Description   : Will write config to SPIFFS
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
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

/**************************************************************************************************
 *    Function      : GetMaxMappedChannels
 *    Description   : Will get the max channels by the mapper
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
uint8_t VALUEMAPPING::GetMaxMappedChannels( void ){
    return ( sizeof(MappingTable) / sizeof( MappingTable[0] )  );
}
