#include <Arduino.h>
#include <WebServer.h>

void SetupWebServer( ); // http version 
WebServer* WebserverGetHandle( void ); //Used to register new Functions to the Server
void WebserverLoopTask( void );
