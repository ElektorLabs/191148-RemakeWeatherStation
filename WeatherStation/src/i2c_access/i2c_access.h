

 /**************************************************************************************************
 *    Function      : i2c_mutex_setup
 *    Description   : Mutex to access I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_mutex_setup( void );

 /**************************************************************************************************
 *    Function      : i2c_lock_bus
 *    Description   : This will lock access to the I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_lock_bus( void );

 /**************************************************************************************************
 *    Function      : i2c_unlock_bus
 *    Description   : This will lock access to the I2C Bus
 *    Input         : bool
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void i2c_unlock_bus( void );


