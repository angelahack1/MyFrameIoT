#include <Arduino.h>
#include "build_config.h"
#include "log.h"
#include "screen_subsystem.h"

#if MF_ENABLE_WIFI
#include "wifi_subsystem.h"
#endif

#if MF_ENABLE_SYNC
#include "net.h"
#endif

static uint16_t counterLoops = 0;

void setup() {
    setupLog();
    setupSD();
    setupScreen();
    cleanScr();

#if MF_ENABLE_WIFI
    wifiScan();

#if MF_READ_WIFI_CREDENTIALS_FROM_SD
    setupCred();
    wifiConnectFromCredFile();
#else
    wifiConnect();
#endif

#endif

    LOG_S(DEBUG, __FILE__, __LINE__, "setup()...");
}

void loop() {
    LOG_S(DEBUG, __FILE__, __LINE__, "loop()...");
    runReel();

    if ((counterLoops & 1u) == 0u) {
#if MF_ENABLE_WIFI && MF_ENABLE_SYNC
        if (sendSync()) {
            LOG_D(INFO, __FILE__, __LINE__, "...Sync sent!");
        } else {
            LOG_D(INFO, __FILE__, __LINE__, "...Sync Not sent!");
        }
        delay(2000);
#endif
    }

    counterLoops++;
    LOG_S(DEBUG, __FILE__, __LINE__, "...loop()");
}