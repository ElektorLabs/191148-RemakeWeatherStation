#include <Arduino.h>

class WindSpeedSensor{

    private:

    typedef struct{
        uint32_t PulseDelta;
        uint32_t lastupdate;
    } SpeedPulse_t;

    uint8_t PIN;
    SemaphoreHandle_t xSemaphore;

    static void CalculateSpeed(  WindSpeedSensor* obj );

    uint16_t _10SecondAVG[10]={0,};
    uint16_t _60SecondAVG[6]={0,};
    uint16_t _3600SecondAVG[60]={0,};
    uint16_t _CurrentSpeed=0;
    SpeedPulse_t _WindSpeed={.PulseDelta=0,.lastupdate=0};
    
    void AddToPulses_10Sec(uint32_t pulses );
    void AddToPulses_60Sec(uint32_t pulses );
    void AddToPulses_3600Sec(uint32_t pulses );

    uint32_t GetPulses_10Sec( void );
    uint32_t GetPulses_60Sec( void );
    uint32_t GetPulses_3600Sec( void );
    
    public:
       typedef enum {
        _10Seconds,
        _60Seconds,
        _3600Seconds,
        } SpeedAVG_t;

        WindSpeedSensor(void );
        ~WindSpeedSensor();
        void begin( int _speedPin );
        void WindSpeedPinISR( void );
        float GetAverageSpeed( SpeedAVG_t type );
        float GetWindSpeed( void );
  
};
