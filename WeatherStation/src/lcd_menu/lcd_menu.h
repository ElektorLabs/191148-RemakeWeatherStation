//We need a way to lock access to the i2c bus
//else we get a mess with the i2c sensors attached

//We need the display we like to talk to 
//And also we need the input buttons we need to access
//This will assume a PCF8574A / PCF8574 / PCF8574T based i2c to HD44780 converter
//Adresses are 0x20 to 0x27 and 0x38 to 0x3F we only support 0x27 for now
//and a LCD 16 Char x 2 Lines
#ifndef _LCDMENU_H_
 #define _LCDMENU_H_
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


  void LCDMenu( int16_t btn, int16_t btn2, uint8_t menuitem );
  void LCDMenuShow( uint8_t Menu ); 
  uint32_t ReadButtonPressCnt( void );
  void ResetButtonPressCnt( void );
  

#endif