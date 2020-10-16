#include "../TimeCore/timecore.h"
#include "../ValueMapping/ValueMapping.h"
/**************************************************************************************************
 *    Function      : setup_sdcard
 *    Description   : This will prepare the sd-card interface
 *    Input         : int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin,  int8_t sd_cs_pin
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void setup_sdcard( int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin,  int8_t sd_cs_pin );

/**************************************************************************************************
 *    Function      : sdcard_mount
 *    Description   : this will try to mount the sd card
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void sdcard_mount( void );


/**************************************************************************************************
 *    Function      : sdcard_umount
 *    Description   : this will try to unmount the sd card
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void sdcard_umount( void );

/**************************************************************************************************
 *    Function      : sdcard_getmounted
 *    Description   : returns if the card is mounted
 *    Input         : void
 *    Output        : bool
 *    Remarks       : none
 **************************************************************************************************/
bool sdcard_getmounted( void );

/**************************************************************************************************
 *    Function      : sdcard_log_enable
 *    Description   : Enables or disables value logging to sd-card
 *    Input         : bool
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void sdcard_log_enable( bool ena);

/**************************************************************************************************
 *    Function      : sdcard_log_int
 *    Description   : Sets the log interval for the sdcard
 *    Input         : uint16_t
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void sdcard_log_int( uint16_t interval);

/**************************************************************************************************
 *    Function      : sdcard_log_getenable
 *    Description   : Checks if the logging is enabled
 *    Input         : uint16_t
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
bool sdcard_log_getenable( void );

/**************************************************************************************************
 *    Function      : sdcard_log_getinterval
 *    Description   : gets the log intervall
 *    Input         : void
 *    Output        : uint16_t
 *    Remarks       : none
 **************************************************************************************************/
uint16_t sdcard_log_getinterval( void );

/**************************************************************************************************
 *    Function      : sdcard_GetCapacity
 *    Description   : Gets the disk capacity in MB
 *    Input         : void
 *    Output        : uint32_t
 *    Remarks       : none
 **************************************************************************************************/
uint32_t sdcard_GetCapacity( void ); //Capacity in MB

/**************************************************************************************************
 *    Function      : sdcard_GetFreeSpace
 *    Description   : Gets the free disk space in MB
 *    Input         : void
 *    Output        : uint32_t
 *    Remarks       : none
 **************************************************************************************************/
uint32_t sdcard_GetFreeSpace( void  ); //FreeSpace in MB


/**************************************************************************************************
 *    Function      : SDCardRegisterTimecore
 *    Description   : Will register a timesource for logging
 *    Input         : Timecore*
 *    Output        : void 
 *    Remarks       : none
 **************************************************************************************************/
void SDCardRegisterTimecore( Timecore* TC);

/**************************************************************************************************
 *    Function      : SDCardRegisterTimecore
 *    Description   : Will register access to the vlauemapper
 *    Input         : Timecore*
 *    Output        : void 
 *    Remarks       : none
 **************************************************************************************************/
void SDCardRegisterMappingAccess(VALUEMAPPING* Mapping);
