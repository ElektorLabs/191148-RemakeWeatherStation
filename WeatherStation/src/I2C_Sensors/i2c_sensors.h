#ifndef _I2C_SENSORS_H_
 #define _I2C_SENSORS_H_
/* This is the main entry for all i2c sensor modules attached to the station */
/* Currently this modules will support: (7-Bit adress noted )
- BME280 ( 0x77 / 0x76 )
- Wuerth WSEN-PAD ( 0x5C / 0x5D )
- Veml6070 ( 0x38 )
- TSL2561 ( 0x29 / 0x39 / 0x49 )
- VEML6075 (0x10)
- TSL2591 ( 0x29 )

*/


#include <Arduino.h>
#include <string.h>
#include <Wire.h>
#include <eHaJo_WSEN_PADS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6070.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_VEML6075.h>
#include "../DataUnits/DataUnits.h"

class I2C_Sensors{

    public:
     typedef enum {
            BME280=0,
            VEML6070,
            VEML6075,
            TSL2561,
            TSL2591,
            WSEN_PADS,
            SENSOR_TYPE_CNT
        } SensorType_t;


        typedef struct{
            SensorType_t Sensor;
            DATAUNITS::MessurmentValueType_t Type;
            uint8_t ChannelID;
        } SensorUnitInfo_t;

        I2C_Sensors( int scl, int sda );
        ~I2C_Sensors( void );
        
        void begin();
        /* We can have multiple channesl for each value, how to handle this? */
        float GetUVALevel( uint8_t channelidx );
        float GetUVBLevel( uint8_t channelidx );
        float GetLightLevel( uint8_t channelidx);
        float GetTemperature( uint8_t channelidx );
        float GetPressure( uint8_t channelidx );
        float GetHumidity( uint8_t channelidx );

        /* We generate a list with known sensors for a given value */
        void GetBrigtnessChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity );
        void GetUVChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity );
        void GetTemperatureChannels ( SensorUnitInfo_t** List, uint8_t* ListCapacity );
        void GetPressureChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity );
        void GetHumidutyChannels( SensorUnitInfo_t** List, uint8_t* ListCapacity );

        /* We need to provide a mapping to the outside and also sensornames */
        bool GetIsSensorOnBus( SensorType_t Sensor );
        uint8_t GetChannelIndex(SensorType_t Sensor );
        String GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel );
        String GetChannelName(SensorType_t Sensor, uint8_t channel=0);
        float GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel );
        bool GetSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements);
        bool GetConnectedSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements, bool mustbepresent);



    private: 
    
        int16_t SCL = -1;
        int16_t SDA = -1; 

        Adafruit_BME280 bme280 = Adafruit_BME280();
        Adafruit_VEML6070 uv_6070 = Adafruit_VEML6070();
        Adafruit_VEML6075 uv_6075 = Adafruit_VEML6075();
        Adafruit_TSL2561_Unified tsl2561= Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
        Adafruit_TSL2591 tsl2591 = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
       
        EHAJO_WSEN_PADS wsen_pads;

       

        bool DeviceOnBus[SENSOR_TYPE_CNT]={false,};
        const String  SensorNames[SENSOR_TYPE_CNT ]={
            String ("BME280"),
            String ("VEML6070"),
            String ("VEML6075"),
            String ("TSL2561"),
            String ("TSL2591"),
            String ("WSEN PADS")
            
        };
        
        //We have some arrays telling us what type of value we can get from wich sensor
        const SensorUnitInfo_t TEMPERATURSENSORS[2]={
            {.Sensor=BME280,.Type=DATAUNITS::TEMPERATURE,.ChannelID=0},
            {.Sensor=WSEN_PADS,.Type=DATAUNITS::TEMPERATURE,.ChannelID=1}
        };

        const SensorUnitInfo_t PRESSURESENSORS[1]={
            {.Sensor=BME280,.Type=DATAUNITS::PRESSURE,.ChannelID=0}    
        };

        const SensorUnitInfo_t HUMIDITYSENSORS[1]={
             {.Sensor=BME280,.Type=DATAUNITS::HUMIDITY,.ChannelID=0}
        };

        const SensorUnitInfo_t VU_LIGHTSENSORS[2]={
             {.Sensor=VEML6070,.Type=DATAUNITS::UV_LIGHT,.ChannelID=0},
             {.Sensor=VEML6075,.Type=DATAUNITS::UV_LIGHT,.ChannelID=1}
        };

        const SensorUnitInfo_t LIGHTSENSORS[2]={
             {.Sensor=TSL2561,.Type=DATAUNITS::LIGHT,.ChannelID=0},
             {.Sensor=TSL2591,.Type=DATAUNITS::LIGHT,.ChannelID=1},
        };


       


    

};
#endif