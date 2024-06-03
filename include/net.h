#ifndef NET_H
#define NET_H
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "log.h"

#define ADDRESS "http://192.168.1.14/api"
#define device_id "0000000000000001"


String sendJson(DynamicJsonDocument &pDoc) {
    LOG_S(DEBUG, __FILE__, __LINE__, "setupNet()...");
    HTTPClient http;
    String response, json;
    
    http.begin(ADDRESS);
    http.addHeader("Content-Type", "application/json ");
    serializeJson(pDoc, json);
    
    int httpResponseCode = http.POST(json);

    if (httpResponseCode > 0) {
      response = http.getString();
      char outputBuffer[50];
      sprintf(outputBuffer, "Response: %s", response.c_str());
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      sprintf(outputBuffer, "Response code: %d", httpResponseCode);
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      return response;
    } else {
      char outputBuffer[50];
      sprintf(outputBuffer, "Error sending POST: %d", httpResponseCode);
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      return String("error: ") + String(outputBuffer);
    }
    http.end();
    LOG_S(DEBUG, __FILE__, __LINE__, "...setupNet()");
}

void sendSync(void) {
    LOG_S(DEBUG, __FILE__, __LINE__, "sendSync()...");
    DynamicJsonDocument doc(256);
    doc["device_id"] = device_id;
    doc["msg"] = "sync";
    String response = sendJson(doc);
    LOG_S(DEBUG, __FILE__, __LINE__, "...sendSync()");
}
#endif