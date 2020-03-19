#include <Ticker.h>
#include "WindDirSensor.h"


/* This is code that will be shifted to the ULP in the future */
Ticker WindDirCapture;

/**************************************************************************************************
 *    Function      : WindDirSensor
 *    Description   : Constructor
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
WindDirSensor::WindDirSensor() {
  

}

/**************************************************************************************************
 *    Function      : determineWindDir
 *    Description   : This will try to determine the wind direction
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void WindDirSensor::determineWindDir( WindDirSensor* obj ) {
  /* We add here 3 averages , 10 seconds , 1 minute and one hour */
  WindDir_t WindDir = WindDir_t::DIR_UNDEFINED;
  
  uint16_t dir = analogRead(obj->dirPin);
  uint8_t u8_dir =  ( dir & 0xF00 ) / 256 ;

  if(u8_dir < 15){
    WindDir = obj->WindDirLUT[u8_dir];   
  }


  /* This is for debug only */
  #ifdef DEBUG_SERIAL
  Serial.printf("Dir-ADC Value:%i\n\r", dir);
  Serial.printf("U8 Value:%i\n\r", u8_dir);
  Serial.printf("Enum Value:%i\n\r", WindDir);
 #endif
  
  obj->AddToAverage_10Sec(WindDir);

  
}


/**************************************************************************************************
 *    Function      : begin
 *    Description   : Sets up the hardware for the direction reading
 *    Input         : int _dirPin
 *    Output        : void
 *    Remarks       : Needs an analouge pin to work
 **************************************************************************************************/
void WindDirSensor::begin( int _dirPin ){
  dirPin = _dirPin;
  pinMode(dirPin, ANALOG);
  WindDirCapture.attach_ms(1000, determineWindDir , this );
}


/**************************************************************************************************
 *    Function      : WindDirEnum2Angle
 *    Description   : Gets from the sector readed an angle in °
 *    Input         : WindDir_t dir
 *    Output        : uint16_t
 *    Remarks       : None
 **************************************************************************************************/
uint16_t WindDirSensor::WindDirEnum2Angle( WindDir_t dir){
  if( dir < WindDir_t::DIR_UNDEFINED){
    return 45*dir;
  } else {
    return 360;
  } 
}



/**************************************************************************************************
 *    Function      : AddToAverage_10Sec
 *    Description   : Adds the value to the average ( 10s )
 *    Input         : WindDir_t dir
 *    Output        : void
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
void WindDirSensor::AddToAverage_10Sec(WindDir_t dir ){
    static uint8_t _10SecondAVG_oldest_idx=0;
    _10SecondAVG[_10SecondAVG_oldest_idx]=dir;
    _10SecondAVG_oldest_idx++;
    if(_10SecondAVG_oldest_idx>9){
      //We also add the value to the 60 sec avg 
      _10SecondAVG_oldest_idx=0;
      AddToAverage_60Sec( GetAverage_10Sec() );
    }

}

/**************************************************************************************************
 *    Function      : AddToAverage_60Sec
 *    Description   : Adds the value to the average ( 60s )
 *    Input         : WindDir_t dir
 *    Output        : void
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
void WindDirSensor::AddToAverage_60Sec(WindDir_t dir ){
    static uint8_t _60SecondAVG_oldest_idx=0;
    _60SecondAVG[_60SecondAVG_oldest_idx]=dir;
    _60SecondAVG_oldest_idx++;
    if(_60SecondAVG_oldest_idx>5){
      //We also add the value to the 3600s average
      _60SecondAVG_oldest_idx=0;
      AddToAverage_3600Sec ( GetAverage_60Sec() );
    }

}

/**************************************************************************************************
 *    Function      : AddToAverage_3600Sec
 *    Description   : Adds the value to the average ( 3600s )
 *    Input         : WindDir_t dir
 *    Output        : void
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
void WindDirSensor::AddToAverage_3600Sec(WindDir_t dir ){
    static uint8_t _3600SecondAVG_oldest_idx=0;
    _3600SecondAVG[_3600SecondAVG_oldest_idx]=dir;
    _3600SecondAVG_oldest_idx++;
    if(_3600SecondAVG_oldest_idx>59){
      _3600SecondAVG_oldest_idx=0;
    }
    

}

/**************************************************************************************************
 *    Function      : GetAverage_10Sec
 *    Description   : Get the averrage for 10s
 *    Input         : void
 *    Output        : WindDir_t
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
WindDir_t WindDirSensor::GetAverage_10Sec( void ){
  
  uint32_t dir_angle = 0;

  for( uint8_t i=0; i<10; i++ ){
    dir_angle += WindDirEnum2Angle(_10SecondAVG[i]);
  }

  dir_angle = dir_angle / 10 ;
  dir_angle = dir_angle % 360;
  // We now have an angle between 0° and 359°
  // Next is to divide it by 45° to get the enmum value
  uint8_t dir_en = dir_angle / 45;
  
  return (WindDir_t)dir_en;
}

/**************************************************************************************************
 *    Function      : GetAverage_60Sec
 *    Description   : Get the averrage for 60s
 *    Input         : void
 *    Output        : WindDir_t
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
WindDir_t WindDirSensor::GetAverage_60Sec( void ){

  uint32_t dir_angle = 0;

  for( uint8_t i=0; i<6; i++ ){
    dir_angle += WindDirEnum2Angle(_60SecondAVG[i]);
  }

  dir_angle = dir_angle / 10 ;
  dir_angle = dir_angle % 360;
  // We now have an angle between 0° and 359°
  // Next is to divide it by 45° to get the enmum value
  uint8_t dir_en = dir_angle / 45;
  return (WindDir_t)dir_en;

}

/**************************************************************************************************
 *    Function      : GetAverage_3600Sec
 *    Description   : Get the averrage for 60s
 *    Input         : void
 *    Output        : WindDir_t
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
WindDir_t WindDirSensor::GetAverage_3600Sec( void ){

  uint32_t dir_angle = 0;

  for( uint8_t i=0; i<6; i++ ){
    dir_angle += WindDirEnum2Angle(_3600SecondAVG[i]);
  }

  dir_angle = dir_angle / 10 ;
  dir_angle = dir_angle % 360;
  // We now have an angle between 0° and 359°
  // Next is to divide it by 45° to get the enmum value
  uint8_t dir_en = dir_angle / 45;
  return (WindDir_t)dir_en;

}

/**************************************************************************************************
 *    Function      : GetAverageDir 
 *    Description   : Get the averrage for a define span
 *    Input         : DirAVG_t type
 *    Output        : uint16_t
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
uint16_t WindDirSensor::GetAverageDir( DirAVG_t type ){
  WindDir_t dir = WindDir_t::DIR_UNDEFINED;
  switch(type){

    case DirAVG_t::_10Seconds:{

      dir = GetAverage_10Sec();

    } break;

    case DirAVG_t::_60Seconds:{
      
      dir = GetAverage_10Sec();

    } break;
    

    case DirAVG_t::_3600Seconds:{

      dir = GetAverage_10Sec();

    } break;

    default:{

    } break;

  }

  return WindDirEnum2Angle ( dir );


}










