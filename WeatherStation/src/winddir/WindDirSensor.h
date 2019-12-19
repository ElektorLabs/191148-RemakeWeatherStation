#ifndef WINDSENSOR_H
#define WINDSENSOR_H

#include "Arduino.h"

typedef enum {
  DIR_NORTH =0,
  DIR_NORTHEAST,
  DIR_EAST,
  DIR_SOUTHEAST,
  DIR_SOUTH,
  DIR_SOUTHWEST,
  DIR_WEST,
  DIR_NORTHWEST,
  DIR_UNDEFINED,
  DIR_DIRCOUNT
} WindDir_t;

class WindDirSensor {
  private:
  
  const WindDir_t WindDirLUT[16]={
    [0]=WindDir_t::DIR_EAST,
    [1]=WindDir_t::DIR_SOUTHEAST,
    [2]=WindDir_t::DIR_SOUTHEAST,
    [3]=WindDir_t::DIR_SOUTH,
    [4]=WindDir_t::DIR_UNDEFINED,
    [5]=WindDir_t::DIR_NORTHEAST,
    [6]=WindDir_t::DIR_NORTHEAST,
    [7]=WindDir_t::DIR_UNDEFINED,
    [8]=WindDir_t::DIR_UNDEFINED,
    [9]=WindDir_t::DIR_SOUTHWEST,
    [10]=WindDir_t::DIR_UNDEFINED,
    [11]=WindDir_t::DIR_NORTH,
    [12]=WindDir_t::DIR_UNDEFINED,
    [13]=WindDir_t::DIR_NORTHWEST,
    [14]=WindDir_t::DIR_UNDEFINED,
    [15]=WindDir_t::DIR_WEST
   };

  WindDir_t _10SecondAVG[10]={WindDir_t::DIR_UNDEFINED,};
  WindDir_t _60SecondAVG[6]={WindDir_t::DIR_UNDEFINED,};
  WindDir_t _3600SecondAVG[60]={WindDir_t::DIR_UNDEFINED,};
   
  int dirPin;
   
  
  uint16_t WindDirEnum2Angle( WindDir_t );
  void AddToAverage_10Sec(WindDir_t dir );
  void AddToAverage_60Sec(WindDir_t dir );
  void AddToAverage_3600Sec(WindDir_t dir );

  WindDir_t GetAverage_10Sec( void );
  WindDir_t GetAverage_60Sec( void );
  WindDir_t GetAverage_3600Sec( void );

  static  void determineWindDir( WindDirSensor* obj ) ;
  

  public:
    typedef enum {
      _10Seconds,
      _60Seconds,
      _3600Seconds,
    } DirAVG_t;

    WindDirSensor(void);
    void begin(int _dirPin);
    uint16_t GetAverageDir( DirAVG_t type );
};

#endif
