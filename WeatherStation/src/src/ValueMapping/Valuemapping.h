#ifndef _VALUEMAPPING_H_
 #define _VALUEMAPPING_H_
 #include <Arduino.h>
 #include "../DataUnits/DataUnits.h"
 #include "../I2C_Sensors/i2c_sensors.h"
 #include "../InternalSensors/InternalSensors.h"
 #include "../uart_pm_sensors/uart_pm_sensors.h"

class VALUEMAPPING{

    public:

       
        
        typedef enum {
            NOTMAPPED=0,
            INTERNAL,
            I2C,
            UART_SERIAL,
            /*ONEWIRE,*/
            SENSORBUS_CNT
        } SensorBus_t;

        typedef struct{
            SensorBus_t Bus;
            DATAUNITS::MessurmentValueType_t ValueType;
            uint8_t ChannelIDX;

        } SensorElementEntry_t;

         VALUEMAPPING( void );
        ~VALUEMAPPING( void );

        void begin( void );
        void RegisterI2CBus(I2C_Sensors* Sensors );
        void RegisterInternalSensors(InternalSensors* Sensors);
        void RegisterUartSensors(UART_PM_Sensors* PMSensors);
        
        //This will grab one mapped value, returns false if not mapped
        bool ReadMappedValue( float* Value, uint8_t MappedChannelIndex );
       
        

        uint8_t GetSensors( SensorElementEntry_t* List, uint8_t capacity);
        uint8_t GetConnectedSensors( SensorElementEntry_t* List, uint8_t capacity);
        void SetMappingForChannel( uint8_t MappedChannelIndex, SensorElementEntry_t Element );
        SensorElementEntry_t GetMappingForChannel( uint8_t MappedChannelIndex);
        
        //This will fetch the corresponding Sensorname for better human identificaion
        bool GetSensorValue( float* Value, VALUEMAPPING::SensorElementEntry_t Element );
        String GetSensorName( SensorElementEntry_t Element);
        String GetSensorNameByChannel(uint8_t Channel);
        uint8_t GetMaxMappedChannels( void );

    private:

    SensorElementEntry_t MappingTable[64]; //If we can reduce the amount of channels here 
    I2C_Sensors* I2CSensorBus=nullptr;
    InternalSensors* IntSensors=nullptr;
    UART_PM_Sensors* PMSensors=nullptr;

    const String BusNames[SENSORBUS_CNT]={
        "NOTMAPPED",
        "INTERNAL",
        "I2C",
        "UART"
    };

    void ReadConfig( void );
    void WriteConfig( void );

    void RegisterOneWire( void);

    void PrintElementData( SensorElementEntry_t Element );


};




#endif