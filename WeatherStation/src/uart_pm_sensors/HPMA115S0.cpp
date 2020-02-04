#include "HPMA115S0.h"

 /**************************************************************************************************
 *    Function      : Constructor
 *    Description   : none
 *    Input         : Stream &_ser
 *    Output        : none
 *    Remarks       : Needs a Stream as input
 **************************************************************************************************/
HPMA115S0::HPMA115S0(Stream &_ser):ser(_ser){

}

 /**************************************************************************************************
 *    Function      : Destructor
 *    Description   : none
 *    Input         : Stream &_ser
 *    Output        : none
 *    Remarks       : Needs a Stream as input
 **************************************************************************************************/
HPMA115S0::~HPMA115S0( void){

}

 /**************************************************************************************************
 *    Function      : begin
 *    Description   : none
 *    Input         : void
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
 bool HPMA115S0::begin ( void ){
     
 }


 /**************************************************************************************************
 *    Function      : ReadMesurement
 *    Description   : This will read the PM2.5 and PM10 value
 *    Input         : uint16_t* pm2_5 , uint16_t* pm10
 *    Output        : void
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::ReadMesurement(uint16_t* pm2_5 , uint16_t* pm10 ){
    //Flush everthing that is in the receive buffer
    bool readok = false;
    bool data_prcessed=false;
    ser.flush();
    uint32_t starttime = millis();
    uint8_t DataIn[8] = { 0x00, };
    uint8_t DataRead =0;
     uint8_t CalcCS = 0;
    
    SendCommand(HPMA115S0_COMMAND_t::READ);
    /*
        We wait for 500ms for complete resonse
        this can be 0x9696 for NAK or here 
        0x40 , 0x05 , 0x04 , 0x?? , 0x?? , 0x?? , 0x?? , CS 
    */
    while( (false == data_prcessed) && ( ( millis() - starttime ) < 500 ) ){
        if(ser.available()){
            int inByte = Serial1.read();
            if(inByte>=0){
                DataIn[DataRead]=inByte;
                DataRead++;
            }
        }
        if( (DataRead==2) && ( DataIn[0] == 0x96) && ( DataIn[1] == 0x96 ) ){
            //NAK received
            *pm2_5= 0;
            *pm10 = 0;
            readok=false;
            data_prcessed=true;
        }

        if(DataRead == 0x08 ){
            //We can process the response first the checksum:
         
           
            for(uint8_t i=0;i<(sizeof(DataIn)-1);i++) {
                CalcCS= CalcCS+DataIn[i];
            }
            CalcCS = CalcCS -1;
            CalcCS = ~CalcCS;
            if( (0x40 == DataIn[0]) && (0x05 == DataIn[1]) && (0x04 == DataIn[2]) && (CalcCS == DataIn[7]) ){
                //Data is okay next is to process the data
                *pm2_5 = DataIn[3]*256 + DataIn[4];
                *pm10 = DataIn[5]*256 + DataIn[6];
                readok=true;
                data_prcessed= true;
            } else {
                *pm2_5= 0;
                *pm10 = 0;
                readok=false;
                data_prcessed=true;
            }


        } 
    }
    return readok;
}


 /**************************************************************************************************
 *    Function      : StartMesurment
 *    Description   : This will send a start messurment command to the sensor
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::StartMesurment( ){


   
    SendCommand(HPMA115S0_COMMAND_t::START);
    /*
        We wait for 500ms for complete resonse
        this can be 0x9696 for NAK or 0xA5A5 for ACK 
     
    */
    return ProcessACKNACK();

}


 /**************************************************************************************************
 *    Function      : StopMesurment
 *    Description   : This will send a stop messurment command to the sensor
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::StopMesurment( ){

   
    SendCommand(HPMA115S0_COMMAND_t::STOP);
    /*
        We wait for 500ms for complete resonse
        this can be 0x9696 for NAK or 0xA5A5 for ACK 
     
    */
    return ProcessACKNACK();


}

 /**************************************************************************************************
 *    Function      : SetCustomAdjustmentCoefficent
 *    Description   : This will set a custom coefficent ( see manual for more info )
 *    Input         : uint8_t coefficent
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::SetCustomAdjustmentCoefficent( uint8_t coefficent ){
    if(coefficent<30){
        coefficent=30;
    }
    SendCommandParameter(HPMA115S0_COMMAND_t::SETADJ, 100);
    return ProcessACKNACK();
}


 /**************************************************************************************************
 *    Function      : GetCustomAdjustmentCoefficent
 *    Description   : not supported
 *    Input         : uint8_t* coefficent
 *    Output        : bool
 *    Remarks       : Not supported
 **************************************************************************************************/
bool HPMA115S0::GetCustomAdjustmentCoefficent( uint8_t* coefficent){
    return false;
}


 /**************************************************************************************************
 *    Function      : StopAutoSend
 *    Description   : Will disable autosend
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::StopAutoSend( ){

       
    SendCommand(HPMA115S0_COMMAND_t::STOPAUTOSEND);
    /*
        We wait for 500ms for complete resonse
        this can be 0x9696 for NAK or 0xA5A5 for ACK 
     
    */
    return ProcessACKNACK();

}

 /**************************************************************************************************
 *    Function      : EnableAutoSend
 *    Description   : Will enable autosend
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::EnableAutoSend( ){

    SendCommand(HPMA115S0_COMMAND_t::STARTAUTOSEND);
    /*
        We wait for 500ms for complete resonse
        this can be 0x9696 for NAK or 0xA5A5 for ACK 
     
    */
    return ProcessACKNACK();

}

 /**************************************************************************************************
 *    Function      : SendCommand
 *    Description   : Will send a command to the sensor
 *    Input         : HPMA115S0_COMMAND_t
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
 void HPMA115S0::SendCommand( HPMA115S0_COMMAND_t command){

    //uint8_t Data[4]={104,1,0x00,0x00}; 
    uint8_t Data[4]={0x68,0x01,0x00,0x00};
    uint8_t CheckSum = 0x00;
    Data[2] = HPMACOMMANDS[command];
    for(uint8_t i=0; i<( sizeof(Data) -1 ) ;i++){
        CheckSum += Data[i];
    }
    CheckSum = CheckSum -1;
    CheckSum = ~CheckSum;
    Data[3] = CheckSum;
    //Checksum is calculated
    for(uint8_t i=0; i<( sizeof(Data) ) ;i++){
     ser.write(Data[i]);
    }
 }

 /**************************************************************************************************
 *    Function      : SendCommandParameter
 *    Description   : Will send a command with parameter to the sensor
 *    Input         : HPMA115S0_COMMAND_t command, uint8_t parameter
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
void HPMA115S0::SendCommandParameter(HPMA115S0_COMMAND_t command, uint8_t parameter){
           //uint8_t Data[4]={104,1,0x00,0x00}; 
    uint8_t Data[5]={0x68,0x01,0x00,0x00,0x00};
    uint8_t CheckSum = 0x00;
    Data[2] = HPMACOMMANDS[command];
    Data[3] = parameter;
    for(uint8_t i=0; i<( sizeof(Data) -1 ) ;i++){
        CheckSum += Data[i];
    }
    CheckSum = CheckSum -1;
    CheckSum = ~CheckSum;
    Data[4] = CheckSum;
    //Checksum is calculated
    for(uint8_t i=0; i<( sizeof(Data) ) ;i++){
        ser.write(Data[i]);
    }
   }


 /**************************************************************************************************
 *    Function      : ProcessACKNACK
 *    Description   : Waits for a NAK / ACK from the sensor or a timout
 *    Input         : void
 *    Output        : bool
 *    Remarks       : None
 **************************************************************************************************/
bool HPMA115S0::ProcessACKNACK( void ){

    bool readok = false;
    bool data_prcessed=false;
    ser.flush();
    uint32_t starttime = millis();
    uint8_t DataIn[2] = { 0x00, };
    uint8_t DataRead =0;
    while( (false == data_prcessed) && ( 125 > ( millis() - starttime ) ) ){
        if(ser.available()>0){
            int inByte = ser.read();
            if(inByte>=0){
                DataIn[DataRead]=inByte;
                DataRead++;
            }
        }
        if( (DataRead==2) && ( DataIn[0] == 0x96) && ( DataIn[1] == 0x96 ) ){
            //NAK received
            readok=false;
            data_prcessed=true;
        }

        if( (DataRead==2) && ( DataIn[0] == 0xA5) && ( DataIn[1] == 0xA5 ) ){
            //ACK received
            readok=true;
            data_prcessed=true;
        }
    }

    return readok;

}