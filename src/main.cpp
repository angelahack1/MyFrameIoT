#include <Arduino.h>
#include "log.h"
#include "screen_subsystem.h"
#include "wifi_subsystem.h"
#include "net.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

bool connected = false;
bool disconnectIssued = false;
int counterLoops = 0;
#define connectWiFi true
#define sendSyncReqs true
#define readCredentials true

void setup() {
    setupLog();    
    setupSD();
    setupScreen();
    cleanScr();
    if(connectWiFi) {
        wifiScan();
        if(readCredentials) {
            setupCred();
            wifiConnectFromCredFile();
        } else {
            wifiConnect();
        }
    }
    connected = true;
    LOG_S(DEBUG, __FILE__, __LINE__, "setup()...");
}

void loop() {
    LOG_S(DEBUG, __FILE__, __LINE__, "loop()...");
    runReel();

    if((counterLoops % 2) == 0) {
        if(sendSyncReqs && connectWiFi) {
            LOG_D(INFO, __FILE__, __LINE__, "About 2 send sync...");
            sendSync();
            if(sentOk)
                LOG_D(INFO, __FILE__, __LINE__, "...Sync sent!");
            else
                LOG_D(INFO, __FILE__, __LINE__, "...Sync Not sent!");
            delay(2000);
        }
    }

    counterLoops = (counterLoops >= 65535)? 0 : counterLoops + 1;
    LOG_S(DEBUG, __FILE__, __LINE__, "...loop()");
}
