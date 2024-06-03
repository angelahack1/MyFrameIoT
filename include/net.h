#ifndef NET_H
#define NET_H
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "log.h"

#define ADDRESS "http://192.168.1.14/api"
#define device_id "0000000000000001"
#define MAXRETRYS 10


String sendJson(DynamicJsonDocument &pDoc) {
    LOG_D(DEBUG, __FILE__, __LINE__, "setupNet()...");
    HTTPClient http;
    String response, json;
    int counterRetrys = 0;
    
    http.begin(ADDRESS);
    http.addHeader("Content-Type", "application/json ");
    serializeJson(pDoc, json);
    
    int httpResponseCode = http.POST(json);

    while(httpResponseCode < 0) {
      http.end();
      delay(500);
      LOG_D(WARN, __FILE__, __LINE__, "Retrying...");
      LOG_S(WARN, __FILE__, __LINE__, "Retrying...");
      http.begin(ADDRESS);
      http.addHeader("Content-Type", "application/json ");
      httpResponseCode = http.POST(json);
      if(counterRetrys >= MAXRETRYS) {
        LOG_D(WARN, __FILE__, __LINE__, "Reytrys stopped XC.");
        LOG_S(WARN, __FILE__, __LINE__, "Reytrys stopped XC.");
        break;
      }
      counterRetrys++;
    }

    if( (httpResponseCode > 0) && (counterRetrys < MAXRETRYS) && (counterRetrys > 0) ) {
        char tempBuffer[50];
        sprintf(tempBuffer, "Retry (%d) success!", counterRetrys);
        LOG_D(INFO, __FILE__, __LINE__, tempBuffer);
        LOG_S(INFO, __FILE__, __LINE__, tempBuffer);
    }

    if (httpResponseCode > 0) {
      response = http.getString();
      char outputBuffer[50];
      sprintf(outputBuffer, "Response: %s", response.c_str());
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      sprintf(outputBuffer, "Response code: %d", httpResponseCode);
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      LOG_D(INFO, __FILE__, __LINE__, outputBuffer);
      return response;
    } else {
      char outputBuffer[50];
      sprintf(outputBuffer, "Error Tx POST: %d", httpResponseCode);
      LOG_S(INFO, __FILE__, __LINE__, outputBuffer);
      LOG_S(INFO, __FILE__, __LINE__, (char*)http.errorToString(httpResponseCode).c_str());
      LOG_D(INFO, __FILE__, __LINE__, outputBuffer);
      return String("error: ") + String(outputBuffer);
    }
    http.end();
    LOG_D(DEBUG, __FILE__, __LINE__, "...setupNet()");
}

void sendSync(void) {
    LOG_D(DEBUG, __FILE__, __LINE__, "sendSync()...");
    DynamicJsonDocument doc(256);
    doc["device_id"] = device_id;
    doc["msg"] = "sync";
    String response = sendJson(doc);
    LOG_D(DEBUG, __FILE__, __LINE__, "...sendSync()");
}
#endif