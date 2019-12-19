//upload the sensor data to thingspeak
bool uploadToThingspeak() {
  Serial.println("Uploading to thingspeak");
  if (thingspeakApi == "") {
    Serial.println("Thingspeak API key not set");
    return false;
  }
  
  printUploadValues();
  
  String thingspeakHost = "api.thingspeak.com";
  String thingspeakUrl = "/update";
  thingspeakUrl += "?api_key=" + thingspeakApi;
  if (tsfWindSpeed != 0)
    thingspeakUrl += "&field" + String(tsfWindSpeed) + "=" + String(windSpeedAvg*3.6); //km/h
  if (tsfWindDir != 0)
    thingspeakUrl += "&field" + String(tsfWindDir) + "=" + String(windDirAvg);
  if (tsfRainAmount != 0)
    thingspeakUrl += "&field" + String(tsfRainAmount) + "=" + String(rainAmountAvg);
  if (tsfTemperature != 0)
    thingspeakUrl += "&field" + String(tsfTemperature) + "=" + String(temperature);
  if (tsfHumidity != 0)
    thingspeakUrl += "&field" + String(tsfHumidity) + "=" + String(humidity);
  if (tsfAirpressure != 0)
    thingspeakUrl += "&field" + String(tsfAirpressure) + "=" + String(pressure);
  if (tsfPM25 != 0)
    thingspeakUrl += "&field" + String(tsfPM25) + "=" + String(PM25);
  if (tsfPM10 != 0)
    thingspeakUrl += "&field" + String(tsfPM10) + "=" + String(PM10);

  String resp = performRequest(false, thingspeakHost, thingspeakUrl);
  return (resp != "" && !resp.startsWith("0"));
}

void printUploadValues() {
  Serial.println("Values:");
  Serial.println("WindSpeedAvg:  " + String(windSpeedAvg));
  Serial.println("WindDirAvg:    " + String(windDirAvg));
  Serial.println("RainAmountAvg: " + String(rainAmountAvg));
  Serial.println("Temperature:   " + String(temperature));
  Serial.println("Humidity:      " + String(humidity));
  Serial.println("Pressure:      " + String(pressure));
  Serial.println("PM25:          " + String(PM25));
  Serial.println("PM10:          " + String(PM10));
}

//request the specified url of the specified host
String performRequest(bool secure, String host, String url, int port = 80, String method = "GET", String headers = "Connection: close\r\n", String data = "") {
  WiFiClient* c = (secure ? clientS : client);
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
