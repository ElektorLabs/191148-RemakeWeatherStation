#ifndef _DATAUNITS_H_
#define _DATAUNITS_H_

class DATAUNITS {
    public:

    typedef enum {
            TEMPERATURE=0,
            PRESSURE,
            HUMIDITY,
            UV_LIGHT,
            LIGHT,
            SPEED,
            RAINAMOUNT,
            DIRECTION,
            PARTICLES,
            MESSUREMENTVALUE_CNT
        } MessurmentValueType_t;
    
     

    static String GetValueTypeName(MessurmentValueType_t Type ){
        String ValueTypeName[]{
         "Temperature",
         "Pressure",
         "Humidity",
         "UVLight",
         "Light",
         "Speed",
         "Rainamount",
         "Direction",
         "Particles"
        };
        return ValueTypeName[(uint8_t)(Type)];
    }

     

};

#endif