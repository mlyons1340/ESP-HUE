/*
 * 
 * 
 * 
 * 
 * 
 */
 
#include "hue4esp32.h"

char* ssid      = "357";
char* password  = "5124970100";

void setup()
{
  Serial.begin(115200);

  hue4esp32Class hue(ssid, password);
  hue.begin();
  hue.getGroups();
  hue.getScenes();

  hue.setScene("Living room", "Bright"); 
}

void loop()
{
  delay(5000);
}
