#include "InternalSensors.h"
#include "../winddir/WindDirSensor.h"
#include "../windspeed/WindSpeedSensor.h"
#include "../rainmeter/rainmeter.h"

WindDirSensor WindDirection;
WindSpeedSensor WindSpeed;
Rainmeter RainMeter;

/**************************************************************************************************
 *    Function      : InternalSensors
 *    Description   : Constuctor
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
InternalSensors::InternalSensors( void ){

}

/**************************************************************************************************
 *    Function      : InternalSensors
 *    Description   : Destructor
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
InternalSensors::~InternalSensors( void ){

}


/**************************************************************************************************
 *    Function      : begin
 *    Description   : This will setup the internal sensors
 *    Input         : int pin_winddir , int pin_windspeed, int pin_rainmeter
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void InternalSensors::begin( int pin_winddir , int pin_windspeed, int pin_rainmeter ){
        WindDirection.begin(pin_winddir);
        WindSpeed.begin(pin_windspeed);
        RainMeter.begin(pin_rainmeter);
}

/**************************************************************************************************
 *    Function      : GetValue
 *    Description   : Will read a value from one sensors
 *    Input         : DATAUNITS::MessurmentValueType_t, uint8_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
float InternalSensors::GetValue( DATAUNITS::MessurmentValueType_t Type, uint8_t channel ){
    float value = NAN;
        switch(Type){
            case DATAUNITS::DIRECTION:{
                switch(channel){
                    case 0:{
                        value = WindDirection.GetAverageDir(WindDirSensor::DirAVG_t::_10Seconds);
                    } break;

                    case 1:{
                        value = WindDirection.GetAverageDir(WindDirSensor::DirAVG_t::_60Seconds);
                    }break;

                    case 2:{
                        value = WindDirection.GetAverageDir(WindDirSensor::DirAVG_t::_3600Seconds);
                    } break;

                    default:{
                        value = NAN;
                    }
                }
               
            } break;

            case DATAUNITS::SPEED:{
                switch(channel){
                    case 0:{
                        value = WindSpeed.GetAverageSpeed(WindSpeedSensor::SpeedAVG_t::_10Seconds);
                    } break;

                    case 1:{
                        value = WindSpeed.GetAverageSpeed(WindSpeedSensor::SpeedAVG_t::_60Seconds);
                    }break;

                    case 2:{
                        value = WindSpeed.GetAverageSpeed(WindSpeedSensor::SpeedAVG_t::_3600Seconds);
                    } break;
                       
                    default:{
                         value = NAN;
                    }
                }
            } break;

            case DATAUNITS::RAINAMOUNT:{
                switch(channel){
                case 0:{
                        value = RainMeter.GetRainAmount(Rainmeter::RainAmountTimespan_t::_30Minutes);
                    } break;

                    case 1:{
                        value = RainMeter.GetRainAmount(Rainmeter::RainAmountTimespan_t::_60Minutes);
                    }break;

                    case 2:{
                        value = RainMeter.GetRainAmount(Rainmeter::RainAmountTimespan_t::_720Minutes);
                    } break;

                    case 3:{
                        value = RainMeter.GetRainAmount(Rainmeter::RainAmountTimespan_t::_1440Minutes);
                    } break;
                       
                    default:{
                         value = NAN;
                    }
                }
            } break;

            default:{
                        value = NAN;
            } break;
        }
    return value;

}


/**************************************************************************************************
 *    Function      : GetConnectedSensorList
 *    Description   : Will return a list of connected internal sensors 
 *    Input         : SensorUnitInfo_t* List , uint8_t capacity, uint8_t* used_elements, bool mustbepresent
 *    Output        : bool
 *    Remarks       : none
 **************************************************************************************************/
bool InternalSensors::GetConnectedSensorList( SensorUnitInfo_t* List , uint8_t capacity, uint8_t* used_elements, bool mustbepresent){
    return GetSensorList( List, capacity,  used_elements);
}



/**************************************************************************************************
 *    Function      : GetSensorList
 *    Description   : Will return a list of connected internal sensors 
 *    Input         : SensorUnitInfo_t* List , uint8_t capacity, uint8_t* used_elements, bool mustbepresent
 *    Output        : bool
 *    Remarks       : none
 **************************************************************************************************/
bool InternalSensors::GetSensorList( SensorUnitInfo_t* List, uint8_t capacity, uint8_t* used_elements){
    /* Badly hard coded here but shall work */
    uint8_t dataused =0;
    for(uint8_t i=0;i<(sizeof(SensorInfo) / sizeof(SensorInfo[0]) );i++ ){
        if(dataused>=capacity){
            return false;
        } else {
            List[i] = SensorInfo[i];
            dataused++;
        }
        
    }
    *used_elements = dataused;
    return true;



}

/**************************************************************************************************
 *    Function      : GetChannelName
 *    Description   : Will return the friendly name of one channel
 *    Input         : SensorType_t Sensor, uint8_t channel
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String InternalSensors::GetChannelName(SensorType_t Sensor, uint8_t channel){

     switch(Sensor){
         case GPIO_SPEED:{
            if(channel>2){
                 return "N/A";
             } else {
                 if(2+channel>=10){
                     return "N/A";
                 } else {
                    return SensorNames[0+channel];
                 }
                 
             }
         } break;

         
         case GPIO_DIRECTION:{
             if(channel>2){
                 return "N/A";
             } else {
                 if(2+channel>=10){
                     return "N/A";
                 } else {
                    return SensorNames[3+channel];
                 }
             }
         } break;

         case GPIO_RAINAMOUNT:{
             if(channel>3){
                 return "N/A";
             } else {
                 //Boundary check
                 if(6+channel>=10){
                     return "N/A";
                 } else {
                     return SensorNames[6+channel];
                 }
                 
             }
         }break;

         default:{
            return "N/A";
         } break;
     
     }
    return "N/A";
 }


/**************************************************************************************************
 *    Function      : GetChannelName
 *    Description   : Will return the friendly name of one channel
 *    Input         : DATAUNITS::MessurmentValueType_t Type, uint8_t channel
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
 String InternalSensors::GetChannelName( DATAUNITS::MessurmentValueType_t Type, uint8_t channel ){
     
     String Result="N/A";

     for(uint8_t i=0;i< ( sizeof(SensorInfo) / sizeof(SensorInfo[0]) ); i++){
         if( ( SensorInfo[i].Type == Type ) && ( SensorInfo[i].ChannelID==channel )){
             Result = GetChannelName(SensorInfo[i].Sensor, channel);

         }
     }
     return Result;

 }