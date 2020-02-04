#include <Ticker.h>
#include <FunctionalInterrupt.h>
#include "rainmeter.h"

//We assume here 1870mm rain per 24h as max ( currently )
//Resulting at 66pulses max per 24 hours
Ticker RainMeterCapture;

/**************************************************************************************************
 *    Function      : Rainmeter
 *    Description   : Costructor
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
Rainmeter::Rainmeter( void){
    xSemaphore = xSemaphoreCreateCounting( 100, 0 );   
}

/**************************************************************************************************
 *    Function      : Rainmeter
 *    Description   : Destructor
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
Rainmeter::~Rainmeter( ){
    detachInterrupt( digitalPinToInterrupt(PIN) );
}

/**************************************************************************************************
 *    Function      : begin
 *    Description   : Will initialize the Rainmeter
 *    Input         : int Pin
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void Rainmeter::begin( int Pin ){
    PIN = Pin;
    pinMode(PIN, INPUT);
    RainMeterCapture.attach(600, CalculateRainAmount , this ); // Every 10 minute is fast enough for this
    attachInterrupt( digitalPinToInterrupt(PIN), std::bind(&Rainmeter::RainBucketISR,this), CHANGE );

}

/**************************************************************************************************
 *    Function      : CalculateRainAmount
 *    Description   : Will calulate the amount of rain for a given time 
 *    Input         : int Pin
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void Rainmeter::CalculateRainAmount( Rainmeter* obj){
  uint16_t pulsecount = uxSemaphoreGetCount ( obj->xSemaphore );
  xQueueReset(obj->xSemaphore);
  Serial.printf("Rain Pulsecount:%i\n\r", pulsecount);
}


/**************************************************************************************************
 *    Function      : Add10MinuteValue
 *    Description   : Will add the pulses to the floating average value
 *    Input         : uint8_t pulsecount
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void Rainmeter::Add10MinuteValue( uint8_t pulsecount ){
    RainAmount24h[oldest_idx]=pulsecount;
    oldest_idx++;
    if(oldest_idx>= ( sizeof( RainAmount24h ) / sizeof( RainAmount24h[0] ) ) ){
        oldest_idx=0;
    }
}

/**************************************************************************************************
 *    Function      : GetRainAmount
 *    Description   : Will calculate a floating average value for a given timespan
 *    Input         : RainAmountTimespan_t
 *    Output        : float
 *    Remarks       : none
 **************************************************************************************************/
float Rainmeter::GetRainAmount( RainAmountTimespan_t Span){
    float RainAmount=0;
    uint32_t rainpulses =0;
    uint8_t valuecount = 0;
    uint8_t startidx=0;
    switch(Span){
        case Rainmeter::_30Minutes:{
            valuecount=3;
        }break;

        case Rainmeter::_60Minutes:{
            valuecount=6;
        }break;

        case Rainmeter::_720Minutes:{
            valuecount=72;
        }break;

        case Rainmeter::_1440Minutes:{
            valuecount=144;
        }break;
    }

    startidx=oldest_idx+1;
    if(startidx>= ( sizeof( RainAmount24h ) / sizeof( RainAmount24h[0] ) ) ){
        startidx=0;
    }

    for(uint8_t i=0;i<valuecount;i++){
        rainpulses+= RainAmount24h[startidx];
        startidx++;
        if(startidx>= ( sizeof( RainAmount24h ) / sizeof( RainAmount24h[0] ) ) ){
            startidx=0;
        }

    }
    RainAmount = (float)rainpulses*0.33; //0.33mm per pulse
    return RainAmount;
    
}

/**************************************************************************************************
 *    Function      : RainBucketISR
 *    Description   : This will prcess a new isr form the rain bucket
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void Rainmeter::RainBucketISR( void ){
    static uint32_t last_isr = 0;
    uint32_t milli = millis();
    //IF we have more than 1 pulse per second we will ignore it
    //Also this will be a bit tricky as we will again sample the 
    //I/O Pin after 500ms to see if it is stable as the magnet
    //Tends to trigger if vibrations are out there 
    if( (milli-last_isr) > 1000 ){
        last_isr=milli;
        if( xSemaphore != NULL ){
          xSemaphoreGiveFromISR( xSemaphore, NULL );
        }

    }
}




