/*
 * 
 * 
 * 
 * 
 * 
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

 WiFiMulti wifiMulti;

const char* ssid      = "357";
const char* password  = "5124970100";
const String HueIP     = "192.168.1.100";
const String UserName  = "EB1bIFxFmG5c4oHuZ1CSa5jBS06dtRyFgjak4wu9";

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  // We start by connecting to a WiFi network
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  wifiMulti.addAP(ssid, password);
  while((wifiMulti.run() != WL_CONNECTED))
  {
    delay(100);
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if((wifiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    String payload;
    static int onOff = true;

    http.begin("https://" + HueIP + "/api/" + UserName + "/lights/3/state");
//    http.addHeader("Content-Type", "application/json");
    if(onOff)
    {
      payload = "{\"on\":true}";
    }
    else
    {
      payload = "{\"on\":false}";
    }
    onOff = !onOff;
    int httpCode = http.PUT(payload);
//    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] code: %d\n", httpCode);
      
      // file found at server
      if(httpCode == HTTP_CODE_OK)
      {
        payload = http.getString();
        Serial.println(payload);
      }
    }
    else
    {
      Serial.printf("[HTTP] failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  delay(5000);
}
