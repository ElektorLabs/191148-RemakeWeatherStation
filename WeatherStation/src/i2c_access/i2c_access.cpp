#include <Arduino.h>
#include <Wire.h>
static SemaphoreHandle_t xI2CBus_Semaphore = nullptr;

 /**************************************************************************************************
 *    Function      : i2c_mutex_setup
 *    Description   : Mutex to access I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_mutex_setup( void ){
    if( nullptr == xI2CBus_Semaphore  ){
        xI2CBus_Semaphore = xSemaphoreCreateMutex();
    } else {
        abort();
    }

}

 /**************************************************************************************************
 *    Function      : i2c_mutex_setup
 *    Description   : Mutex to access I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_lock_bus( void ){
   
    if( nullptr == xI2CBus_Semaphore  ){
        xI2CBus_Semaphore = xSemaphoreCreateMutex();
    }
    
    if( xSemaphoreTake( xI2CBus_Semaphore, portMAX_DELAY ) == pdTRUE ){

    } else {
        //Strange ...
    }
    Wire.flush(); //This will prevent misshaps on the i2c bus
}

 /**************************************************************************************************
 *    Function      : i2c_unlock_bus
 *    Description   : This will lock access to the I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_unlock_bus( void ){
    if( nullptr == xI2CBus_Semaphore  ){
        //This is bad
        abort();
    } else {
        xSemaphoreGive( xI2CBus_Semaphore );
    }
    Wire.flush(); //This will prevent misshaps on the i2c bus
}