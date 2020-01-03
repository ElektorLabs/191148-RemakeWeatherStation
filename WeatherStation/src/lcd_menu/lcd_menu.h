//We need a way to lock access to the i2c bus
//else we get a mess with the i2c sensors attached

//We need the display we like to talk to 
//And also we need the input buttons we need to access
//This will assume a PCF8574A / PCF8574 / PCF8574T based i2c to HD44780 converter
//Adresses are 0x20 to 0x27 and 0x38 to 0x3F we only support 0x27 for now
//and a LCD 16 Char x 2 Lines
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
class LCDMenu{

        public :
        
           static void begin( void );
            //We need a way to get a few informations and also
            //start a few actions (WPS ?! ), wake station
            //enable WiFI......
            //Also this will handle the button(s)

        private:

        static LiquidCrystal_I2C* lcd;
        static void DefaultDisplay( void );
        static void EnableLCD( void );
        static void DisableLCD( void );
            
};