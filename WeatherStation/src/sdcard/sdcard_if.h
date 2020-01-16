#include "../TimeCore/TimeCore.h"
#include "../ValueMapping/Valuemapping.h"
void setup_sdcard( int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin,  int8_t sd_cs_pin );
void sdcard_mount( void );
void sdcard_umount( void );
bool sdcard_getmounted( void );

void sdcard_log_enable( bool ena);
void sdcard_log_int( uint16_t interval);
bool sdcard_log_getenable( void );
uint16_t sdcard_log_getinterval( void );

uint32_t sdcard_GetCapacity( void ); //Capacity in MB
uint32_t sdcard_GetFreeSpace( void  ); //FreeSpace in MB

void SDCardRegisterTimecore( Timecore* TC);
void SDCardRegisterMappingAccess(VALUEMAPPING* Mapping);