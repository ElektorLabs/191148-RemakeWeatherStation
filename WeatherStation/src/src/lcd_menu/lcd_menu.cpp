#include "../i2c_access/i2c_access.h"
#include "../isr_fnc/timefunctions.h"

//Menu Entrys are:

//WiFi Status and IP Address (v4 only)

// DEFAULT DISPLAY
//-----------------
//|WiFI: xx dBm   |
//|192.168.000.000|
//-----------------

//-----------------
//|WiFi: Sleep    |
//|No IP          |
//-----------------

//-----------------
//|WiFi: Not Con. |
//|               |
//-----------------

//-----------------
//|WiFi: AP       |
//|192.168.000.000|            
//-----------------

//-----------------
//|WiFi: WPS      |
//|WPS active     |
//-----------------

//General if the button is pressed and the display is off the backlight will 
//light up and the display is active for 30 seconds
//If button is pressed for more than 20 seconds while the display is active we will enter WPS mode !


#include <Arduino.h>
#include "../../wifi_net.h"
#include "../sdcard/sdcard_if.h"
#include "lcd_menu.h"

static LiquidCrystal_I2C lcd(0x27,16,2);
void UserBtnISR( void );
void DefaultDisplay( void );
void EnableLCD( void );
void DisableLCD( void );

static int16_t userbtn=-1;
static int16_t userbtn2=-1;
static volatile bool pressed = false;
static volatile uint32_t  press_start = 0;
static volatile uint32_t  press_end = 0;
static volatile uint32_t press_duration = 0;
static volatile uint8_t displaymode = 0;
SemaphoreHandle_t xBtnSemaphore;
static volatile uint32_t BtnPressCnt=0;

static bool DisplayAttached = false;


void LCDisplayTask( void* param);
void NoneDisplayTask( void* param);

void LCDMenuShow( uint8_t Menu ); 
#define DEBUG_SERIAL

/**************************************************************************************************
 *    Function      : LCDMenu
 *    Description   : Will setup the functions for the LCD menu
 *    Input         : int16_t btn, int16_t btn2
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void LCDMenu( int16_t btn, int16_t btn2 , uint8_t menuitem ){
    displaymode = menuitem;
    if(btn < 0){
      //No Button defined we will disbale the display at all, as we expect the boot button to be present
      #ifdef DEBUG_SERIAL
        Serial.println("No USR Btn defined, disable Display");
      #endif
      return;
    }
    userbtn = btn;
    userbtn2 = btn2;
    xBtnSemaphore = xSemaphoreCreateBinary();
    attachInterrupt( digitalPinToInterrupt(userbtn), UserBtnISR , CHANGE );

    i2c_lock_bus();
        Wire.beginTransmission( 0x27 );
        if(0 ==  Wire.endTransmission() ) {
          #ifdef DEBUG_SERIAL
            Serial.println("PCF8754A found @ 0x27");
          #endif
          DisplayAttached = true;
        } else {
          #ifdef DEBUG_SERIAL
            Serial.println("No PCF8754A found");
          #endif
          DisplayAttached = false;
        }
    i2c_unlock_bus();

  if(true == DisplayAttached){
  
        xTaskCreate(
        LCDisplayTask,
        "LCD Task",
        20000,
        NULL,
        1,
        NULL);

  } else {

        xTaskCreate(
        NoneDisplayTask,
        "LCD Task",
        20000,
        NULL,
        1,
        NULL);


  }


}


uint32_t ReadButtonPressCnt( void ){
  uint32_t value = BtnPressCnt;
  Serial.printf("BTN CNT: %u\n\r", value);
  return value;
}

void ResetButtonPressCnt( void ){
   BtnPressCnt=0;
}

/**************************************************************************************************
 *    Function      : DisableLCD
 *    Description   : This will power down the lcd
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void DisableLCD( void ){
    if(false == DisplayAttached){
      return;
    }
    //This will clear the lcd and disable the backligt if possible
    i2c_lock_bus();
    lcd.init();                      // initialize the lcd 
    lcd.noDisplay();
    lcd.noBacklight();
    i2c_unlock_bus();
}

/**************************************************************************************************
 *    Function      : EnableLCD
 *    Description   : This will power up the lcd
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void EnableLCD( void ){
    if(false == DisplayAttached){
      return;
    }
    //This will enable lcd and the backligt if possible
    i2c_lock_bus();
    lcd.init();                      // initialize the lcd 
    lcd.display();
    lcd.backlight();    
    i2c_unlock_bus();
}


/**************************************************************************************************
 *    Function      : DefaultDisplay
 *    Description   : Function to updatet the LCD content
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void DefaultDisplay( void ){
  if(false == DisplayAttached){
    return;
  }
  wifi_connection_info_t ConnectionInfo;
  GetWiFiConnectionInfo( &ConnectionInfo );
  i2c_lock_bus();
  if(true == ConnectionInfo.WPS_Active){
          lcd.setCursor(0,0);
          lcd.print("WiFi:       ");
          lcd.setCursor(0,1);
          lcd.print("WPS active      ");
  } else {

      if(true == ConnectionInfo.IsAP){
        //We are in AP mode....
        lcd.setCursor(0,0);
        lcd.print("WiFi: AP    ");
        lcd.setCursor(0,1);
        lcd.print("192.168.4.1     ");
      } else if(true == ConnectionInfo.IsStation) {
        //We are in STA mode....
        if(ConnectionInfo.Connected==true){
          lcd.setCursor(0,0);
          lcd.print("WiFi:");
          lcd.print(WiFiGetRSSI());
          lcd.print("dBm ");
          lcd.setCursor(0,1);
          lcd.print(ConnectionInfo.IP_Info.SystemIP);
          lcd.print("         ");
        } else {
          lcd.setCursor(0,0);
          lcd.print("WiFi:       ");
          lcd.setCursor(0,1);
          lcd.print("Not connected   ");
          
        }
      } else {
        //WiFi Off
        lcd.setCursor(0,0);
        lcd.print("WiFi: OFF   ");
        lcd.setCursor(0,1);
        lcd.print("                ");
    
      }
  }

  bool mnt = sdcard_getmounted();
  lcd.setCursor(12,0);
  if(false == mnt){
    lcd.print("NOSD");
  } else {
    lcd.print("_SD_");
  }
  
  i2c_unlock_bus();

}

/**************************************************************************************************
 *    Function      : DisplayBootToSTA
 *    Description   : 
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void DisplayBootToSTA(){
          lcd.setCursor(0,0);
          lcd.print("Press Button to ");
          lcd.setCursor(0,1);
          lcd.print("force AP mode   ");
}

/**************************************************************************************************
 *    Function      : DisplayBootToAP
 *    Description   : Displays message that AP mode will be choosen
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void DisplayBootToAP(){
          lcd.setCursor(0,0);
          lcd.print("AP mode is now ");
          lcd.setCursor(0,1);
          lcd.print("forced         ");
}

/**************************************************************************************************
 *    Function      : DisplayDefault
 *    Description   : Defautl display if the station is running
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void DisplayDefault( bool reset ){
  static bool displayoff = false;
  static uint8_t timeout=0;
  wifi_connection_info_t ConnectionInfo;
  if(true == reset ){
    displayoff = false;
    uint8_t timeout=0;
    EnableLCD();
  }
  if ( false == xSemaphoreTake( xBtnSemaphore, 1000 ) ){
      if(timeout < 30 ){
        DefaultDisplay();
        GetWiFiConnectionInfo( &ConnectionInfo );
        if(true == ConnectionInfo.WPS_Active){
            timeout=0;
        } else {
            if(true == pressed){
              timeout=0;
              uint32_t pd = ( millis()-press_start );
              if( pd > 10000 ){
                    //We can enter WPS mode....
                    WPS_Start();
              }
            } else {
               timeout++;
            }
        }


      } else {
         if(false == displayoff ){
            DisableLCD();
            displayoff = true;
         }

      }

        
    } else {
      //We got a button press ( up or down)
      if(false == pressed ){
        if(false == displayoff ){
            //Display is active
            if( ( press_duration > 0 ) && ( press_duration < 7500 ) ){
              //Mount or eject sd card
              bool mnt = sdcard_getmounted();
              if(true == mnt){
                sdcard_umount();
              } else {
                sdcard_mount();
              }
            }
        } else {
          //We need to reactivate the display
          displayoff=false;
          timeout=0;
          press_duration=0;
          EnableLCD();
        }
      } else {
        //We have a button down event
      } 
    }
}

/**************************************************************************************************
 *    Function      : DisplayBootToAPorSTA
 *    Description   : Will display if boot to AP is forced
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void DisplayBootToAPorSTA(bool changed){
  if(true == changed ){
    EnableLCD();
  }

  if ( false == xSemaphoreTake( xBtnSemaphore, 1000 ) ){
    //Nothing has happened
  } else {
    if( ( press_duration > 100 ) && ( press_duration < 7500 ) ){
      BtnPressCnt++;
      #ifdef DEBUG_SERIAL
        Serial.println("BTN++");
      #endif
    } else {
      //Not valid
    }
  }

  if(BtnPressCnt==0){
    DisplayBootToSTA();
  } else {
      DisplayBootToAP();    
  }
}

/**************************************************************************************************
 *    Function      : LCDisplayTask
 *    Description   : Task to handle the display and user input
 *    Input         : void* param
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void LCDisplayTask( void* param){
  static uint8_t lastmode = 0;
  bool changed = false;

  while(1==1){
    if(lastmode != displaymode){
      changed = true;
      ResetButtonPressCnt();
    } else {
      changed = false;
    }
    switch( displaymode ){
      case 0:{
        DisplayDefault(changed);
      } break;

      case 1:{
        DisplayBootToAPorSTA(changed);
      } break;

      default:{
        DisplayDefault( changed );
      }break; 
    }
    lastmode = displaymode;    
  }
}


/**************************************************************************************************
 *    Function      : NoneDisplayDefault
 *    Description   : If no display is connected this is the default "menu"
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void NoneDisplayDefault( bool reset ){
  wifi_connection_info_t ConnectionInfo;
  if(true == reset ){
    press_duration=0;
  }
  if ( false == xSemaphoreTake( xBtnSemaphore, 1000 ) ){
        GetWiFiConnectionInfo( &ConnectionInfo );
        if(true == ConnectionInfo.WPS_Active){
            //We don't process any buttons now
        } else {
            if(true == pressed){
              uint32_t pd = ( millis()-press_start );
              if( pd > 10000 ){
                    //We can enter WPS mode....
                    WPS_Start();
              }
              
            } 
        }


      } else {
         //We got a complete button press, if this is shorter than 5 seconds we will do a unmount of the sd card
         if( ( press_duration > 100 ) && ( press_duration < 7500 ) ){
            //Mount or eject sd card
              bool mnt = sdcard_getmounted();
              if(true == mnt){
                    if(userbtn2 < 0){
                      sdcard_umount();
                    } else {
                    
                      if( 0 == digitalRead( userbtn2 ) ){
                            sdcard_umount();      
                      }
                    }
              } else {
                if(userbtn2 < 0){
                  sdcard_mount();
                } else {
                   if( 1 == digitalRead( userbtn2 ) ){
                          sdcard_mount();    
                   }
                }
              } 
          }
        
      }
}

/**************************************************************************************************
 *    Function      : ButtonPressCounter
 *    Description   : Will count button presses
 *    Input         : none
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void ButtonPressCounter( void ){
  
  if ( false == xSemaphoreTake( xBtnSemaphore, 1000 ) ){
  
  } else {
    if( ( press_duration > 100 ) && ( press_duration < 7500 ) ){
      if( BtnPressCnt < UINT32_MAX){
        #ifdef DEBUG_SERIAL
          Serial.println("BTN++");
        #endif
        BtnPressCnt++;
      }
      
    } else {
      //Not valid
    }
  }

}

/**************************************************************************************************
 *    Function      : NoneDisplayTask
 *    Description   : Task to run if no Display is persent
 *    Input         : void* param
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void NoneDisplayTask( void* param){
  //This needs a second button ( sort of )
  //If we have a second use button we will use it to dismount the sdcard
  //As we don't have any led the user must trust the statuion
  //to mount the card again....

static uint8_t lastmode = 0;
  bool changed = false;

  while(1==1){
    if(lastmode != displaymode){
      changed = true;
      ResetButtonPressCnt();
    } else {
      changed = false;
    }
    switch( displaymode ){
      case 0:{
        NoneDisplayDefault( changed );     
      }

      case 1:{
        //We process and count button presses 
        ButtonPressCounter();
      }
      default:{
        NoneDisplayDefault( changed );
      } break;
    }
    lastmode = displaymode;    

  }   

}

/**************************************************************************************************
 *    Function      : LCDMenuShow
 *    Description   : LCDMenuShow will display a certain Menu 
 *    Input         : uint8_t Menu
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void LCDMenuShow( uint8_t Menu ){
  //This will force a Message to be shown
  //If the value is Zero it goes back to automode
  displaymode=Menu;
}

//----------------------------------------------------------------------------------
//As some parts in millis are nor ISR safe we need to 
//use code that already is in RAM and ISR safe
//----------------------------------------------------------------------------------


/**************************************************************************************************
 *    Function      : UserBtnISR
 *    Description   : Interrupt to handle button input
 *    Input         : void
 *    Output        : void
 *    Remarks       : none
 **************************************************************************************************/
void IRAM_ATTR UserBtnISR(){
    uint32_t millis = isr_millis();
    //We need to debounce this software wyse
    if( 0 == digitalRead( userbtn ) ){
      pressed=true;
      //Set the duration to zero
      press_start=millis;
      press_end=millis;
      press_duration=0;
      if( xBtnSemaphore != NULL ){
          xSemaphoreGiveFromISR( xBtnSemaphore, NULL );
          
      }
    } else {
      pressed = false;
      press_end=millis;
      press_duration = press_end - press_start;
      if(press_duration>=100){
        if( xBtnSemaphore != NULL ){
            xSemaphoreGiveFromISR( xBtnSemaphore, NULL );
            
        }
      }
      
    }
    //A press duration less than 500ms will be ignored
}

#undef DEBUG_SERIAL