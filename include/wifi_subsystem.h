#pragma once
#ifndef WIFI_SUBSYSTEM_H
#define WIFI_SUBSYSTEM_H

#include <WiFi.h>

#define LSSID "AiXKare-4G"
#define LSSIDP "1164Louder!#"

char namebufCred[] = "/data/credentials.dat";
File rootCred;
int pathlenCred = 21;
char LSSIDF[32] = "\0";
char LSSIDFP[32] = "\0";


void wifiScan(void) {
    char buffer[50] = "\0";
    char bufferTemp[10] = "\0";
    LOG_D(INFO, __FILE__, __LINE__, "wifiScan()...");
    int n = WiFi.scanNetworks();
    LOG_D(INFO, __FILE__, __LINE__, "Scan done!");
    if (n == 0) {
        LOG_D(INFO, __FILE__, __LINE__, "No networks found!");
    } else {
        strcpy(buffer, "Networks found: ");
        itoa(n, bufferTemp, 10);
        strcat(buffer, bufferTemp);
        LOG_D(INFO, __FILE__, __LINE__, buffer);
        for (int i = 0; i < n; ++i) {
            strcpy(buffer, "");
            itoa(i + 1, bufferTemp, 10);
            strcat(buffer, bufferTemp);
            strcat(buffer, ": ");
            strcat(buffer, WiFi.SSID(i).c_str());
            strcat(buffer, " (");
            itoa(WiFi.RSSI(i), bufferTemp, 10);
            strcat(buffer, bufferTemp);
            strcat(buffer, ")");
            strcat(buffer, (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "*" : " " );
            printText(buffer);
        }
    }
    LOG_D(INFO, __FILE__, __LINE__, "...wifiScan()");
}

void wifiConnect(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiConnect()...");
    char buffer[64];
    sprintf(buffer, "Cx: <%s>", LSSID);
    LOG_D(INFO, __FILE__, __LINE__, buffer);
    sprintf(buffer, "Px: <%s>", LSSIDP);
    LOG_D(INFO, __FILE__, __LINE__, buffer);
    WiFi.begin(LSSID, LSSIDP);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        LOG_D(INFO, __FILE__, __LINE__, "Connecting WiFi...");
    }

    if(WiFi.isConnected())
        LOG_D(INFO, __FILE__, __LINE__, ">>WiFi Connected!!<<");
    else 
        LOG_D(INFO, __FILE__, __LINE__, "xxWiFi Not connected!!xx");
    LOG_D(INFO, __FILE__, __LINE__, "...wifiConnect()");
}

void wifiConnectFromCredFile(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiConnectFromCredFile()...");
    char buffer[64];
    sprintf(buffer, "Cx: <%s>", LSSIDF);
    LOG_D(INFO, __FILE__, __LINE__, buffer);
    sprintf(buffer, "Px: <%s>", LSSIDFP);
    LOG_D(INFO, __FILE__, __LINE__, buffer);
    WiFi.begin(LSSIDF, LSSIDFP);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        LOG_D(INFO, __FILE__, __LINE__, "Connecting WiFi...");
    }

    if(WiFi.isConnected())
        LOG_D(INFO, __FILE__, __LINE__, ">>WiFi Connected!!<<");
    else 
        LOG_D(INFO, __FILE__, __LINE__, "xxWiFi Not connected!!xx");
    LOG_D(INFO, __FILE__, __LINE__, "...wifiConnectFromCredFile()");
}

void wifiDisconnect(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiDisconnect()...");

    WiFi.disconnect(true);
    while (WiFi.status() == WL_CONNECTED) {
        delay(500);
        LOG_D(INFO, __FILE__, __LINE__, "Disconnecting WiFi...");
    }

    if(!WiFi.isConnected())
        LOG_D(INFO, __FILE__, __LINE__, ">>WiFi Disconnected!!<<");
    else 
        LOG_D(INFO, __FILE__, __LINE__, "xxWiFi Still connected XC!!xx");
    LOG_D(INFO, __FILE__, __LINE__, "...wifiDisconnect()");
}

void setupCred(void) {
    LOG_D(INFO, __FILE__, __LINE__, "setupCred()...");
    rootCred = SD.open(namebufCred);

    if (!rootCred) {
        LOG_D(INFO, __FILE__, __LINE__, "Failed to open credentials file");
        return;
    }
    String line = rootCred.readStringUntil('\n');
    int commaIndex = line.indexOf(',');
    String str1 = line.substring(0, commaIndex);
    String str2 = line.substring(commaIndex + 1);
    strcpy(LSSIDF, str1.c_str());
    strcpy(LSSIDFP, str2.c_str());
    LOG_D(INFO, __FILE__, __LINE__, "...setupCred()");
}

#endif
