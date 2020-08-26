/*
*   hue4esp32.h - A utility for interfacing Hue devices with the esp32.
*
*
*
*
*/


#include "hue4esp32.h"

#include <HTTPClient.h>
#include <WiFi.h>

DynamicJsonDocument  groupsAndScenesDoc(4000);

//
// Print Utility
//
// void prettyPrintArray(String arr[][])
// {
//     int x, y;
//     String test[10][10];
//     int total = sizeof(arr);
//     int column = sizeof(arr[0]);
//     int row = total / column;

//     for(x = 0; x < row; x++)
//     {
//         for(y = 0; y < column; y++)
//         {
//             Serial.printf("%s ", arr[x][y]);
//         }
//         Serial.println();
//     }
// }

//
//
//
hue4esp32Class::hue4esp32Class(char *ssid, char *password)
{
    _ssid = ssid;
    _password = password;
    userName = "EB1bIFxFmG5c4oHuZ1CSa5jBS06dtRyFgjak4wu9";
}

//
//
//
int hue4esp32Class::begin()
{
    //
    // First connect to the WiFi.
    //
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(_ssid);

    WiFi.begin(_ssid, _password);

    //
    // TODO: add timeout.
    //
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(10);
    }

    Serial.println();
    Serial.println("WiFi connected");
    selfIP = WiFi.localIP();
    Serial.print("IP address: ");
    Serial.println(selfIP);

    getBridgeIP();

    return 0;
}

//
//
//
IPAddress hue4esp32Class::getBridgeIP()
{
    String payload;
    //
    // get the IP address of the bridge from https://discovery.meethue.com/
    //
    payload = HttpGET("https://discovery.meethue.com/");

    //
    // Parse the response.
    //
    DynamicJsonDocument  doc(150);
    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();

    //
    // TODO: check doc first!
    //
    hueID = doc[0]["id"].as<String>();
    hueIP.fromString(doc[0]["internalipaddress"].as<String>());
    Serial.print("HueID:\t");
    Serial.println(hueID);
    Serial.print("hueIP:\t");
    Serial.println(hueIP);

    return hueIP;
}

//
//
//
void hue4esp32Class::getGroups(void)
{
    String payload, url;

    url = "https://";
    url.concat(hueIP.toString());
    url.concat("/api/");
    url.concat(userName);
    url.concat("/groups");
    Serial.println(url);
    payload = HttpGET(url);

    //
    // Parse response.
    //

    DynamicJsonDocument  doc(4000);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    JsonObject obj = doc.as<JsonObject>();
    uint8_t i = 0;
    for(JsonPair p : obj)
    {
        String number = p.key().c_str();
        String name = obj[p.key().c_str()]["name"];
        groupsAndScenesDoc[number] = NULL;
        JsonObject nested = groupsAndScenesDoc.createNestedObject(number);
        nested["name"] = name;
        JsonArray scenes = groupsAndScenesDoc[number].createNestedArray("scenes");
    }
    // serializeJsonPretty(groupsAndScenesDoc, Serial);
    // Serial.println();
}

//
//
//
void hue4esp32Class::getScenes()
{
    String payload, url;

    url = "https://";
    url.concat(hueIP.toString());
    url.concat("/api/");
    url.concat(userName);
    url.concat("/scenes");
    Serial.println(url);
    payload = HttpGET(url);

    //
    // Parse response.
    //
    DynamicJsonDocument  doc(20000);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    JsonObject obj1 = doc.as<JsonObject>();
    JsonObject obj2 = groupsAndScenesDoc.as<JsonObject>();
    for(JsonPair p : obj1)
    {
        String name = obj1[p.key().c_str()]["name"];
        String group = obj1[p.key().c_str()]["group"];

        if(group != "null")
        {
            JsonObject scenes = obj2[group]["scenes"].createNestedObject();
            scenes[name] = p.key().c_str();
        }

    }
    serializeJsonPretty(groupsAndScenesDoc, Serial);
    Serial.println();
}

//
//
//
uint8_t hue4esp32Class::setScene(String group, String scene)
{
    String payload, response, url, groupNum, sceneNum;

    //
    // Lookup numbers.
    //
    groupNum = lookupGroup(group);
    sceneNum = lookupScene(groupNum, scene);
    Serial.println(groupNum);
    Serial.println(sceneNum);

    //
    // construct url.
    //
    url = "https://";
    url.concat(hueIP.toString());
    url.concat("/api/");
    url.concat(userName);
    url.concat("/groups/");
    url.concat(groupNum);
    url.concat("/action");
    Serial.println(url);

    //
    // construct payload.
    //
    payload = "{\"scene\": \"";
    payload.concat(sceneNum);
    payload.concat("\"}");
    Serial.println(payload);

    //
    //
    //
    response = HttpPUT(url, payload);
    Serial.println(response);
}

//
// Look up group number using group name.
//
String hue4esp32Class::lookupGroup(String group)
{
    String groupNum;

    JsonObject obj = groupsAndScenesDoc.as<JsonObject>();
    for(JsonPair p : obj)
    {
        if(obj[p.key().c_str()]["name"] == group)
        {
            groupNum = p.key().c_str();
            return groupNum;
        }
    }

    return("NULL");
}

//
// Look up scene number using group number and scene name.
//
String hue4esp32Class::lookupScene(String groupNumber, String scene)
{
    JsonObject obj = groupsAndScenesDoc.as<JsonObject>();

    return obj[groupNumber]["scenes"][scene];
}

//
// HTTP Get utility.
//
String hue4esp32Class::HttpGET(String url)
{
    String response;
    HTTPClient http;
    int httpCode;

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot process GET. Not connected to WiFi!");
        return "";
    }

    http.begin(url);
    httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0)
    {
         // file found at server
         if(httpCode == HTTP_CODE_OK)
         {
           response = http.getString();
         }
    }
    else
    {
        Serial.printf("[HTTP] GET failed, error: %s\n",
                      http.errorToString(httpCode).c_str());
    }
    http.end();

    return response;
}

String hue4esp32Class::HttpPUT(String url, String payload)
{
    String response;
    HTTPClient http;
    int httpCode;

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot process GET. Not connected to WiFi!");
        return "";
    }

    http.begin(url);
    httpCode = http.PUT(payload);

    // httpCode will be negative on error
    if(httpCode > 0)
    {
         // file found at server
         if(httpCode == HTTP_CODE_OK)
         {
           response = http.getString();
         }
    }
    else
    {
        Serial.printf("[HTTP] GET failed, error: %s\n",
                      http.errorToString(httpCode).c_str());
    }
    http.end();

    return response;
}
