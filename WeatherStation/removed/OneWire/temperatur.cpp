#include <OneWire.h>
#include "../../datastore.h"
#include "temperatur.h"

OneWire ds(21);

float DS18x20_GetTemperatureFahrenheit( float celsius );

static Sensor_Value LastData[4];
static float Sensor_Mapped_LastValue[4]={NAN,};

static volatile SemaphoreHandle_t DataxSemaphore=NULL;
static volatile SemaphoreHandle_t OneWireBusMtx=NULL;





// This is the current config......
static onewire_temp_sensor_t sensor[4];

// This is the stored config, including mapping
static onewire_temp_sensor_t configured_sensor[4];
static float temperatur[4] = {NAN,};

void ds18x20_task( void* param );
void DS18x20_AddToHistroy( uint8_t channel, float value );

void DS18x20_init( uint8_t core ){
  if(core>1){
    core=1;
  }
  
  DS18x20_LoadSensorConfig();
  Serial.printf("Configured 1-Wire DS18x20 Devices %u\n\r", DS18x20_GetConfiguredDeviceCount() );
  for(uint8_t i=0;i<4;i++){
  Serial.printf("Chip  @ %u:%u:%u:%u:%u:%u:%u\n\r",  configured_sensor[i].addr[0], 
                                                    configured_sensor[i].addr[1],
                                                    configured_sensor[i].addr[2],
                                                    configured_sensor[i].addr[3],
                                                    configured_sensor[i].addr[4],
                                                    configured_sensor[i].addr[5],
                                                    configured_sensor[i].addr[6] );  
                                            
  }
  /*
  DS18x20_Search();
  uint8_t data[8];
   
  for(uint8_t i=0;i<DS18x20_GetDeviceCount();i++){
    DS18x20_GetDeviceAdd(data,8,i);
    DS18x20_SetMapping( data, i);
  }
  DS18x20_SaveSensorConfig();
  */                      
  DataxSemaphore = xSemaphoreCreateMutex();
  if(  NULL == DataxSemaphore  )
  {
    abort();
  }

  OneWireBusMtx = xSemaphoreCreateMutex();
  if(  NULL == OneWireBusMtx  )
  {
    abort();
  }

  DS18x20_Search();
  xTaskCreatePinnedToCore(
                    ds18x20_task,   /* Function to implement the task */
                    "ds18x20_task",  /* Name of the task */
                    4096,       /* Stack size in words */
                    NULL,       /* Task input parameter */
                    5,          /* Priority of the task */
                    NULL,  /* Task handle. */
                    core);         /* Core where the task should run */
  
 
}

void DS18x20_LoadSensorConfig( void ){

 // We read the config from the NVS 
 temp_sensorconfig_t config = read_temp_sensor_settings( );
 memcpy(configured_sensor, config.sensor, sizeof( configured_sensor ) );
 // Data is loeaded we do a debugprint here 


}
void DS18x20_SaveSensorConfig( void ){

  // We save the current config to NVS
  temp_sensorconfig_t config;
  memcpy(config.sensor, sensor, sizeof( sensor ) );
  write_temp_sensor_settings( config );

}

void DS18x20_SetMapping(uint8_t addr[8], int8_t channel){
  addr[7] = OneWire::crc8(addr, 7) ;
  // This will overwrite without warning 
  if(channel>=4){
    return;
  } else {
    DS18x20_DisableMapping( channel);
    for( uint8_t i = 0 ; i<4 ;i++){
      if(0 == memcmp(addr, sensor[i].addr, sizeof(onewire_temp_sensor_t::addr))){
        Serial.printf("Set Channel %i for Sensor %i\n\r", channel, i);
        sensor[i].channelmapping = channel;
      }

      if(0 == memcmp(addr, configured_sensor[i].addr, sizeof(onewire_temp_sensor_t::addr))){
        Serial.printf("Set Channel %i for Config %i\n\r", channel, i);
        configured_sensor[i].channelmapping = channel;
      }
      
    }
  }

}


void DS18x20_DisableMapping( int8_t channel ) {
  for( uint8_t i = 0 ; i<4 ;i++){
      if(channel == configured_sensor[i].channelmapping ){
          configured_sensor[i].channelmapping = -1;
      }

      if(channel == sensor[i].channelmapping ){
          sensor[i].channelmapping = -1;
      }

      
  }
}

void DS18x20_GetDeviceAdd(uint8_t* out_addr, uint8_t out_addr_size, uint8_t idx){
  if(out_addr_size<8){
    return;
  }

  if(idx>=4){
    return;
  }

  memcpy( out_addr , sensor[idx].addr, sizeof( sensor[idx].addr ));
  

  
}


bool DS18x20_Search( void ){

 bool found = false;
 if( xSemaphoreTake( OneWireBusMtx, portMAX_DELAY  ) == pdTRUE )
 {

    /* we need to collect all devices on bus */ 
    for(uint8_t i=0;  i<4 ; i++){
        for(uint8_t s=0;s<8;s++){
          sensor[i].addr[s]=0;
        }
        sensor[i].channelmapping = -1;
    }

    ds.search( sensor[0].addr);
    ds.reset_search();

   
    /* we do multiple runs */
    for ( uint8_t i=0 ; i < 4;i++){
            
          
            if ( !ds.search(sensor[i].addr)) {
                ds.reset_search();
                Serial.println("Reset Search" );  // or old DS1820           
                found |= false;
                bzero(sensor[i].addr,sizeof(sensor[i].addr));
            } else {
            
                    if (OneWire::crc8(sensor[i].addr, 7) != sensor[i].addr[7]) {
                        found |= false;
                        bzero(sensor[i].addr,sizeof(sensor[i].addr));
                    } else {
                                
                                // the first ROM byte indicates which chip
                                switch (sensor[i].addr[0]) {
                                    case 0x10:
                                    Serial.printf("Chip = DS18S20 @ %u:%u:%u:%u:%u:%u\n\r", sensor[i].addr[1],sensor[i].addr[2],sensor[i].addr[3],sensor[i].addr[4],sensor[i].addr[5],sensor[i].addr[6] );  // or old DS1820
                                    found |= true;
                                    break;
                                    case 0x28:
                                    Serial.printf("Chip = DS18B20 @ %u:%u:%u:%u:%u:%u\n\r",  sensor[i].addr[1],sensor[i].addr[2],sensor[i].addr[3],sensor[i].addr[4],sensor[i].addr[5],sensor[i].addr[6] );
                                    found |= true;
                                    break;
                                    case 0x22:
                                    Serial.printf("Chip = DS1822 @ %u:%u:%u:%u:%u:%u\n\r",  sensor[i].addr[1],sensor[i].addr[2],sensor[i].addr[3],sensor[i].addr[4],sensor[i].addr[5],sensor[i].addr[6] );
                                    found |= true;
                                    default:
                                    Serial.println("Device is not a DS18x20 family device.");
                                    
                                } 
                        // We need to find the channelmapping from the loaded values 
                        switch (sensor[i].addr[0]) {

                        case 0x10:
                        case 0x28:
                        case 0x22:{
                          for(uint8_t s=0; s < 4 ;s++ ){
                            if( 0 == memcmp (sensor[i].addr, configured_sensor[s].addr, sizeof(configured_sensor[s].addr) )){
                                  Serial.printf("Config Serial @ %u:%u:%u:%u:%u:%u\n\r",  sensor[i].addr[1],sensor[i].addr[2],sensor[i].addr[3],sensor[i].addr[4],sensor[i].addr[5],sensor[i].addr[6] );
                                  Serial.printf("Found Serial @ %u:%u:%u:%u:%u:%u\n\r",  configured_sensor[s].addr[1],configured_sensor[s].addr[2],configured_sensor[s].addr[3],configured_sensor[s].addr[4],configured_sensor[s].addr[5],configured_sensor[s].addr[6] );
                                  Serial.printf("Map Sensor to Channel %i \n\r", sensor[i].channelmapping);
                                  sensor[i].channelmapping=configured_sensor[s].channelmapping;
                            }
                          }
                        } break;

                        default:{}break;

                      }

                    }
            

            }
    
        }
      xSemaphoreGive( OneWireBusMtx );
    }
    return found;

}

void DS18x20_ReadActivSensors(Sensor_Value* out_values , uint8_t count  ,  Temperatur_Unit_t unit ){

 // Serial.println("Start Reading Sensors");

  /* We loop through the bus and try to collect as much data as possible */
  /* to do this we return the values form nodes found in the last search */
  float values[4]={NAN,};
  bool buserror= false; 
  uint8_t CRC=0;
  byte data[12];
  float temperatur = NAN;
  for(uint8_t i=0;i<count;i++){
    out_values[i].value=NAN; 
  }
  

  if(count>=4){
    count = 4;
  } 

 if( xSemaphoreTake( OneWireBusMtx, 250  ) == pdTRUE )
 {

    for( uint8_t i=0; i< count ;i++){
      values[i]=NAN;
      memcpy( out_values[i].addr , sensor[i].addr, sizeof( sensor[i].addr ) );
      if( ( sensor[i].addr[0] != 0x10 ) && ( sensor[i].addr[0] != 0x28 ) && ( sensor[i].addr[0] != 0x22 )   ){
          
          break; // We can*t use the slave 
      } 

      if ( 0 == ds.reset() ){
        buserror = true;
        
        break; // Bus is gone !
      }

      ds.select(sensor[i].addr);
      ds.write(0x44, 0);        // start conversion, with active power on at the end
     
    }
    xSemaphoreGive( OneWireBusMtx );
 } else {
   /* We can't access the bus and throw an error */
//   Serial.println("END Reading Sensors; MTX Error");
   return;
 }
  
  vTaskDelay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

if( xSemaphoreTake( OneWireBusMtx, 250  ) == pdTRUE )
{
 //  Serial.println("Grab Data");
  for( uint8_t i=0; i< count;i++){
  
    if( ( ( sensor[i].addr[0] != 0x10 ) && ( sensor[i].addr[0] != 0x28 ) && ( sensor[i].addr[0] != 0x22 )   )  ){
        break; // We can*t use the slave 
    } 

    if ( 0 == ds.reset() ){
      break; // Bus is gone !
    }

    ds.select(sensor[i].addr);    
    ds.write(0xBE);         // Read Scratchpad
    for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
    }
    CRC= OneWire::crc8(data, 8);
      

    // Check CRC
    if( (CRC != data[8]) ) {
      // CRC is bad, we assume a communication problem, we also check if the 
      // data read is all 0xFF meaning the sensor is gone....
      uint32_t value = 0;
      for(uint8_t d=0;d<9;d++){
          value+=data[d];
      }
     
      if(value == ( 255*( 9 ) ) ) {
        Serial.print("Sensor is gone -> Bus all high");
      } else {
        Serial.print("CRC Error");
      }   
      Serial.print(" Data: ");
      for(uint8_t i=0;i<9;i++){
        Serial.printf(" 0x%02x", data[i]);
      }
      Serial.printf(" -> Calculated CRC : %u\n\r", CRC);


    } else {

      // Convert the data to actual temperature
      // because the result is a 16 bit signed integer, it should
      // be stored to an "int16_t" type, which is always 16 bits
      // even when compiled on a 32 bit processor.
      int16_t raw = (data[1] << 8) | data[0];
      if (sensor[i].addr[0]==0x10) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
          // "count remain" gives full 12 bit resolution
          raw = (raw & 0xFFF0) + 12 - data[6];
        }
      } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
      }
      
      temperatur = (float)raw / 16.0;
      if ( unit == T_Kelvin ) {
        values[i] =  temperatur - 271.3;
      } else if ( unit == T_Celsius ){
          values[i] = temperatur;
      } else {
          values[i] = DS18x20_GetTemperatureFahrenheit( temperatur);
      }
    }

    out_values[i].value = values[i];
    out_values[i].unit = unit;

    

    

    
    }
    xSemaphoreGive( OneWireBusMtx );
  //  Serial.println("Data arrived");
    if( true == xSemaphoreTake(  DataxSemaphore , 2 ) ){
        //Serial.println("Sync Data");
          for(uint8_t i=0;i<4;i++){
            LastData[i].value=values[i];;
            LastData[i].unit=unit;
            memcpy( LastData[i].addr , sensor[i].addr, sizeof( sensor[i].addr ) );
          }

         
         xSemaphoreGive( DataxSemaphore );
  //       Serial.println("Sync done");
    }
  //   Serial.println("Reading done");
  } else {
    Serial.println("END Reading Sensors; MTX Error2 ");
    return;
  }
  
  
    
    

} // End of function 


void DS18x20_ReadMapped(float* out_values , uint8_t count  ,  Temperatur_Unit_t unit ){
  Sensor_Value values[4];
  if( count >4 ){
    count = 4;
  }

  for(uint8_t i=0;i<count;i++){
    out_values[i]=NAN;
  }
  
  DS18x20_ReadActivSensors( &values[0] ,  4  ,  unit );
  DS18x20_MapValues(out_values, count, &values[0], 4);
  

}

void DS18x20_MapValues( float* out_values, uint8_t out_values_count , Sensor_Value* values, uint8_t values_count ) {


   for (uint8_t i = 0 ; i < ( sizeof( sensor ) / sizeof( sensor[i]  ) ); i++){

      if( ( sensor[i].channelmapping >= 0 ) && ( sensor[i].channelmapping  < out_values_count ) ){
      
          if(i<values_count){
            //Serial.printf("Map reading %u to output %u  with value %f\n\r",i ,sensor[i].channelmapping, values[i].value);
            out_values[sensor[i].channelmapping] = values[i].value;
          }
      }
  }

}

void DS18x20_GetLastData( Sensor_Value* values, uint8_t values_count ){

  if(values_count>4){
    values_count=4;
  }
  if( xSemaphoreTake( DataxSemaphore, portMAX_DELAY  ) == pdTRUE ){
    for(uint8_t i=0;i<values_count;i++){
      memcpy(&values[i], &LastData[i] , sizeof( Sensor_Value ) );
    }
    xSemaphoreGive( DataxSemaphore );
  }

}


float DS18x20_GetTemperatureFahrenheit( float celsius ){
    return ( celsius*1.8)+32;
}

uint8_t DS18x20_GetDeviceCount( void ){

  uint8_t count = 0;
  for(uint8_t i=0;i<( sizeof(  sensor ) / sizeof( sensor[0] ) ); i++){
      if( ( sensor[i].addr[0] != 0x10 ) && ( sensor[i].addr[0] != 0x28 ) && ( sensor[i].addr[0] != 0x22 )   ){
        count = count;
      } else {
        count++;
      }
  }
  return count;

}

uint8_t DS18x20_GetConfiguredDeviceCount( void ){

  uint8_t count = 0;
  for(uint8_t i=0;i<( sizeof(  sensor ) / sizeof( sensor[0] ) ); i++){
      if( ( configured_sensor[i].addr[0] != 0x10 ) && ( configured_sensor[i].addr[0] != 0x28 ) && ( configured_sensor[i].addr[0] != 0x22 )   ){
        count = count;
      } else {
        count++;
      }
  }
  return count;

}

bool DS18x20_MissingSensors( void ){

  /*  
      we need to check if the configure sensors ( active ) 
      don't match the current config
  */
  
  bool error = false;

  for( uint8_t i=0;i<4;i++){
    if( (configured_sensor[i].channelmapping >= 0 ) && 
      (  ( configured_sensor[i].addr[0] == 0x10 ) || ( configured_sensor[i].addr[0] == 0x28 ) || ( configured_sensor[i].addr[0] == 0x22 )   )   ) {
      /* We need to check if the address is in the active pool */ 
      bool found = false;
      for( uint8_t x=0;x<4;x++){
        
        if( 0 == memcmp(  (void*)configured_sensor[i].addr, (void*)sensor[x].addr , sizeof(sensor[x].addr) ) )  {
          /* we found the sensor */
          found |= true;
          break;
        }
      
      }

      if( found == false ){
        Serial.printf("Error Sensor %u not found \n\r", i   );
        error |= true;
      }

    }
  }

}

uint8_t DS18x20_GetAllSensorID(OneWireBusSensor_t* Sensors, uint8_t capacity ){

  uint8_t used_capacity=0;
  /* we need to grab all sensors we have on the onewirebus and those missing ones */
  /* this means the fist byte must be 0x10 , 0x28 or 0x22 */
  bzero(Sensors, (sizeof(OneWireBusSensor_t)*capacity) ) ;
  for(uint8_t i = 0 ; i <4 ;i++){
    Serial.printf("1-Wire Sensor %u:", i);
    DS18x20_PrintSensorAddress(sensor[i].addr, true);
  }

  for(uint8_t i = 0 ; i <4 ;i++){
    Serial.printf("Configured Sensor %u:", i);
    DS18x20_PrintSensorAddress(configured_sensor[i].addr, true);
  }




  for(uint8_t i=0; i<4;i++){
    // We run through the found sensors
    if( ( sensor[i].addr[0] == 0x10 ) || ( sensor[i].addr[0] == 0x28 ) || ( sensor[i].addr[0] == 0x22 )    ){
      /* used_capacity needs to be smaller then the delivered one */
      if(capacity>used_capacity){
        memcpy( Sensors[used_capacity].addr , sensor[i].addr, sizeof( Sensors[used_capacity].addr ) );
        Sensors[used_capacity].onbus = true;
        Sensors[used_capacity].mappedchannel = sensor[i].channelmapping;
        Serial.printf("Add Sensor:");
        DS18x20_PrintSensorAddress(Sensors[used_capacity].addr, true);

        used_capacity++;
                            
      }
    }

    /* Next is the amount of configured sensors */
    if( ( configured_sensor[i].addr[0] == 0x10 ) || ( configured_sensor[i].addr[0] == 0x28 ) || ( configured_sensor[i].addr[0] == 0x22 )  ){
      /* used_capacity needs to be smaller then the delivered one */
      if(capacity>used_capacity){
        /* We need to check if the sensor is already in list */
        bool in_list = false;
        for(uint8_t x= 0 ; x < capacity ; x++ ){
            
            if ( 0 == memcmp ( Sensors[x].addr,  configured_sensor[i].addr , sizeof(  Sensors[x].addr ) )   ){
              // Already in list 
              in_list |= true;
            } else {
              // We need to add it 
              in_list |= false;
            }
        }
        
        if( false == in_list ){
            memcpy( Sensors[used_capacity].addr , configured_sensor[i].addr, sizeof( Sensors[used_capacity].addr ) );
            Sensors[used_capacity].onbus = false;
            Sensors[used_capacity].mappedchannel = configured_sensor[i].channelmapping;
            Serial.printf("Add Missing Sensor:");
            DS18x20_PrintSensorAddress(Sensors[used_capacity].addr, true);

            used_capacity++;
        } else {
          /* is in list so we don't care */
        }

       }

      }

    }

  return used_capacity;

}

float Ds18x20_GetMappedChannelValue( uint8_t channel ){
  /* we need to fetch the value  */
  if(channel >= 4 ){
    channel = 3;
  }
  return Sensor_Mapped_LastValue[channel]; //If this is NAN this means there is an error
}


void ds18x20_task ( void* param ){

// New values will be put into one queue with size of one result 
// Receiver will queue the elements but be able to wait untill 
// there are new values 


Sensor_Value values[4];
uint8_t errorcounter=0;
// Every 200s we send one value to the history 

while ( 1 == 1 ){

   
      /* We found at least on sensor lets check if the config matches */
      if( true == DS18x20_MissingSensors( ) ){
        Serial.println(" Config or Bus is wrong, missing DS18x20 sensor");
        DS18x20_ReadActivSensors( &values[0] ,  4  ,  T_Celsius );
        vTaskDelay( 1000 );
      } else {
          errorcounter=0;
          /* If we end up here we had once a complete config....*/
          while ( errorcounter < 4 ){
              /* We check if the config is still valid, if not we read the best we can */
              uint8_t device_count = DS18x20_GetConfiguredDeviceCount();
              DS18x20_ReadActivSensors( &values[0] ,  4  ,  T_Celsius );
              DS18x20_MapValues(&temperatur[0], device_count, &values[0], 4);

            for(uint8_t i=0;i< 4 ;i++){
                if( (true == DS180x20_IsChannelMapped(i) ) && ( isnan(temperatur[i]) == true) ) {
                  Serial.printf("Channel %u read failed\n\r", i);
                  errorcounter++;
                } else {
                  /*
                    Serial.print("  Temperature = ");
                    Serial.print(temperatur[i]);
                    Serial.print(" Celsius\n\r");
                   */
                   //Serial.printf("Send Data for CH %u to AVG with value %f \n\r", i, temperatur[i] );
                   if((true == DS180x20_IsChannelMapped(i) )){
                    //data_hist_avg::AddAVG_temperatur( (TemperatureChannel_en)((uint8_t)TemperatureChannel_en::TCH_Bat1+i), temperatur[i]  );
                    Sensor_Mapped_LastValue[i]=temperatur[i];
                   }
                }
              }
              vTaskDelay( 1000 );
            }
            /* if we end here something went wrong */

      }
  }

}

bool DS180x20_IsChannelMapped( uint8_t channel ){


  bool mapped = false;
  for( uint8_t i = 0; i< 4 ; i++ ){
    if(channel == configured_sensor[i].channelmapping ){
      mapped |= true;
    }
  }

  for( uint8_t i = 0; i< 4 ; i++ ){
    if(channel == sensor[i].channelmapping ){
      mapped |= true;
    }
  }


  return mapped;
}





void DS18x20_PrintSensorAddress(uint8_t* addr, bool newline = false){
 for(uint8_t i=0;i<7;i++){
   Serial.printf("%02X:", addr[i]);
 }

 if(false == newline){
   Serial.printf("%02X", addr[7]);
 } else {
   Serial.printf("%02X\n\r", addr[7]);
 }


                            
}


