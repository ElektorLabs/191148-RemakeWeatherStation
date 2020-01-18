#include <Arduino.h>
#include <WebServer.h>
//We need here a webserver, a timecore and a ntpclient
#include "./src/TimeCore/timecore.h"
#include "./src/NTPClient/ntp_client.h"

void Webserver_Time_FunctionsRegister(WebServer* server);
void Webserver_Time_FunctionRegisterTimecore(Timecore* TcPtr);
void Webserver_Time_FunctionRegisterNTPClient(NTP_Client* NtpCPtr);
//This will register all specific functions
 