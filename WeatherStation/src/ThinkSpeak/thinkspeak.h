#ifndef _THINKSPEAK_H_
 #define _THINKSPEAK_H_
    /*  
        This module is here for backward compability with the current station out
        Written just to add support for Thinkspeak to the station but not preffered
        Still remember there is no cloud, just someone else computer out there
    */ 
    #include <Arduino.h>
    #include <Wifi.h>
    #include <WiFiClientSecure.h>

    class ThinkspeakUpload{

        public:
        ThinkspeakUpload( void );
        ~ThinkspeakUpload( void );

            typedef struct{
                bool enable;
                uint8_t StationChannelIdx;
            } ThinkspeakMapping_t;

            typedef bool(*DataAccesFnc)(float*,uint8_t);

            typedef struct {
                bool Enabled;
                uint16_t UploadInterval;
                //char ChannelID[16]; //Seems only be requiered for a read , we won't support here
                char ThingspealAPIKey[32]; //We reserve 32 chars for this..... 
            } ThinkspeakSettings_t;

            typedef struct{
                ThinkspeakUpload* obj;
                SemaphoreHandle_t CfgSem;
            } TaskData_t;


            void begin( bool usehttps=true );
            bool PostData( void );
            void RegisterDataAccess(DataAccesFnc Fnc);

            //We ned some functions to get and set settings
            void SetThinkspeakAPIKey( String ID ); //WriteKey here expected
            void SetThinkspeakEnable( bool Enable );
            void SetThinkspeakUploadInterval( uint16_t Interval ); //65535 minutes Max ( 45 days )
            void SetMapping(uint8_t Channel, ThinkspeakMapping_t Mapping);

            String GetThinkspeakAPIKey( void );
            bool GetThinkspeakEnable( void );
            uint16_t GetThinkspeakUploadInterval( void );
            ThinkspeakMapping_t GetMapping(uint8_t Channel );
            uint8_t GetMaxMappingChannels( void );

            
        private:
        
        bool usesecure=true; 
        WiFiClientSecure* clientS;
        WiFiClient* client;
        
        ThinkspeakMapping_t Mapping[8]; //This max supported by Thinkspeak for one "Channel" or Device
        ThinkspeakSettings_t Settings;
        DataAccesFnc DaFnc=nullptr;


        TaskData_t TaskData;
        void configureClient();
        //We write the whole mapping to disk( why not ....)
        void ReadSettings( void );
        void WriteSettings( void );
        void WriteMapping( void );
        void ReadMapping( void );

        static bool PostData( ThinkspeakUpload* obj);
        
        static String performRequest(WiFiClient* c , String host, String url, int port = 80, String method = "GET", String headers = "Connection: close\r\n", String data = "")   ;
        //Own UploadTask.?!
        static void UploadTaskFnc(  void* params );
    };

#endif