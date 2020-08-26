/*
*   hue4esp32.h - A utility for interfacing Hue devices with the esp32.
*
*
*
*
*/

#ifndef HUE4ESP32_H
#define HUE4ESP32_H

#include <WiFi.h>
#include <ArduinoJson.h>

#define MAX_GROUPS          10
#define MAX_SCENES          10
class hue4esp32Class
{
    public:
        String hueID, userName;
        IPAddress hueIP, selfIP;
        String groupsAndScenes[MAX_GROUPS][MAX_SCENES];

        hue4esp32Class(char *ssid, char *password);
        int begin();
        IPAddress getBridgeIP();
        void getGroups();
        void getScenes();
        uint8_t setScene(String group, String scene);

    private:
        char* _ssid;
        char* _password;

        String lookupGroup(String group);
        String lookupScene(String groupNumber, String scene);
        String HttpGET(String url);
        String HttpPUT(String url, String payload);


};




#endif //HUE4ESP32_H
