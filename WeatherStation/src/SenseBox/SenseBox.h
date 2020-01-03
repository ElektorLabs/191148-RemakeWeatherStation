#ifndef _SENSEBOX_H_
 #define _SENSEBOX_H_
 #include <Arduino.h>
 #include <WiFi.h>
 #include <WiFiClientSecure.h>
 

    class SenseBoxUpload {

        public:
            SenseBoxUpload( void );
            ~SenseBoxUpload( void );

            typedef struct{
                bool enable;
                uint8_t StationChannelIdx;
                char SenseBoxSensorID[32]; //We reserve 32 chars for this..... 
            } SensBoxMapping_t;

            typedef bool(*DataAccesFnc)(float*,uint8_t);

            typedef struct {
                bool Enabled;
                uint16_t UploadInterval;
                char SenseBoxID[32]; //We reserve 32 chars for this..... 
            } SenseBoxSettings_t;

            typedef struct{
                SenseBoxUpload* obj;
                SemaphoreHandle_t CfgSem;
            } TaskData_t;

         

            void begin( bool usehttps=true);
            bool PostData( void );
            void RegisterDataAccess(DataAccesFnc Fnc);
           //We ned some functions to get and set settings
            void SetSensBoxID( String ID );
            void SetSensBoxEnable( bool Enable );
            void SetSensBoxUploadInterval( uint16_t Interval ); //65535 minutes Max ( 45 days )
            void SetMapping(uint8_t Channel, SensBoxMapping_t Mapping);

            String GetSensBoxID( void );
            bool GetSensBoxEnable( void );
            uint16_t SetSensBoxUploadInterval( void );
            SensBoxMapping_t GetMapping(uint8_t Channel );

            
        private:
        bool usesecure=true;
        WiFiClient* client;
        WiFiClientSecure* clientS;

        SensBoxMapping_t Mapping[16];
        SenseBoxSettings_t Settings;
        DataAccesFnc DaFnc=nullptr;

       

        TaskData_t TaskData;
        void configureClient();
        //We write the whole mapping to disk( why not ....)
        void ReadSettings( void );
        void WriteSettings( void );
        void WriteMapping( void );
        void ReadMapping( void );

        static bool PostData( SenseBoxUpload* obj);
        
        static String performRequest(WiFiClient* c , String host, String url, int port = 80, String method = "GET", String headers = "Connection: close\r\n", String data = "") ;  
        //Own UploadTask.?!
        static void UploadTaskFnc(  void* params );
        

    };

   

#endif