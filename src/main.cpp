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
#define readCredentials false

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
            sendSync();
        }
    }

    counterLoops = (counterLoops >= 65535)? 0 : counterLoops + 1;
    LOG_S(DEBUG, __FILE__, __LINE__, "...loop()");
}
