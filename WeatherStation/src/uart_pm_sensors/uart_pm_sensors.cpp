#include "uart_pm_sensors.h"




UART_PM_Sensors::UART_PM_Sensors( int16_t rx, int16_t tx  ){

    //Depending on the sensor we will choose which driver to use
    TX = tx;
    RX = rx;
    xUARTSemaphore = xSemaphoreCreateMutex();
    if(NULL == xUARTSemaphore){
        abort();
    }
}

UART_PM_Sensors::~UART_PM_Sensors( void ){
    if(SDS011device != nullptr){
        delete SDS011device;
    }

    if(SerialPort != nullptr){
        SerialPort->end();
        /* this will ensure we can access the gpio pins afterwards */
        gpio_matrix_out(TX, SIG_GPIO_OUT_IDX, false, false);
        gpio_matrix_out(RX, SIG_GPIO_OUT_IDX, false, false);
    }
    
}

void UART_PM_Sensors::begin( HardwareSerial &_hwserial, SerialSensorDriver_t device ){

    SerialPort = &_hwserial;
    SelectedDriver = device;
    switch( device ){
         case SENSOR_SDS011:{ 
             Serial.print("Configure SDS011: ");
            _hwserial.begin( 9600, SERIAL_8N1 , RX , TX);
            SDS011device = new SDS011( _hwserial );   
            if (SDS011device->setSleepMode(1) ){
                if(true == SDS011device->setMode(SDS_SET_QUERY)){
                    Serial.println("Query Mode");
                    SensorDetected=true;
                } else {
                    Serial.println("failed");    
                }
                
            } else {
                Serial.println("failed");
            }
         } break;

         case SENSOR_HPM115S0:{
             Serial.print("Configure HPM115S0-XXX:");
             _hwserial.begin( 9600, SERIAL_8N1 , RX , TX);
             HPMA115S0device = new HPMA115S0( _hwserial );
             HPMA115S0device->begin( );
             if(false == HPMA115S0device->StopAutoSend() ){
                Serial.println(" failed");
             } else {
                 if( false == HPMA115S0device->StartMesurment() ){
                     Serial.println(" failed");
                 } else {
                     Serial.println(" Autosend disabled, sensor active");
                     SensorDetected=true;
                 }
             }

         } break;

        default:{
            Serial.println("HW not supported");  
         } break;

    }
}

bool UART_PM_Sensors::GetParticleCount( float* value, UART_PM_Sensors::ParticleSize_t ps ){
  bool reading_ok= false;
  float p25 = 0, p10 = 0;
  uint16_t up25, up10;
  float result = 0;
    if( xSemaphoreTake( xUARTSemaphore, portMAX_DELAY  ) == pdTRUE )
    {
        switch( SelectedDriver ){
            case SENSOR_SDS011:{
                if(SDS011device!=nullptr){
                    SDS011device-> setSleepMode(false);
                    SDS011device->getData(&p25, &p10);
                    SDS011device-> setSleepMode(true);
                    reading_ok = true;
                }
            } break;

            case SENSOR_HPM115S0:{
                if(HPMA115S0device != nullptr){
                    reading_ok=HPMA115S0device->ReadMesurement(&up25, &up10);
                }
            } break;

            default:{
                Serial.println("Invaild driver selected");
            }
        }
        if(true == reading_ok){
            switch( ps ){
                case  PM2_5:{
                    result = p25;
                } break;

                case PM10:{
                    result = p10;
                } break;
            }
        }
        xSemaphoreGive( xUARTSemaphore );
    }
    return reading_ok;

 }

void UART_PM_Sensors::suspend( void ){
     if( xSemaphoreTake( xUARTSemaphore, portMAX_DELAY  ) == pdTRUE )
    { 
     switch( SelectedDriver ){
        case SENSOR_SDS011:{ 
          if(SDS011device!=nullptr){
          }
        } break;

        case SENSOR_HPM115S0:{
            if(HPMA115S0device != nullptr){
                HPMA115S0device->StopMesurment();
            }
        } break;

        default:{
            Serial.println("Invaild driver selected");
        } break;
    }
     xSemaphoreGive( xUARTSemaphore );
    }
}

void UART_PM_Sensors::wakeup( void ){
     if( xSemaphoreTake( xUARTSemaphore, portMAX_DELAY  ) == pdTRUE )
    {
    switch( SelectedDriver ){
        case SENSOR_SDS011:{ 
          if(SDS011device!=nullptr){
          }
        } break;

        case SENSOR_HPM115S0:{
            if(HPMA115S0device != nullptr){
                HPMA115S0device->StartMesurment();
            }
        } break;

        default:{
            Serial.println("Invaild driver selected");
        } break;
    }
     xSemaphoreGive( xUARTSemaphore );
    }

}


float UART_PM_Sensors::GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel ){
    float value = NAN;
    
    if(DATAUNITS::MessurmentValueType_t::PARTICLES != Type){
        value = NAN;
    } else {
            switch(channel){
                case 0:{
                    //PM2.5
                    if(true == GetParticleCount(&value, UART_PM_Sensors::ParticleSize_t::PM2_5) ){
                        //Noting to do....
                    } else {
                        value = NAN;
                    }
                } break;

                case 1:{
                    //PM10
                    if(true == GetParticleCount(&value, UART_PM_Sensors::ParticleSize_t::PM10) ){
                        //Noting to do....
                    } else {
                        value = NAN;
                    }
                }break;

                default:{
                    value = NAN;
                } break;
            }
    }

}


bool UART_PM_Sensors::GetSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements){
    return GetConnectedSensorList(List,capacity,used_elements,false);
}
bool UART_PM_Sensors::GetConnectedSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements, bool mustbedetected){
       
        if( (true == mustbedetected ) && (false == SensorDetected) ){
            Serial.println("No Serial Sensors detected");
            return true;
        }
        switch( SelectedDriver ){
            case SENSOR_SDS011:{
                 *used_elements=0; 
                for(uint8_t i=0;i< ( sizeof(SDSSensorInfo) / sizeof( SDSSensorInfo[0] )) ; i++){
                    if( *used_elements >= capacity ){
                        Serial.println("UART SENSORLIST OUT OF MEMORY");
                        return false;
                    }
                    List[i] = SDSSensorInfo[i];
                    *used_elements=*used_elements+1;
                }
            } break;

            case SENSOR_HPM115S0:{
                 *used_elements=0;
                for(uint8_t i=0;i< ( sizeof(HMP115SSensorInfo) / sizeof( HMP115SSensorInfo[0] ) ); i++){
                    if( *used_elements >= capacity ){
                        Serial.println("UART SENSORLIST OUT OF MEMORY");
                        return false;
                    }
                    List[i] = HMP115SSensorInfo[i];
                    *used_elements=*used_elements+1;
               }            
            } break;

            default:{
                Serial.println("Invaild driver selected");
                *used_elements=0;
                return false;
            } break;
    }

    return true;
}

String UART_PM_Sensors::GetChannelName(SensorType_t Sensor, uint8_t channel){

    switch( SelectedDriver ){
            case SENSOR_SDS011:{ 
               if( channel < ( sizeof(SDS011ChannelNames) / sizeof( SDS011ChannelNames[0] ) ) ){
                   return SDS011ChannelNames[channel];
               } else {
                    return "N/A";
               }
            } break;

            case SENSOR_HPM115S0:{
                 if(channel < ( sizeof(HMP115SChannelNames) / sizeof( HMP115SChannelNames[0] ) ) ){
                     return HMP115SChannelNames[channel];
                 } else {
                     return "N/A";
                 }
            } break;

            default:{
                Serial.println("Invaild driver selected");
                return "N/A";
            } break;
    }
    return "N/A";

}

String UART_PM_Sensors::GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel){
    if(Type==DATAUNITS::MessurmentValueType_t::PARTICLES){
        switch( SelectedDriver ){
                case SENSOR_SDS011:{ 
                if( channel < ( sizeof(SDS011ChannelNames) / sizeof( SDS011ChannelNames[0] ) ) ){
                    return SDS011ChannelNames[channel];
                } else {
                        return "N/A";
                }
                } break;

                case SENSOR_HPM115S0:{
                    if(channel < ( sizeof(HMP115SChannelNames) / sizeof( HMP115SChannelNames[0] ) ) ){
                        return HMP115SChannelNames[channel];
                    } else {
                        return "N/A";
                    }
                } break;

                default:{
                    Serial.println("Invaild driver selected");
                    return "N/A";
                } break;
        }
    } else {
        return "N/A";
    }

    return "N/A";
}


void UART_PM_Sensors::SDS011_TEST( void ){


  // put your main code here, to run repeatedly:
  Serial.println("getting firmware version");
  bool mode = false;
  float p25 = 0, p10 = 0;
  bool sleepMode = false;
  int per = 0;
  byte ver[3] = {0};
  SDS011device->getMode(&mode);
  delay(100);
  SDS011device->getData(&p25, &p10);
  delay(100);
  SDS011device->getSleepMode(&sleepMode);
  delay(100);
  SDS011device->getWorkingPeriod(&per);
  delay(100);
  SDS011device->getFirmwareVersion(ver);

  Serial.println("mode: " + String(mode));
  Serial.println("data: p2.5: " + String(p25) + ", p10: " + String(p10));
  Serial.println("sleepmode: " + String(sleepMode));
  Serial.println("period: " + String(per));
  Serial.println("version: " + String(ver[2]) + "/" + String(ver[1]) + "/" + String(ver[0]));
  delay(5000);

}
 



