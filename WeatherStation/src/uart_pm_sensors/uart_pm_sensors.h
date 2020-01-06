#ifndef _UART_PARTICLESENSOR_H_
 #define _UART_PARTICLESENSOR_H_

 #include <Arduino.h>
 #include "../DataUnits/DataUnits.h"
 #include "SDS011.h"
 #include "HPMA115S0.h"

 
    class UART_PM_Sensors{

       public:

         typedef enum {
            UNDEF=0,
            PM2_5,
            PM10,
            SIZEELEMNENT_CNT
         } ParticleSize_t;

         typedef enum {
            NONE=0,
            SENSOR_HPM115S0,
            SENSOR_SDS011,
            SERIALSENSORDRIVER_CNT
         } SerialSensorDriver_t;


         typedef enum {
            SERIAL_HPMA115S0=0,
            SERIAL_SDS011,
            SENSOR_CNT
        } SensorType_t;
        
        
        typedef struct{
            SensorType_t Sensor;
            DATAUNITS::MessurmentValueType_t Type;
            uint8_t ChannelID;
        } SensorUnitInfo_t;



         UART_PM_Sensors( int16_t rx, int16_t tx );
         ~UART_PM_Sensors( void );
         bool GetParticleCount( float* value, UART_PM_Sensors::ParticleSize_t ps);
         void begin( HardwareSerial &_hwserial, SerialSensorDriver_t device );
         void suspend( void );
         void wakeup( void );

         float GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel );
         bool GetSensorList( SensorUnitInfo_t* , uint8_t capacity, uint8_t* used_elements);
         bool GetConnectedSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements, bool mustbedetected);
        
         String GetChannelName(SensorType_t Sensor, uint8_t channel);
         String GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel);

         

       private :
            bool SensorDetected = false; 
            UART_PM_Sensors::SerialSensorDriver_t SelectedDriver=  UART_PM_Sensors::SerialSensorDriver_t::NONE;
            SDS011* SDS011device=nullptr;
            HPMA115S0* HPMA115S0device=nullptr;
            HardwareSerial* SerialPort=nullptr;
            int16_t RX;
            int16_t TX;
     
        const String  HMP115SChannelNames[2]={
            String ("PM2.5"),
            String ("PM10"),
        };

         const String  SDS011ChannelNames[2]={
            String ("PM2.5"),
            String ("PM10"),
        };

        const SensorUnitInfo_t HMP115SSensorInfo[2]={
            {.Sensor=SERIAL_HPMA115S0, .Type=DATAUNITS::MessurmentValueType_t::PARTICLES,.ChannelID=0 },
            {.Sensor=SERIAL_HPMA115S0, .Type=DATAUNITS::MessurmentValueType_t::PARTICLES,.ChannelID=1 },
        };

         const SensorUnitInfo_t SDSSensorInfo[2]={
            {.Sensor=SERIAL_SDS011, .Type=DATAUNITS::MessurmentValueType_t::PARTICLES,.ChannelID=0 },
            {.Sensor=SERIAL_SDS011, .Type=DATAUNITS::MessurmentValueType_t::PARTICLES,.ChannelID=1 },
        };

        void SDS011_TEST( void ); //As the sensor seems not good....       
        SemaphoreHandle_t xUARTSemaphore = NULL;
       

    };

 #endif