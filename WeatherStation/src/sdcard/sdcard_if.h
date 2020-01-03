#include "../TimeCore/TimeCore.h"
#include "../ValueMapping/Valuemapping.h"
void setup_sdcard( int8_t sd_sck_pin , int8_t sd_miso_pin, int8_t sd_mosi_pin,  int8_t sd_cs_pin );
void sdcard_mound( void );
void sdcard_umound( void );
void register_timecore( Timecore* TC);
void SDCardRegisterMappingAccess(VALUEMAPPING* Mapping);