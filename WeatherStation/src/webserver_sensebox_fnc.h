#include <Arduino.h>
#include <WebServer.h>
#include "./src/SenseBox/SenseBox.h"
//We need here a webserver, a timecore and a ntpclient

void Webserver_SenseBox_RegisterSensebox( SenseBoxUpload * Sbxptr);
void Webserver_SenseBox_FunctionsRegister(WebServer* server);
//This will register all specific functions
 