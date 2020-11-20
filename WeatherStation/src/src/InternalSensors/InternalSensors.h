#ifndef _INTERNALSENSORS_H_
 #define _INTERNALSENSORS_H_
 #include <Arduino.h>
 #include "../DataUnits/DataUnits.h"


class InternalSensors{
 
    public:

        InternalSensors( void );
        ~InternalSensors( void );

        typedef enum {
            GPIO_SPEED=0,
            GPIO_DIRECTION,
            GPIO_RAINAMOUNT,
            SENSOR_CNT
        } SensorType_t;
        
        
        typedef struct{
            SensorType_t Sensor;
            DATAUNITS::MessurmentValueType_t Type;
            uint8_t ChannelID;
        } SensorUnitInfo_t;

        void begin( int pin_winddir , int pin_windspeed, int pin_rainmeter );
        float GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel );
        bool GetSensorList( SensorUnitInfo_t* , uint8_t capacity, uint8_t* used_elements);
        bool GetConnectedSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements, bool mustbepresent);
        
        String GetChannelName(SensorType_t Sensor, uint8_t channel);
        String GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel );

    private:
       
        const String  SensorNames[12]={

            String ("Windspeed"),
            String ("WINDSPEED(10S)"),
            String ("WINDSPEED(60S)"),
            String ("WINDSPEEED(3600S)"),
            String ("WINDDIRECTION(10S)"),
            String ("WINDDIRECTION(60S)"),
            String ("WINDDIRECTION(3600S)"),
            String ("RAINAMOUNT(30M)"),
            String ("RAINAMOUNT(60M)"),
            String ("RAINAMOUNT(720M)"),
            String ("RAINAMOUNT(1440M)"),
            String ("RAINAMOUNT ACCUMULATED")
        };

        const SensorUnitInfo_t SensorInfo[12]={
            {.Sensor=GPIO_SPEED, .Type=DATAUNITS::MessurmentValueType_t::SPEED,.ChannelID=3 },
            {.Sensor=GPIO_SPEED, .Type=DATAUNITS::MessurmentValueType_t::SPEED,.ChannelID=0 },
            {.Sensor=GPIO_SPEED, .Type=DATAUNITS::MessurmentValueType_t::SPEED,.ChannelID=1 },
            {.Sensor=GPIO_SPEED, .Type=DATAUNITS::MessurmentValueType_t::SPEED,.ChannelID=2 },
            {.Sensor=GPIO_DIRECTION, .Type=DATAUNITS::MessurmentValueType_t::DIRECTION,.ChannelID=0 },
            {.Sensor=GPIO_DIRECTION, .Type=DATAUNITS::MessurmentValueType_t::DIRECTION,.ChannelID=1 },
            {.Sensor=GPIO_DIRECTION, .Type=DATAUNITS::MessurmentValueType_t::DIRECTION,.ChannelID=2 },
            {.Sensor=GPIO_RAINAMOUNT, .Type=DATAUNITS::MessurmentValueType_t::RAINAMOUNT,.ChannelID=0 },
            {.Sensor=GPIO_RAINAMOUNT, .Type=DATAUNITS::MessurmentValueType_t::RAINAMOUNT,.ChannelID=1 },
            {.Sensor=GPIO_RAINAMOUNT, .Type=DATAUNITS::MessurmentValueType_t::RAINAMOUNT,.ChannelID=2 },
            {.Sensor=GPIO_RAINAMOUNT, .Type=DATAUNITS::MessurmentValueType_t::RAINAMOUNT,.ChannelID=3 },
            {.Sensor=GPIO_RAINAMOUNT, .Type=DATAUNITS::MessurmentValueType_t::RAINAMOUNT,.ChannelID=4 }
        };

};

#endif