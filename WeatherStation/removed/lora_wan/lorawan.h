#ifndef _LORAWAN_H_
 #define _LORAWAN_H_
 
 /* this is the lora wan basic interface */
 /* This will be different from the other interfaces as only some values will be transmitted */
 /* and we only have 51byte of payload here */
 
 #include <SPI.h>
 #include <lmic.h>


    class lorawan{
        public:
            bool begin( uint8_t NSS, uint8_t RXTX, uint8_t RST, uint8_t DIO0, uint8_t DIO1, uint8_t DIO2  );
            static void do_send(osjob_t* j); 
            static void LoRaWAN_Task( void );
        private:
          
            
            

    };
    

#endif