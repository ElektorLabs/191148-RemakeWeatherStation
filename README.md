# 191148-Remake Weather Station

This is the software to be used with the Elektor weather station remake of 2020.

The core of the station is a ESP32 Pico Kit V4.x designed around the WH-SP-WS02 ( https://www.elektor.de/professional-outdoor-weather-station-wh-sp-ws02 )
The software and also some parts of the webpage have been overhauled or completely rewritten. 

The new modular design should allow for the later addition of new sensors or features with less hassle than currently.

## Getting Started

Use your current Arduino IDE to compile the Code. Also, the ESP32 Arduino Core board library must be installed.
Have a look at : https://www.elektormagazine.com/labs/esp32-getting-started for a getting started guide.

You will need the following libraries:

* LiquidCrystal_I2C ( https://github.com/johnrickman/LiquidCrystal_I2C )
* eHaJo Absolute Pressure AddOn by Hannes Jochriem ( https://github.com/ehajo/WSEN-PADS )
* Adafruit Unified Sensors
* Adafruit BME280 Library
* Adafruit VEML6070 Library
* Adafruit VEML6075 Library/
* Adafruit TSL2561 Library
* Adafruit TSL2591 Library
* Adafruit BusIO
* Time by Michael Margolis
* ArduinoJson 6.x 
* PubSubclient by Nick o'Leary
* NTP Client Lib by German Martin

### Pre-requisites

If you try to program the ESP32 Pico Kit, be aware that using more modern USB3.x ports for data transfer may cause the board
to fail to enter programming mode. Use a USB2.0 port or at least a USB2.0 Hub to get around this. 

## Pin Mappings

Currently the following pin map is used:

* IO02 : RES  ( RFM95 )
* IO14 : SCKL ( RFM95 )
* IO12 : MISO ( RFM95 )
* IO13 : MOSI ( RFM95 )
* IO15 : NSS  ( RFM95 )
* IO32 : DIO2 ( RFM95 )
* IO33 : DIO1 ( RFM95 )
* IO34 : DIO0 ( RFM95 )
* IO25 : SCL
* IO26 : SDA
* IO34 : WIND SPEED
* IO38 : RAIN
* IO37 : WIND DIR
* IO19 : MISO ( SD-CARD )
* IO23 ; MOSI ( SD-CARD )
* IO18 : SCKL ( SD-CARD )
* IO05 : CS0  ( SD-CARD )
* IO09 : TX1 ( Particle Sensor )
* IO10 : RX1 ( Particle Sensor )
