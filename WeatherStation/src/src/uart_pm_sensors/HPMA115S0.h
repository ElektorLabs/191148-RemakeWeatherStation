#ifndef _HPMA115S0_H_
 #define _HPMA115S0_H_
 
#include <Arduino.h>

    class HPMA115S0{

        public:
            HPMA115S0( Stream &_ser );
            ~HPMA115S0( void);

            bool begin ( void );

            bool ReadMesurement( uint16_t* pm2_5, uint16_t* pm10);
            bool StartMesurment( void );
            bool StopMesurment( void );
            bool SetCustomAdjustmentCoefficent( uint8_t coefficent );
            bool GetCustomAdjustmentCoefficent( uint8_t* coefficent);
            bool StopAutoSend( void );
            bool EnableAutoSend( void );


        private:
            
            typedef enum {
                READ=0,
                START,
                STOP,
                SETADJ,
                READADJ,
                STOPAUTOSEND,
                STARTAUTOSEND,
                COMMAND_CNT
               
            } HPMA115S0_COMMAND_t;

            uint8_t HPMACOMMANDS[ COMMAND_CNT ] ={
                /*
                [READ]=0x04,
                [START]=0x01,
                [STOP]=0x02,
                [SETADJ]=0x08,
                [READADJ]=0x10,
                [STOPAUTOSEND]=0x20,
                [STARTAUTOSEND]=0x40
                */ //This will cause a compiler fault / internal error
                [0]=0x04,
                [1]=0x01,
                [2]=0x02,
                [3]=0x08,
                [4]=0x10,
                [5]=0x20,
                [6]=0x40
            };

            Stream &ser;

            void SendCommand( HPMA115S0_COMMAND_t command);
            void SendCommandParameter(HPMA115S0_COMMAND_t command, uint8_t parameter);
            bool ProcessACKNACK( void );



    };


 #endif