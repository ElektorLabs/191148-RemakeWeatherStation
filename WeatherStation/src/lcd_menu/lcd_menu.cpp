#include "../i2c_access/i2c_access.h"

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

//General if the button is pressed <3s the backlight will light up for 60 seconds
//If button is pressed for more than 20 seconds we will enter WPS mode !


#include "../../wifi_net.h"
#include "lcd_menu.h"



void LCDMenu::begin( void ){
    i2c_lock_bus();
    //lcd.init();                      // initialize the lcd 
    //lcd.backlight();
    i2c_unlock_bus();
}

void LCDMenu::DisableLCD( void ){
    //This will clear the lcd and disable the backligt if possible
    i2c_lock_bus();
    //lcd.noDisplay();
    //lcd.nobacklight();
    i2c_unlock_bus();
}

void LCDMenu::EnableLCD( void ){
    //This will enable lcd and the backligt if possible
    i2c_lock_bus();
    //lcd.Display();
    //lcd.backlight();    
    i2c_unlock_bus();
}


void LCDMenu::DefaultDisplay( void ){
  wifi_connection_info_t ConnectionInfo;
  GetWiFiConnectionInfo( &ConnectionInfo );
  i2c_lock_bus();
  if(ConnectionInfo.IsAP=true){
    //We are in AP mode....
    //lcd.setCursor(0,0);
    //lcd.print("WiFi: AP        ");
    //lcd.setCursor(1,0);
    //lcd.print("192.168.4.1     ");
  } else if(ConnectionInfo.IsStation==true) {
    //We are in STA mode....
    if(ConnectionInfo.Connected=true){
      //  lcd.setCursor(0,0);
      // lcd.print("WiFi:");
      //  lcd.print(WiFiGetRSSI());
      //  lcd.print("dBm    ");
      //  lcd.setCursor(1,0);
      //  lcd.print(ConnectionInfo.IP_Info.SystemIP);
      //  lcd.print("         ");
    } else {
      //  lcd.setCursor(0,0);
      //  lcd.print("WiFi: No con.   ");
      //  lcd.setCursor(1,0);
      //  lcd.print("                ");
    }
  } else {
    //WiFi Off
    //lcd.setCursor(0,0);
    //lcd.print("WiFi: OFF   ");
    //lcd.setCursor(1,0);
    //lcd.print("                ");

  }
  i2c_unlock_bus();

}