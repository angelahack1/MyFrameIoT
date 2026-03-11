#ifndef NET_H
#define NET_H

#include <WiFi.h>
#include "log.h"

#define NET_HOST "192.168.1.14"
#define NET_PORT 80
#define NET_PATH "/api"
#define device_id "0000000000000001"
#define MAXRETRYS 10

static bool sendSyncPayload(void) {
    static const char payload[] = "{\"device_id\":\"" device_id "\",\"msg\":\"sync\"}";
    WiFiClient client;
    char statusLine[32] = "\0";

    if (!client.connect(NET_HOST, NET_PORT)) {
        return false;
    }

    client.print(F("POST " NET_PATH " HTTP/1.1\r\n"
                   "Host: " NET_HOST "\r\n"
                   "Content-Type: application/json\r\n"
                   "Connection: close\r\n"
                   "Content-Length: "));
    client.print(sizeof(payload) - 1);
    client.print(F("\r\n\r\n"));
    client.write(reinterpret_cast<const uint8_t *>(payload), sizeof(payload) - 1);
    client.setTimeout(1500);

    size_t len = client.readBytesUntil('\n', statusLine, sizeof(statusLine) - 1);
    statusLine[len] = '\0';
    client.stop();

    if (len < 10) {
        return false;
    }

    return statusLine[9] == '2';
}

bool sendSync(void) {
    LOG_D(DEBUG, __FILE__, __LINE__, "sendSync()...");
    for (uint8_t counterRetrys = 0; counterRetrys < MAXRETRYS; ++counterRetrys) {
        if (sendSyncPayload()) {
            LOG_D(DEBUG, __FILE__, __LINE__, "...sendSync()");
            return true;
        }
        delay(500);
    }
    LOG_D(DEBUG, __FILE__, __LINE__, "...sendSync()");
    return false;
}

#endif