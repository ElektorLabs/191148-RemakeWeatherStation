#include <Ticker.h>
#include <FunctionalInterrupt.h>
#include "../isr_fnc/timefunctions.h"
#include "WindSpeedSensor.h"


/* This is code that will be shifted to the ULP in the future */
Ticker WindSpeedCapture;

/**************************************************************************************************
 *    Function      : WindSpeedSensor 
 *    Description   : Constructor
 *    Input         : void 
 *    Output        : void
 *    Remarks       : Values shall be put every 10 seconds
 **************************************************************************************************/
WindSpeedSensor::WindSpeedSensor( void )  {
  xSemaphore = xSemaphoreCreateCounting( 500, 0 );
}

/**************************************************************************************************
 *    Function      : WindSpeedSensor 
 *    Description   : Destructror
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
WindSpeedSensor::~WindSpeedSensor() {
		detachInterrupt( digitalPinToInterrupt(PIN) );
	}


/**************************************************************************************************
 *    Function      : begin 
 *    Description   : Set up the sensor
 *    Input         : int _speedPin 
 *    Output        : void
 *    Remarks       : needs a digital io with interrupt capability
 **************************************************************************************************/
void WindSpeedSensor::begin( int _speedPin  ){
  PIN = _speedPin;
  pinMode(PIN, INPUT);
  WindSpeedCapture.attach_ms(1000, CalculateSpeed , this );
  attachInterrupt( digitalPinToInterrupt(PIN), std::bind(&WindSpeedSensor::WindSpeedPinISR,this), FALLING);
}

/**************************************************************************************************
 *    Function      : CalculateSpeed 
 *    Description   : calculates the current speed
 *    Input         : WindSpeedSensor* obj
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void WindSpeedSensor::CalculateSpeed( WindSpeedSensor* obj ){
  // This is called once a second 
  uint16_t pulsecount = uxSemaphoreGetCount ( obj->xSemaphore );
  xQueueReset(obj->xSemaphore);
  obj->AddToPulses_10Sec(pulsecount);
  /*
  Serial.printf("Pulsecount:%i\n\r",pulsecount);
  float wind_m_s = obj->GetAverageSpeed(WindSpeedSensor::SpeedAVG_t::_10Seconds);
  float wind = (float)pulsecount * 0.33;
  Serial.printf("Windspeed : %f m/s\n\r", wind  );
  Serial.printf("Windspeed AVG10: %f m/s\n\r", wind_m_s);
  */


}

/**************************************************************************************************
 *    Function      : AddToPulses_10Sec 
 *    Description   : Adds the pulses for a 10 second intervall
 *    Input         : uint32_t
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void WindSpeedSensor::AddToPulses_10Sec(uint32_t pulses ){
    static uint8_t _10SecondAVG_oldest_idx=0;
    _10SecondAVG[_10SecondAVG_oldest_idx]=pulses;
    _10SecondAVG_oldest_idx++;
    if(_10SecondAVG_oldest_idx>9){
      //We also add the value to the 60 sec avg 
      _10SecondAVG_oldest_idx=0;
      AddToPulses_60Sec( GetPulses_10Sec() );
    }

}

/**************************************************************************************************
 *    Function      : AddToPulses_60Sec 
 *    Description   : Adds the pulses for a 60 second intervall
 *    Input         : uint32_t
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void WindSpeedSensor::AddToPulses_60Sec(uint32_t pulses ){
    static uint8_t _60SecondAVG_oldest_idx=0;
    _60SecondAVG[_60SecondAVG_oldest_idx]=pulses;
    _60SecondAVG_oldest_idx++;
    if(_60SecondAVG_oldest_idx>5){
      //We also add the value to the 3600s average
      _60SecondAVG_oldest_idx=0;
      AddToPulses_3600Sec ( GetPulses_60Sec() );
    }

}

/**************************************************************************************************
 *    Function      : AddToPulses_3600Sec 
 *    Description   : Adds the pulses for a 3600 second intervall
 *    Input         : uint32_t
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void WindSpeedSensor::AddToPulses_3600Sec(uint32_t  pulses ){
    static uint8_t _3600SecondAVG_oldest_idx=0;
    _3600SecondAVG[_3600SecondAVG_oldest_idx]=pulses;
    _3600SecondAVG_oldest_idx++;
    if(_3600SecondAVG_oldest_idx>59){
      _3600SecondAVG_oldest_idx=0;
    }
    

}

/**************************************************************************************************
 *    Function      : GetPulses_10Sec 
 *    Description   : Gets the pulses for 10 seconds AVG
 *    Input         : void
 *    Output        : uint32_t
 *    Remarks       : None
 **************************************************************************************************/
uint32_t WindSpeedSensor::GetPulses_10Sec( void ){
  
  uint32_t pulses = 0;

  for( uint8_t i=0; i<10; i++ ){
    pulses += _10SecondAVG[i];
  }
  return pulses;
}

/**************************************************************************************************
 *    Function      : GetPulses_60Sec 
 *    Description   : Gets the pulses for 60 seconds AVG
 *    Input         : void
 *    Output        : uint32_t
 *    Remarks       : None
 **************************************************************************************************/
uint32_t WindSpeedSensor::GetPulses_60Sec( void ){

  uint32_t pulses = 0;

  for( uint8_t i=0; i<6; i++ ){
    pulses += _60SecondAVG[i];
  }
  return pulses;

}

/**************************************************************************************************
 *    Function      : GetPulses_3600Sec 
 *    Description   : Gets the pulses for 3600 seconds AVG
 *    Input         : void
 *    Output        : uint32_t
 *    Remarks       : None
 **************************************************************************************************/
uint32_t WindSpeedSensor::GetPulses_3600Sec( void ){

  uint32_t pulses = 0;
  for( uint8_t i=0; i<60; i++ ){
    pulses +=  _3600SecondAVG[i];
  }
  return pulses;

}

/**************************************************************************************************
 *    Function      : GetAverageSpeed 
 *    Description   : Gets the pulses average pluses for a given time
 *    Input         : SpeedAVG_t type
 *    Output        : float
 *    Remarks       : None
 **************************************************************************************************/
float WindSpeedSensor::GetAverageSpeed( SpeedAVG_t type ){
 float speed = 0;
 uint32_t pulses=0;
  switch(type){

    case SpeedAVG_t::_10Seconds:{

      pulses = GetPulses_10Sec();
      speed = (float)(pulses) *0.33; // m/s
      speed = speed / 10;

    } break;

    case SpeedAVG_t::_60Seconds:{
      
      pulses = GetPulses_60Sec();
      speed = (float)(pulses) *0.33; // m/s
      speed = speed / 60;

    } break;
    

    case SpeedAVG_t::_3600Seconds:{

      pulses = GetPulses_3600Sec();
      speed = (float)(pulses) *0.33; // m/s
      speed = speed / 3600;

    } break;

    default:{

    } break;

  }

  return speed ;
}





/**************************************************************************************************
 *    Function      : WindSpeedPinISR 
 *    Description   : Interrupt for a windspeed pulse
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
void IRAM_ATTR WindSpeedSensor::WindSpeedPinISR(){
    static uint32_t last_millis = 0;
    uint32_t millis = isr_millis();
    if(millis - last_millis > 3 ){
      last_millis = millis;
      //We count +1 for the pulses 
      if( xSemaphore != NULL ){
          xSemaphoreGiveFromISR( xSemaphore, NULL );
      }
    }
 

}

