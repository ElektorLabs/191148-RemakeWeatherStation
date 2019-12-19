#include <Arduino.h>
#include <avr/pgmspace.h>
#include <lmic.h>
#include "serialcli.h"

uint8_t msgbuffer[64]={0,};
uint8_t msgbufferidx=0;
bool first_con = false;

/* A command can have three parts at the moment 
- set / get / help / clear  
- setpoint / temperatur / error 
- integer value ( 0 - 999 )
*/


void WriteTXPower(uint8_t power);
uint8_t GetTXPower( void );

typedef enum {
cmd_data_error=0,
cmd_data_get,
cmd_data_set,
cmd_data_clear,
cmd_data_help
} cmd_accesstype_t;


typedef enum {
    cmd_none = 0,
    cmd_key,
    cmd_power,
    cmd_state,
    cmd_framecounter,
    cmd_error,
    cmd_sf,
    cmd_cnt
} cmd_command_t;

typedef enum{
    param_none=0,
    param_u8,
    param_i8,
    praam_u16,
    param_i16,
    param_u32,
    param_i32,
    param_string,
    param_keyword
} cmd_param_t;

typedef enum {
  KW_PARSE_ERROR=0,
  KW_ON,
  KW_OFF,
  KW_SF7B,
  KW_SF7,
  KW_SF8,
  KW_SF9,
  KW_SF10,
  //SF11 - SF12 is not supported here!
  KW_CNT
}keyword_t;




typedef union {
        uint8_t u8;
        int8_t i8;
        uint16_t u16;
        int16_t i16;
        uint32_t u32;
        int32_t i32;
        float fl;
        char* ch;
        uint32_t kw; //keyword index for parameter like on or off
    }cmd_parameter_t;

typedef struct{
    cmd_accesstype_t cmd_dir;
    cmd_command_t command;
    cmd_parameter_t parameter;
    uint8_t param_len;
} serial_command_t;

cmd_param_t ParamLUT[ cmd_cnt ]={
 [cmd_none] = param_none,
 [cmd_key] = param_string ,
 [cmd_power] = param_u8 ,
 [cmd_state]= param_keyword,
 [cmd_framecounter] = param_u32,
 [cmd_error]= param_none,
 [cmd_sf] = param_keyword
 
};



/* This is a bit hacky and needs to be solved by functionpointer */
extern void WriteFrameCounter( uint32_t) ;
extern uint32_t ReadFrameCounter( void );
extern bool ReadKey(char* key, uint8_t* maxLen);
extern void WriteKey(char* key, uint8_t keyLen);
extern bool GenerateAESKey( void );
extern bool GetSwitchActiveState( void );
extern void SetSwitchActiveState( bool on );
extern void WriteTXPower(uint8_t power);
extern uint8_t GetTXPower( void );
extern void SetLMICDataRate(u1_t  DataRate);
extern u1_t  GetLMICDataRate();

/**********************************************************************************************************
                                void PrintErrorLocation()        
**********************************************************************************************************
 Function:    void PrintErrorLocation()
 Input:       uint8_t startidx, uint8_t current_index 
 Output:      None 
 Description: Show the location of a parsing error
**********************************************************************************************************/
void PrintErrorLocation(uint8_t startidx, uint8_t current_index  ){
          Serial.print(F(" Syntax error near:\""));
                        for(uint8_t c = startidx;c<current_index;c++){
                            if( msgbuffer[c]=='\n' ) {
                                Serial.print(F("[NEW LINE]"));
                            } else if ( msgbuffer[c]=='\r'){
                            Serial.print(F("[RETURN]"));
                            } else {
                                Serial.write( msgbuffer[c]);
                            }
                        }
                    Serial.println("\"");
}


/**********************************************************************************************************
                                void SerialConsolePrintWelcome()        
**********************************************************************************************************
 Function:    void SerialConsolePrintWelcome()
 Input:       None
 Output:      None 
 Description: Show the welcome banner
**********************************************************************************************************/
void SerialConsolePrintWelcome( void ){
  if(Serial){
    Serial.println(F(" LoRa Switch Transmitter"));
    Serial.println(F(" for HW 180516-1 (STM32F072)"));
    Serial.println(F("------------------------------------------------------------------"));
    Serial.print(F(">"));
  }
}


/**********************************************************************************************************
                                void SerialCommandShowHelp()        
**********************************************************************************************************
 Function:    void SerialCommandShowHelp()
 Input:       None
 Output:      None 
 Description: Show the help
**********************************************************************************************************/
void SerialCommandShowHelp( void ){
    Serial.println(F("------------------------------------------------------------------"));
    Serial.println(F("LoRa Switch ( Transmitter )"));
    Serial.println(F("Supported commands"));
    Serial.println(F("___WIFI Radio Settings___"));
    Serial.println(F("set wifi_key <passphrase> -> This will set the wifi passphrase used ( 8 to 32 chars )"));
    Serial.println(F("get wifi_key              -> This will get the wifi passphrase used"));
    Serial.println(F("get wifi                  -> This will get the wifi enable status"));
    Serial.println(F("set wifi <en>             -> This will set the wifi enable <on/off>"));
    Serial.println(F("force wifi_ap             -> This will activate the AP-Mode"));
    Serial.println(F("clear wifi                -> This will wipe all WiFi settings to default"));
    Serial.println(F("___Sensor Info___"));
    Serial.println(F("___Misc___"));
    Serial.println(F("help                      -> This will show this help"));
    Serial.println(F("-------------------------------------------------------------------"));
}



/**********************************************************************************************************
                                void SerialConsoleParseInput()        
**********************************************************************************************************
 Function:    void SerialConsoleParseInput()
 Input:       None
 Output:      serial_command_t 
 Description: Command parser, returns a struct with the parsed result
**********************************************************************************************************/
serial_command_t SerialConsoleParseInput( void ){
    serial_command_t command;
    command.cmd_dir = cmd_data_error;
    command.command = cmd_none;
    command.parameter.u32=0;
    if(msgbufferidx>sizeof(msgbuffer)){
        Serial.println( F("Buffer overflow") );
        for(uint8_t i=0;i<sizeof(msgbuffer);i++){
           msgbuffer[i]=0;
        }
        msgbufferidx=0;
    } else {
        uint8_t startidx = 0;
        uint8_t endidx = 0;
        for(uint8_t i=0;i<msgbufferidx;i++){
            if( ( msgbuffer[i]==' ') || (msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                /* The first token has ended, we accept only 'set', 'get' and 'help' */
                if(strncmp((const char*)&msgbuffer[startidx] ,("set"), i)==0){
                    command.cmd_dir = cmd_data_set;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("get"), i)==0){
                    command.cmd_dir = cmd_data_get;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("help"), i)==0){
                    command.cmd_dir = cmd_data_help;
                } else if (strncmp((const char*)&msgbuffer[startidx] ,("clear"), i)==0){
                     command.cmd_dir = cmd_data_clear;
                } else {
                    command.cmd_dir = cmd_data_error;
                    PrintErrorLocation(startidx,i);
                }
                endidx=i;
                break;

                
                
            } else {
              endidx=i;
            }
        } 

        startidx = endidx+1;
        if( (startidx<msgbufferidx) && (msgbuffer[endidx]!='\n' ) &&  (msgbuffer[endidx]!='\r') )  {
          
        } else {
            /* Parsing is done */
            return command;
        }

        for(uint8_t i=startidx;i<msgbufferidx;i++){
            if( ( msgbuffer[i]==' ') || (msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                /* The second token has ended, we accept only 'setpoint' , 'temperature' and 'error' */
                uint8_t tokenlenght = i - startidx;
                if(strncmp((const char*)&msgbuffer[startidx] ,("key"), (tokenlenght) )==0){
                    command.command=cmd_key;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("txpower"), (tokenlenght) )==0){
                    command.command=cmd_power;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("state"), (tokenlenght) )==0){
                    command.command=cmd_state;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("framecounter"), (tokenlenght) )==0){
                    command.command=cmd_framecounter;
                } else if(strncmp((const char*)&msgbuffer[startidx] ,("datarate"), (tokenlenght) )==0){
                    command.command=cmd_sf;
                } else {
                    command.command = cmd_none;
                        PrintErrorLocation(startidx,i);
                }
                endidx=i;
                break;
                
            } else {
                endidx=i;
            }
        }

         startidx = endidx+1;
        if( (startidx<msgbufferidx) && (msgbuffer[endidx]!='\n' ) &&  (msgbuffer[endidx]!='\r') )  {
          
        } else {
            /* Parsing is done */
            return command;
        }
        /*  
            last is to parse the parameter here 
            we currently strings and integer ( u32 ) but need to decide what we must do
        */
        
        if( (ParamLUT[command.command]!=param_string) && (ParamLUT[command.command]!=param_keyword) ){
          uint32_t value = 0;
          command.param_len=0;
          for(uint8_t i=startidx;i<msgbufferidx;i++){
              
              if( (msgbuffer[i]>='0') && (msgbuffer[i]<='9') ){
                  value = value * 10;
                  value = value + ( msgbuffer[i] -'0');
              } else {
                  if( ( msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                      command.param_len = i;
                       command.parameter.u32 = value;
                      return command;
                  } else if( (msgbuffer[i]<31) || ( msgbuffer[i]>=127)){
                       /* Parsing error */
                  command.cmd_dir = cmd_data_error;
                  command.command = cmd_none;
                  command.parameter.u32=0;
                  Serial.print(" Syntax error near:\"");
                  PrintErrorLocation(startidx,i);
                    
                  } else {
                    //Next char
                  }
              }                
          }
        } else if( (ParamLUT[command.command]==param_string) || (ParamLUT[command.command]==param_keyword) ){
          //This will be "simple" we look if we end with \n or \r and use the rest as
          //string for command processing
          command.parameter.ch=(char*)&msgbuffer[startidx]; //String start in Buffer
          uint8_t StringLen = 0;
          for(uint8_t i=startidx;i<msgbufferidx;i++){
             
                if( ( msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                      //If we expect a param_sting we are done
                    if(ParamLUT[command.command]==param_string){
                        command.param_len=StringLen;
                        return command;
                    } else {
                          //We need to ckeck for supported keywords
                        if(strncmp((const char*)&msgbuffer[startidx] ,("ON"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_ON );
                                 return command;
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("OFF"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_OFF );
                                 return command;
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("OFF"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_OFF );
                                 return command;
                        /*} else if(strncmp((const char*)&msgbuffer[startidx] ,("SF7B"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_SF7B );
                                 return command;*/
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("SF7"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_SF7 );
                                 return command;
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("SF8"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_SF8 );
                                 return command;
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("SF9"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_SF9 );
                                 return command;
                        } else if(strncmp((const char*)&msgbuffer[startidx] ,("SF10"), (StringLen) )==0){
                                command.parameter.u32 =(uint32_t)( KW_SF10);
                                return command;
                        } else {
                            //We need to signal an parsing error
                            Serial.print("'");
                            for( uint8_t i=0;i<StringLen;i++){
                                Serial.print((char)(msgbuffer[startidx+i]) );
                            }

                            
                            Serial.println("' not recognized as keyword" );
                            command.parameter.u32 =(uint32_t)( KW_PARSE_ERROR );
                            return command;
                        }
                    }
                }
                StringLen++;
            }
            /* Parsing error */
            command.cmd_dir = cmd_data_error;
            command.command = cmd_none;
            command.parameter.u32=0;
            Serial.print("Syntax error near:\"");
            PrintErrorLocation(startidx,msgbufferidx);
          
          
        } else {
          
        }
        
        //We need to grab the end of the input and build a kind of param out of it
        //Also we may need to do the error handling here 

        return command;

    }

}


/**********************************************************************************************************
                                void SerialConsoleProcess()        
**********************************************************************************************************
 Function:    void SerialConsoleProcess()
 Input:       None
 Output:      None 
 Description: Processes incomming serial data
**********************************************************************************************************/
bool SerialConsoleProcess( void ){
    bool InputProcessed = false;
    if(Serial){
        if(first_con == false){
            SerialConsolePrintWelcome();
            first_con=true;
        }
        /* Grab data from interface */
        while ( ( Serial ) && (Serial.available() > 0)) {
                InputProcessed = true;
                // read the incoming byte:
                uint8_t data = Serial.read();
                if( (data!='\r' ) && (data!='\n')){
                    Serial.write(data);
                }
            
                if(msgbufferidx<sizeof(msgbuffer) ){
                    msgbuffer[msgbufferidx]=data;
                    msgbufferidx++;
                } else {
                    /* we have an overflow */
                    msgbufferidx=sizeof(msgbuffer);
                }

                if( (data=='\r' ) || (data=='\n')){   /* Time to parse data */
                    Serial.print(F("\n\r"));
                    if( msgbufferidx == 0){
                       /* No Data in the buffer */ 
                    } else {
                        /* we need to parse the buffer */
                        serial_command_t command = SerialConsoleParseInput();

                        switch( command.cmd_dir ){
                            case cmd_data_help:{
                                SerialCommandShowHelp();
                            } break;

                            case cmd_data_set:{

                                switch ( command.command ){
                                    
                                    case cmd_key:{
                                     if( (command.param_len>7) && (command.param_len<33) ){
                                            //command_if_update_setpoint(command.parameter.u32);
                                            Serial.print("Key leght:");
                                            Serial.print(command.param_len);
                                            Serial.print(" chars, ");
                                            Serial.print(F("save KEY '"));
                                            for(uint8_t i=0;i<command.param_len;i++){
                                                Serial.print(command.parameter.ch[i]);
                                            }
                                            Serial.print("' to EEPROM");
                                            WriteKey(command.parameter.ch,command.param_len);
                                            if ( true == GenerateAESKey() ){
                                                Serial.println("New Key applied");
                                            } else {
                                               Serial.println("EEPROM corruption problem"); 
                                            }
                                            Serial.print(F("\n\rOK\n\r"));
                                            //We need to issue a write to the eeprom

                                      } else {
                                          Serial.println("Key must be 8 to 32 chars long");
                                          Serial.print(F("\n\rOK\n\r")); 
                                      }
                                    } break;

                                    case cmd_power:{
                                         Serial.print(F("Save TX-Power of "));
                                         Serial.print(command.parameter.u32);
                                         Serial.println(" to EEPROM");
                                         WriteTXPower(command.parameter.u32);
                                         Serial.print(F("\n\rOK\n\r")); 
                                    } break;


                                    case cmd_state:{
                                        keyword_t keyword = (keyword_t)(command.parameter.u32);
                                        if(keyword == KW_ON){
                                            SetSwitchActiveState( true );
                                            Serial.println("Set remote to ON ");
                                        } else if( keyword == KW_OFF){
                                            SetSwitchActiveState( false );
                                            Serial.println("Set remote to OFF ");
                                        } else if(keyword == KW_PARSE_ERROR){
                                           //We fail here silently as the parser shoudl alredy have warned
                                        } else {
                                            Serial.println("Keyword not implemented");
                                        }
                                    } break;

                                    case cmd_framecounter:{
                                        Serial.print("Set framcounter to ");
                                        Serial.print(command.parameter.u32);
                                        WriteFrameCounter(command.parameter.u32);
                                        Serial.println(" ");
                                    } break;

                                     case cmd_sf:{
                                      Serial.print("Set LoRa Datarate to:");
                                      keyword_t keyword = (keyword_t)(command.parameter.u32);
                                      switch(keyword){
                                        case KW_SF7B:{
                                          SetLMICDataRate(DR_SF7B);
                                          Serial.println(" SF7B");
                                        } break;

                                        case KW_SF7:{
                                          SetLMICDataRate(DR_SF7);
                                          Serial.println(" SF7");
                                        } break;

                                        case KW_SF8:{
                                          SetLMICDataRate(DR_SF8);
                                          Serial.println(" SF8");
                                        }break;

                                        case KW_SF9:{
                                          SetLMICDataRate(DR_SF9);
                                          Serial.println(" SF9");
                                        } break;

                                        case KW_SF10:{
                                          SetLMICDataRate(DR_SF10);
                                          Serial.println(" SF10");
                                        }break;

                                        default:{
                                          Serial.println("unknown");
                                        } break;
                                      }
                                      Serial.print("Current LoRa Datarate is:");
                                      switch( GetLMICDataRate() ){
                                         case DR_SF7B:{
                                            Serial.println(" SF7B");
                                         }break;

                                         case DR_SF7:{
                                            Serial.println(" SF7");
                                         }break;

                                         case DR_SF8:{
                                            Serial.println(" SF8");
                                         }break;

                                         case DR_SF9:{
                                            Serial.println(" SF9");
                                         }break;

                                         case DR_SF10:{
                                            Serial.println(" SF10");
                                         }break;

                                         default:{
                                          Serial.println(" unknown");
                                         } break;
                                      }
                                      
                                      
                                    } break;

                                    default:{
                                        Serial.println("Not supported");
                                    }
                                }
                       
                            }break;

                            case cmd_data_get:{

                                  switch ( command.command ){
                                    
                                    case cmd_key:{
                                    /* We read the plain text key form the eeprom */
                                    char key_buffer[32];
                                    uint8_t maxLen = sizeof(key_buffer);
                                    if(false == ReadKey(key_buffer, &maxLen) ){
                                        Serial.println("No valid KEY in EEPROM");
                                    } else {
                                        Serial.print("Key stored: '");
                                        for(uint8_t i=0;i<maxLen;i++){
                                            Serial.print((char)(key_buffer[i]));
                                        }
                                        Serial.println("'");
                                    }
                                      Serial.print(F("\n\rOK\n\r")); 
                                    } break;

                                    case cmd_power:{
                                    uint8_t pw = 0;
                                    pw = GetTXPower();
                                    Serial.print(F("Current Power:"));
                                    Serial.println(pw); 
                                    Serial.print(F("\n\rOK\n\r")); 
                                    } break;

                                    case cmd_state:{
                                    Serial.print(F("The remote relay is switched to"));
                                    bool state = GetSwitchActiveState( );
                                    if( false == state){
                                        Serial.println(" OFF");
                                    } else {
                                        Serial.println(" ON");
                                    }
                                    
                                    Serial.print(F("\n\rOK\n\r")); 

                                    } break;

                                     case cmd_framecounter:{
                                        Serial.print("Current framcounter is ");
                                        Serial.print(ReadFrameCounter());
                                        Serial.println(" ");
                                    } break;

                                     case cmd_sf:{

                                      Serial.print("Current LoRa Datarate is:");
                                      switch( GetLMICDataRate() ){
                                         case DR_SF7B:{
                                            Serial.println(" SF7B");
                                         }break;

                                         case DR_SF7:{
                                            Serial.println(" SF7");
                                         }break;

                                         case DR_SF8:{
                                            Serial.println(" SF8");
                                         }break;

                                         case DR_SF9:{
                                            Serial.println(" SF9");
                                         }break;

                                         case DR_SF10:{
                                            Serial.println(" SF10");
                                         }break;

                                         default:{
                                          Serial.println(" unknown");
                                         } break;
                                      }
                                      
                                    } break;

                                    default:{
                                        Serial.println("Not supported");
                                    }
                                }

                            } break;

                            case cmd_data_clear:{

                                switch ( command.command ){
                                    
                                    case cmd_framecounter:{
                                        //We reset the cpunter to zero
                                        WriteFrameCounter( 0 );
                                    } break;

                                    default:{
                                        Serial.println("Not supported");
                                    }
                                }
                          
                            }break;

                            default:{

                            } break;
                        }

                        /* we clear the buffer */

                        for (uint8_t i=0;i<sizeof(msgbuffer);i++){
                            msgbuffer[i]=0;
                        }
                        msgbufferidx = 0;
                        Serial.print(F("\r\n>"));

                    }
                }

        }     
    } else {
        first_con=false;
    }


    return InputProcessed;


}
