#include <Arduino.h>
#include <WebServer.h>
#include "./src/ThinkSpeak/thinkspeak.h"
//We need here a webserver, a timecore and a ntpclient

void Webserver_Thinkspeak_RegisterThinkspeak( ThinkspeakUpload * Thpsp);
void Webserver_Thinkspeak_FunctionsRegister(WebServer* server);
//This will register all specific functions
 
 