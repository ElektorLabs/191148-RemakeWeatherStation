#ifndef _SENSEBOX_H_
 #define _SENSEBOX_H_
 #include <Arduino.h>

    class SenseBoxUpload {

        public:
            SenseBoxUpload( void );
            ~SenseBoxUpload( void );

            void begin( bool usehttps=true);
            bool postdata( void );

            typedef struct{
                bool enable;
                uint8_t StationChannelIdx;
                char SenseBoxSensorID[32]; //We reserve 32 chars for this..... 
            } SensBoxMapping_t;



            typedef struct {
                float LocationLong;
                float LocationLang;
                char SenseBoxID[32]; //We reserve 32 chars for this..... 
            } SenseBoxSettings_t;

            //We write the whole mapping to disk( why not ....)
            void WriteMapping( void );
            void ReadMapping( void );
            
        private:
        bool usesecure=true;
        WiFiClient client;
        WiFiClientSecure clientS;

        SensBoxMapping_t Mapping[16];
        SenseBoxSettings_t Settings;
        

    };

#endif