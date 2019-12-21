//upload the sensor values to senseBox
#include "SPIFFS.h"
#include "sslCertificate.h"
#include "SenseBox.h"


SenseBoxUpload::SenseBoxUpload( void ){

  //For the Upload we also use a mapping table but limit the amount of data currently to 16 entrys
  //For every entry we have a internal channel no and also a senseBoxID at hand ( hopfully )


}

SenseBoxUpload::~SenseBoxUpload( void ){

}

SenseBoxUpload::begin( bool usehttps){
  
  this->usesecure = usehttps;
  if(true == usehttps){
    clientS.setCACert(certificate);
  }
}

void SenseBoxUpload::WriteMapping( void ){
  //This will just convert the Mapping to JSON and write it to the local filesystem
   File file = SPIFFS.open("/SenseBoxMapping.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    JsonArray SenseboxMapping = doc.createNestedArray("Mapping");
    for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
            JsonObject MappingObj = SenseboxMapping.createNestedObject();
            MappingObj["Enabled"] = Mapping[i].enable;
            MappingObj["Channel"] = Mapping[i].StationChannelIdx;
            if(sizeof(Mapping[i].SenseBoxSensorID)>1){
              Mapping[i].SenseBoxSensorID[(sizeof(Mapping[i].SenseBoxSensorID)-1)]=0; //Just to terminate the string propper
              MappingObj["Key"] = Strig(Mapping[i].SenseBoxSensorID);
            } else {
              MappingObj["Key"] = "";
            }
    }
    serializeJson(doc, file);
}

void SenseBoxUpload::ReadMapping( void ){

//Config will be stored as JSON String on SPIFFS
    //This makes mapping more complicated but will easen web access
    if(SPIFFS.exists("/SenseBoxMapping.json")){
        File file = SPIFFS.open("/SenseBoxMapping.json");
        //We need to read the file into the ArduinoJson Parser
         /*
        ReadBufferingStream bufferingStream(file, 64);
        deserialzeJson(doc, bufferingStream);
        */
        
        const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3) + 1580;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, file);
        JsonArray SenseboxMapping = doc["Mapping"];
        for(uint8_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] );i++){
            JsonObject MappingObj = SenseboxMapping[i];
           
            Mapping[i].enable =MappingObj["Enabled"];
            Mapping[i].StationChannelIdx = MappingObj["Channel"];
            String Key = MappingObj["Key"];

            if(Key.lenght()<32){
              Mapping[i].SenseBoxSensorID = Key.c_str();
            } else {
              Mapping[i].SenseBoxSensorID[0]=0;
            }

        }
    } else {
        //We need to create a blank mapping scheme
        for(uint32_t i=0;i<( sizeof(Mapping) / sizeof( Mapping[0] )  );i++){
              Mapping[i].enable;
              Mapping[i].StationChannelIdx;
            for(uint8_t i=0; i< sizeof(Mapping[i].SenseBoxSensorID);i++){
              Mapping[i].SenseBoxSensorID[i]=0;
            }
         }
         WriteMapping();
    }

}

void WriteSettings(){

    File file = SPIFFS.open("/SenseBoxSetting.json", FILE_WRITE);
    const size_t capacity = JSON_ARRAY_SIZE(64) + JSON_OBJECT_SIZE(1) + 64*JSON_OBJECT_SIZE(3)+(64*16);
    DynamicJsonDocument doc(capacity);

    serializeJson(doc, file);


}

void ReadSettings(){

      if(SPIFFS.exists("/SenseBoxSetting.json")){
        File file = SPIFFS.open("/SenseBoxSetting.json");
      }

      

}



bool postdata() {
  WiFiClient* clientptr=nullptr;
  /*
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
  */
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