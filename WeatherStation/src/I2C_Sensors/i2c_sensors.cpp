/* Includes for the sensor modules */
/*
- BME280 
- Wuerth WSEN-PAD
- Veml6070
- Tsl2561
- VEML6075
- TSL2591
*/
#include "../i2c_access/i2c_access.h"
#include "i2c_sensors.h"

I2C_Sensors::I2C_Sensors( int scl, int sda ){
   SDA = sda;
   SCL = scl;
   xI2C_Semaphore = xSemaphoreCreateMutex();
   if( xI2C_Semaphore == NULL )
   {
        //This is bad!
        abort();
   }
}

I2C_Sensors::~I2C_Sensors( void ){



}

void I2C_Sensors::begin( void ){
    uint8_t driver_idx=0;
    //100kHz max for the wire, may reduced to 10kHz......
    Wire.begin(SDA,SCL,100000);
    //We fist look for the BME280 at address 0x77 / 0x76 
    i2c_lock_bus();
    if( true == bme280.begin( BME280_ADDRESS ) ){
        Serial.println("BME found @ 0x77");
        DeviceOnBus[BME280]=true;
    } else if( true == bme280.begin( BME280_ADDRESS_ALTERNATE  ) ){
        Serial.println("BME found @ 0x76");
        DeviceOnBus[BME280]=true;
    } else {
        //No BME280 found....
        Serial.println("No BME280 found");
        DeviceOnBus[BME280]=false;
    }
    //Next is to check for the WSEN-PAD
    wsen_pads.setAddress(1); // SAO standard 1 for power safe
    if(true == wsen_pads.begin(WSEN_PADS_ONESHOT ) ){
       Serial.println("WSEN-PAD found @ 0x5D"); 
        DeviceOnBus[WSEN_PADS] = true;

    } else {
        wsen_pads.setAddress(0);
        if( true == wsen_pads.begin(WSEN_PADS_ONESHOT ) ) {
            Serial.println("WSEN-PAD found @ 0x5C");
            DeviceOnBus[WSEN_PADS] = true;
        } else {
            Serial.println("No WSEN-PAD found");
            DeviceOnBus[WSEN_PADS] = false;
        }
    }

    Wire.beginTransmission( 0x38 );
    if(0 ==  Wire.endTransmission() ) {
        uv_6070.begin(VEML6070_4_T);  // pass in the integration time constant
        Serial.println("VEML6070 found @ 0x38");
        DeviceOnBus[VEML6070] = true;
    } else {
        Serial.println("No VEML6070 found");
        DeviceOnBus[VEML6070]=false;
    }

    if(true ==  uv_6075.begin() ){
        Serial.println("VEML6075 found @ 0x10");
        DeviceOnBus[VEML6075] = true;
    } else {
        Serial.println("No VEML6075 found");
        DeviceOnBus[VEML6070] = false;
    }



    if( true == tsl2561.begin() ){
        Serial.println("TSL2561 found @ 0x39");
        DeviceOnBus[TSL2561] = true;
    } else {
       Serial.println("No TSL2561 found");
       DeviceOnBus[TSL2561] = false;
    }

    if( true == tsl2591.begin( )){
         Serial.println("TSL2591. found @ 0x29");
         DeviceOnBus[TSL2591] = true;
    } else {
         Serial.println("No TSL2591. found");
         DeviceOnBus[TSL2591] = false;
    }


    i2c_unlock_bus();
}


void I2C_Sensors::GetBrigtnessChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity ){
    //We collect all Brightnesssensors and return a list
    *List = (SensorUnitInfo_t*)(&LIGHTSENSORS[0]);
    *ListCapacity = ( sizeof( LIGHTSENSORS ) / sizeof( LIGHTSENSORS[0] ) );

}

void I2C_Sensors::GetUVChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity ){
    //We collect all UV Sensors and return a list
    *List = (SensorUnitInfo_t*)(&VU_LIGHTSENSORS[0]);
    *ListCapacity = ( sizeof( VU_LIGHTSENSORS ) / sizeof( VU_LIGHTSENSORS[0] ) );

}

void I2C_Sensors::GetTemperatureChannels ( SensorUnitInfo_t** List, uint8_t* ListCapacity ){
    //We collect all Temperatur Sensors and return a list
    *List = (SensorUnitInfo_t*)(&TEMPERATURSENSORS[0]);
    *ListCapacity = ( sizeof( TEMPERATURSENSORS ) / sizeof( TEMPERATURSENSORS[0] ) );

}

void I2C_Sensors::GetPressureChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity ){
    //We collect all Pressure Sensors and return a list
    *List = (SensorUnitInfo_t*)(&PRESSURESENSORS[0]);
    *ListCapacity = ( sizeof( PRESSURESENSORS ) / sizeof( PRESSURESENSORS[0] ) );
}

void I2C_Sensors::GetHumidutyChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity ){
    //We collect all Himidity Sensors and return a list
    *List = (SensorUnitInfo_t*)(&HUMIDITYSENSORS[0]);
    *ListCapacity = ( sizeof( HUMIDITYSENSORS ) / sizeof( HUMIDITYSENSORS[0] ) );
}


float I2C_Sensors::GetUVALevel( uint8_t channelidx ){
    float value = NAN;
    switch( channelidx ){
        
        case 0:{
            if(true == DeviceOnBus[VEML6070]){
                value = uv_6070.readUV();

            } else {
                value = NAN;
            }

        } break;

        case 1:{
            if(true == DeviceOnBus[VEML6075]){
                value = uv_6075.readUVA();

            } else {
                value = NAN;
            }

        } break;

        default:{

        } break;
    } 
    return value;
}

float I2C_Sensors::GetUVBLevel( uint8_t channelidx ){
    float value = NAN;
    switch( channelidx ){
        
        case 1:{
            if(true == DeviceOnBus[VEML6075]){
                value = uv_6075.readUVB();
            } else {
                value = NAN;
            }
        } break;

        default:{

        } break;
    } 
    return value;

}
        
float I2C_Sensors::GetLightLevel( uint8_t channelidx  ){
   float value = NAN;
    switch( channelidx ){

        case 0:{
            if(true == DeviceOnBus[TSL2561]){
                    sensors_event_t event;
                    tsl2561.getEvent(&event);
                    if ((event.light == 0) |
                    (event.light > 4294966000.0) | 
                    (event.light <-4294966000.0))
                    {
                        value = NAN;
                    } else {
                        value = event.light;
                    }                  
            } else {
                value = NAN;
            }
        } break;

        case 1:{
            if(true == DeviceOnBus[TSL2591]){
                 sensors_event_t event;
                 tsl2591.getEvent(&event);
                  if ((event.light == 0) |
                    (event.light > 4294966000.0) | 
                    (event.light <-4294966000.0))
                    {
                        value = NAN;
                    } else {
                        value = event.light;
                    }        
                
            } else {
                value = NAN;
            }
        } break;


        default:{

        } break;
    } 

    return  value;

}
float I2C_Sensors::GetTemperature( uint8_t channelidx ){
float value = NAN;
    switch( channelidx ){

        case 0:{
            if(true==DeviceOnBus[BME280]){
                bme280.readTemperature(); //°C as value 
            } else {
                value = NAN;
            }
        } break;

        case 1:{
            if(true == DeviceOnBus[WSEN_PADS] ){
                float pressure;
                wsen_pads.getValues(&pressure, &value); // pressure in kPa, temp in Celsius
            } else {
                value = NAN;
            }

        } break;


        default:{
            value = NAN;
        } break;
    } 
    return value;

}
float I2C_Sensors::GetPressure( uint8_t channelidx ){
    float pressure=NAN;
    switch( channelidx ){

        case 0:{
            if(true == DeviceOnBus[BME280]){
               pressure =  bme280.readPressure() / 100; // This will give the perssure in hPa
            } else {

            }
        } break;

        case 1:{
             if(true == DeviceOnBus[WSEN_PADS] ){
                float temperature;
                wsen_pads.getValues(&pressure, &temperature); // pressure in kPa, temp in Celsius
                pressure = pressure *10 ;
            } else {
                pressure = NAN;
            }
        } break;


        default:{

        } break;
    } 
    return pressure;

}

float I2C_Sensors::GetHumidity( uint8_t channelidx ){
    float temp=NAN;
    switch( channelidx ){

        case 0:{
            if(true == DeviceOnBus[BME280]){
               temp =  bme280.readHumidity(); // This will give the humidity in %
            } else {
                float temp=NAN;
            }
        } break;

        default:{

        } break;
    } 
    return temp;

}


bool I2C_Sensors::GetIsSensorOnBus( SensorType_t Sensor ){

    return DeviceOnBus[Sensor]; 
}

uint8_t I2C_Sensors::GetChannelIndex(SensorType_t Sensor){
   /* we do a static channel mapping here 
        Humidity:
            CH0 = BME280
        
        Temperatur
            CH0 = BME280
            CH1 = WSEN_PADS

        Pressure
            CH0 = BME280
            CH1 = WSEN_PADS

        UV_Light ( A+B )
            CH0 = VMEL6070
            CH1 = VMEL6075
        
        UV_Light ( A )
            CH1 = VMEL6075

        UV_Light ( B ) 
            CH1 = VMEL6075  

        Brightness
            CH0 = TSL2561
            CH1 = TSL2591  
        */
       uint8_t idx=0;
       switch(Sensor){
           case BME280:{
               idx=0;
           } break;
           case VEML6070:{
               idx=0;
           } break;
           case VEML6075:{
               idx=1;
           } break;
           case TSL2561:{
               idx=0;
           } break;
           case TSL2591:{
               idx=1;
           } break;
           case WSEN_PADS:{
               idx=1;
           } break;
           default:{
               idx=0;
           }

       }
    return idx;
}

String I2C_Sensors::GetChannelName(SensorType_t Sensor , uint8_t channel){
    return SensorNames[Sensor];
}


/*

 //We have some arrays telling us what type of value we can get from wich sensor
        const SensorUnitInfo_t TEMPERATURSENSORS[2]={
            {.Sensor=BME280,.Type=TEMPERATURE,.ChannelID=0},
            {.Sensor=WSEN_PADS,.Type=TEMPERATURE,.ChannelID=1}
        };

        const SensorUnitInfo_t PRESSURESENSORS[1]={
            {.Sensor=BME280,.Type=PRESSURE,.ChannelID=0}    
        };

        const SensorUnitInfo_t HUMIDITYSENSORS[1]={
             {.Sensor=BME280,.Type=HUMIDITY,.ChannelID=0}
        };

        const SensorUnitInfo_t VU_LIGHTSENSORS[2]={
             {.Sensor=VEML6070,.Type=UV_LIGHT,.ChannelID=0},
             {.Sensor=VEML6075,.Type=UV_LIGHT,.ChannelID=1}
        };

        const SensorUnitInfo_t LIGHTSENSORS[2]={
             {.Sensor=TSL2561,.Type=LIGHT,.ChannelID=0},
             {.Sensor=TSL2591,.Type=LIGHT,.ChannelID=1},
        };


*/

float I2C_Sensors::GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel  ){
    float value = NAN;
    //We need at this point to aquiere a mutex as only on task at a time can access the i2c bus */
    if( xSemaphoreTake( xI2C_Semaphore, portMAX_DELAY ) == pdTRUE )
    {
        i2c_lock_bus();
        switch( Type ){

            case DATAUNITS::TEMPERATURE:{

                value = GetTemperature( channel );

            } break;

            case  DATAUNITS::HUMIDITY:{

                value = GetHumidity( channel );

            } break;

            case  DATAUNITS::PRESSURE:{

                value = GetPressure( channel );

            } break;

            case  DATAUNITS::UV_LIGHT:{

                value = GetUVALevel( channel );

            } break;

            case  DATAUNITS::LIGHT:{
                
                value = GetLightLevel( channel );

            } break;

            default:{
                value = NAN;
            } break;

        }
        i2c_unlock_bus();
        xSemaphoreGive( xI2C_Semaphore );
    }
    return value;

}



bool I2C_Sensors::GetSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements ){
    return GetConnectedSensorList( List, capacity, used_elements, false );
}

bool I2C_Sensors::GetConnectedSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements, bool mustbepresent ){
    //We collect everything we have on bus .....
    uint8_t usedcapacity=0;
    for(uint8_t i=0;i< ( sizeof( TEMPERATURSENSORS ) / sizeof( TEMPERATURSENSORS[0] ) ) ; i++  ){
        if(usedcapacity >= capacity ){
            return false; //Ugly 
        }

        if(( false == mustbepresent) || (true == GetIsSensorOnBus(TEMPERATURSENSORS[i].Sensor) )){
            List[usedcapacity]=TEMPERATURSENSORS[i]; 
            usedcapacity++;
        }
    }

    for(uint8_t i=0;i< ( sizeof( PRESSURESENSORS ) / sizeof( PRESSURESENSORS[0] ) ) ; i++  ){
        if(usedcapacity >= capacity ){
            return false; //Ugly 
        }
        if( (false == mustbepresent) || (true == GetIsSensorOnBus(PRESSURESENSORS[i].Sensor) )){
            List[usedcapacity]=PRESSURESENSORS[i]; 
            usedcapacity++;
        }
    }

    for(uint8_t i=0;i< ( sizeof( HUMIDITYSENSORS ) / sizeof( HUMIDITYSENSORS[0] ) ) ; i++  ){
        if( usedcapacity >= capacity ){
            return false; //Ugly 
        }
       if( (false == mustbepresent) || (true == GetIsSensorOnBus(HUMIDITYSENSORS[i].Sensor) )){
            List[usedcapacity]=HUMIDITYSENSORS[i]; 
            usedcapacity++;
        }
    }

    for(uint8_t i=0;i< ( sizeof( VU_LIGHTSENSORS ) / sizeof( VU_LIGHTSENSORS[0] ) ) ; i++  ){
        if(usedcapacity >= capacity ){
            return false; //Ugly 
        }
        if( (false == mustbepresent) || (true == GetIsSensorOnBus(VU_LIGHTSENSORS[i].Sensor) )){
            List[usedcapacity]=VU_LIGHTSENSORS[i]; 
            usedcapacity++;
        }
    }

    for(uint8_t i=0;i< ( sizeof( LIGHTSENSORS ) / sizeof( LIGHTSENSORS[0] ) ) ; i++  ){
        if(usedcapacity >= capacity ){
            return false; //Ugly 
        }
        if( (false == mustbepresent) || (true == GetIsSensorOnBus(LIGHTSENSORS[i].Sensor) )){
            List[usedcapacity]=LIGHTSENSORS[i]; 
            usedcapacity++;
        }
    }
    /* Ugly and sort of hardcoded */
    *used_elements = usedcapacity;
    return true;


}


 String I2C_Sensors::GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel ){

     bool found = false;
     SensorType_t Sensor;
     String Result="N/A";

     

    for(uint8_t i=0;i< ( sizeof( TEMPERATURSENSORS ) / sizeof( TEMPERATURSENSORS[0] ) ) ; i++  ){
         if( ( TEMPERATURSENSORS[i].Type == Type ) && ( TEMPERATURSENSORS[i].ChannelID==channel )){
             Result = GetChannelName(TEMPERATURSENSORS[i].Sensor, channel);
         }
     
     }

    for(uint8_t i=0;i< ( sizeof( PRESSURESENSORS ) / sizeof( PRESSURESENSORS[0] ) ) ; i++  ){
       if( ( PRESSURESENSORS[i].Type == Type ) && ( PRESSURESENSORS[i].ChannelID==channel )){
             Result = GetChannelName(PRESSURESENSORS[i].Sensor, channel);
         }
     
    }

    for(uint8_t i=0;i< ( sizeof( HUMIDITYSENSORS ) / sizeof( HUMIDITYSENSORS[0] ) ) ; i++  ){
       if( ( HUMIDITYSENSORS[i].Type == Type ) && ( HUMIDITYSENSORS[i].ChannelID==channel )){
             Result = GetChannelName(HUMIDITYSENSORS[i].Sensor, channel);
         }
    }

    for(uint8_t i=0;i< ( sizeof( VU_LIGHTSENSORS ) / sizeof( VU_LIGHTSENSORS[0] ) ) ; i++  ){
        if( ( VU_LIGHTSENSORS[i].Type == Type ) && ( VU_LIGHTSENSORS[i].ChannelID==channel )){
             Result = GetChannelName(VU_LIGHTSENSORS[i].Sensor, channel);
         }
    }

    for(uint8_t i=0;i< ( sizeof( LIGHTSENSORS ) / sizeof( LIGHTSENSORS[0] ) ) ; i++  ){
        if( ( LIGHTSENSORS[i].Type == Type ) && ( LIGHTSENSORS[i].ChannelID==channel )){
             Result = GetChannelName(LIGHTSENSORS[i].Sensor, channel);
         }
    }

     return Result;

 }
