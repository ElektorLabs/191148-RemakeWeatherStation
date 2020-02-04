
#include "hal_units_def.h"


typedef struct {
   uint8_t addr[8];
   float value;
   Temperatur_Unit_t unit;
} Sensor_Value;

typedef struct{
    uint8_t addr[8];
    bool onbus;
    uint8_t mappedchannel;
} OneWireBusSensor_t;


void DS18x20_init( uint8_t core );

void DS18x20_LoadSensorConfig( void );
void DS18x20_SaveSensorConfig( void );

float Ds18x20_GetMappedChannelValue( uint8_t channel );

bool DS18x20_Search( void );
void DS18x20_ReadActivSensors(Sensor_Value* out_values , uint8_t count  ,  Temperatur_Unit_t unit );
void DS18x20_ReadMapped(float* out_values , uint8_t count  ,  Temperatur_Unit_t unit );
void DS18x20_MapValues( float* out_values, uint8_t out_values_count , Sensor_Value* values, uint8_t values_count );

void DS18x20_GetLastData( Sensor_Value* values, uint8_t values_count );

uint8_t DS18x20_GetDeviceCount( void );
void DS18x20_SetMapping(uint8_t addr[8], int8_t channel);
void DS18x20_DisableMapping( int8_t channel );
void DS18x20_GetDeviceAdd(uint8_t* out_addr, uint8_t out_addr_size, uint8_t idx);
uint8_t DS18x20_GetConfiguredDeviceCount( void );
bool DS18x20_MissingSensors( void );
uint8_t DS18x20_GetAllSensorID(OneWireBusSensor_t* Sensors, uint8_t cpacity );
bool DS180x20_IsChannelMapped( uint8_t channel );

void DS18x20_PrintSensorAddress(uint8_t* addr, bool newline);
