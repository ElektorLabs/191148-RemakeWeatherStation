#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

class ChargerEvent{

    public :
        typedef enum{
            NOP=0,
            Abort,
            NewTemperature,
            NewCurrent,
            NewVoltage,
            NewError,
            NewStatus,
            NewAvgVoltages,
            NewAvgCurrent,
            NewAvgTemperatures,
            ChargerEventCNT
        } ChargerEvent_t;

    private:
};


#endif