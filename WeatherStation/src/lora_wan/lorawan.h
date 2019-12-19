#ifndef _LORAWAN_H_
 #define _LORAWAN_H_
 
 /* this is the lora wan basic interface */
 /* we transmitt temperature, humidity and pressure as 16Bit values */
 /* temperature will be in 1/100 kelvin 0.00° to */ 
 
 #include <SPI.h>
 #include <lmic.h>


    class lorawan{
        public:
            bool begin( uint8_t NSS, uint8_t RXTX, uint8_t RST, uint8_t DIO0, uint8_t DIO1, uint8_t DIO2  );
            static void do_send(osjob_t* j); //This is not nice we need something different here
            static void LoRaWAN_Task( void );
        private:
          
            
            

    };
    

#endif