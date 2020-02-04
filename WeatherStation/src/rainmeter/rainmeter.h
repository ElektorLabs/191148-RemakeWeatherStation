#include <Arduino.h>

class Rainmeter{

    private:
    
        SemaphoreHandle_t xSemaphore;
        uint8_t PIN;
        static void  CalculateRainAmount( Rainmeter* obj);
        void Add10MinuteValue( uint8_t pulsecount );
        uint8_t RainAmount24h [ 144 ]={0,}; //Every 10 Minutes 
        uint8_t oldest_idx=0;
        
    public:
        typedef enum {
            _30Minutes=0,
            _60Minutes,
            _720Minutes,
            _1440Minutes
        } RainAmountTimespan_t;
        
        Rainmeter( void );
        ~Rainmeter( void );
    
        void begin( int Pin  );
        float GetRainAmount( RainAmountTimespan_t Span);
        void RainBucketISR( void );
        

};