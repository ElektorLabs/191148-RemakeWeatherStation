#ifndef _SENSEBOX_H_
 #define _SENSEBOX_H_

    class SenseBoxUpload {

        public:
            SenseBoxUpload( void );
            ~SenseBoxUpload( void );

            void begin( bool usehttps=true);
            bool postdata( void );
            
        private:
        bool usesecure=true;
        WiFiClient client;
        WiFiClientSecure clientS;
        

    };

#endif