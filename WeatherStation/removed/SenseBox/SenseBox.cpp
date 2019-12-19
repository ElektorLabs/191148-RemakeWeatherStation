//upload the sensor values to senseBox
#include "sslCertificate.h"
#include "SenseBox.h"


SenseBoxUpload::SenseBoxUpload( void ){

}

SenseBoxUpload::~SenseBoxUpload( void ){

}

SenseBoxUpload::begin( bool usehttps){
  
  this->usesecure = usehttps;
  if(true == usehttps){
    clientS.setCACert(certificate);
  }
}


bool postdata() {
  WiFiClient* clientptr=nullptr;
  Serial.println("Uploading to SenseBox");
  if (senseBoxStationId == "") {
    Serial.println("SenseBox station ID not set");
    return false;
  }

  String csv;
  if (senseBoxWindSId != "")
    csv += senseBoxWindSId + "," + String(windSpeedAvg*3.6) + "\r\n"; //km/h
  if (senseBoxWindDId != "")
    csv += senseBoxWindDId + "," + String(windDirAvg) + "\r\n";
  if (senseBoxRainId != "")
    csv += senseBoxRainId + "," + String(rainAmountAvg) + "\r\n";
  if (senseBoxTempId != "")
    csv += senseBoxTempId + "," + String(temperature) + "\r\n";
  if (senseBoxHumId != "")
    csv += senseBoxHumId + "," + String(humidity) + "\r\n";
  if (senseBoxPressId != "")
    csv += senseBoxPressId + "," + String(pressure) + "\r\n";
  if (senseBoxPM25Id != "")
    csv += senseBoxPM25Id + "," + String(PM25) + "\r\n";
  if (senseBoxPM10Id != "")
    csv += senseBoxPM10Id + "," + String(PM10) + "\r\n";

  if (csv == "") {
    Serial.println("Sensor API keys not set");
    return false;
  }
  
  String senseBoxHost = "api.opensensemap.org";
  String senseBoxUrl = "/boxes/" + senseBoxStationId + "/data";
  String headers = "Content-Type: text/csv\r\n";
  headers += "Connection: close\r\n";
  headers += "Content-Length: " + String(csv.length()) + "\r\n";
  if(true == this->usesecure){
      clientptr=&clientS;
  } else {
     clientptr=&client;
  }

  
  String resp = performRequest(clientptr, senseBoxHost, senseBoxUrl, 443, "POST", headers, csv);
  return true;
}

//request the specified url of the specified host
String performRequest(WiFiClient* c , String host, String url, int port = 80, String method = "GET", String headers = "Connection: close\r\n", String data = "") {
  
  Serial.println("Connecting to host '" + host + "' on port " + String(port));
  c->connect(host.c_str(), port); //default ports: http: port 80, https: 443
  String request = method + " " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   headers + "\r\n";
  Serial.println("Requesting url: " + request);
  c->print(request);
  if (data != "") {
    Serial.println("Data: " + data);
    c->print(data + "\r\n");
  }
  
  unsigned long timeout = millis();
  while (c->available() == 0) {
    if (timeout + 5000 < millis()) {
      Serial.println("Client timeout");
      c->stop();
      return "";
    }
  }
  //read client reply
  String response;
  while(c->available()) {
    response = c->readStringUntil('\r');
  }
  Serial.println("Response: " + response);
  c->stop();
  return response;
}

void configureClient() {
  client = new WiFiClient();
  clientS = new WiFiClientSecure();
  clientS->setCACert(certificate);
}