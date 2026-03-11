#pragma once
#ifndef WIFI_SUBSYSTEM_H
#define WIFI_SUBSYSTEM_H

#include <SD.h>
#include <WiFi.h>
#include "build_config.h"
#include "log.h"

#define LSSID "AiXKare-4G"
#define LSSIDP "1164Louder!#"

static char namebufCred[] = "/data/credentials.dat";
File rootCred;
char LSSIDF[32] = "\0";
char LSSIDFP[32] = "\0";

#if MF_ENABLE_WIFI_SCAN

void wifiScan(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiScan()...");
    int n = WiFi.scanNetworks();
    (void)n;
    WiFi.scanDelete();
    LOG_D(INFO, __FILE__, __LINE__, "...wifiScan()");
}

#else

inline void wifiScan(void) {}

#endif

static void wifiConnectImpl(const char *ssid, const char *password) {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void wifiConnect(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiConnect()...");
    wifiConnectImpl(LSSID, LSSIDP);
    LOG_D(INFO, __FILE__, __LINE__, "...wifiConnect()");
}

void wifiConnectFromCredFile(void) {
    LOG_D(INFO, __FILE__, __LINE__, "wifiConnectFromCredFile()...");
    wifiConnectImpl(LSSIDF, LSSIDFP);
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
    char line[80] = "\0";
    size_t read = rootCred.readBytesUntil('\r', line, sizeof(line) - 1);
    line[read] = '\0';

    char *comma = strchr(line, ',');
    if (comma != NULL) {
        *comma = '\0';
        strncpy(LSSIDF, line, sizeof(LSSIDF) - 1);
        strncpy(LSSIDFP, comma + 1, sizeof(LSSIDFP) - 1);
        LSSIDF[sizeof(LSSIDF) - 1] = '\0';
        LSSIDFP[sizeof(LSSIDFP) - 1] = '\0';
    }

    rootCred.close();
    LOG_D(INFO, __FILE__, __LINE__, "...setupCred()");
}

#endif
