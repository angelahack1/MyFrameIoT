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

// Module name for this file - used in logging
static const char MODULE[] PROGMEM = "MAIN";

void setup() {
    // Debounce Serial initialization to avoid bootloader collision
    // Wait for any pending bootloader communication to settle
    delay(100);
    
    // Initialize logging subsystem
    LOG_INIT();
    
    // System boot checkpoint
    LOGI("System boot");
    LOGD("Build: %s %s", __DATE__, __TIME__);
    LOGD("Free heap at boot: %d bytes", ESP.getFreeHeap());
    
    // SD card initialization
    LOGI("Initializing SD...");
    setupSD();
    LOGI("SD initialized");
    
    // Screen subsystem initialization
    LOGI("Initializing screen...");
    setupScreen();
    cleanScr();
    LOGI("Screen ready");
    
#if MF_ENABLE_WIFI
    LOGI("WiFi enabled - starting scan...");
    wifiScan();
    LOGI("WiFi scan complete");

#if MF_READ_WIFI_CREDENTIALS_FROM_SD
    LOGI("Loading WiFi credentials from SD...");
    setupCred();
    wifiConnectFromCredFile();
    if (WiFi.status() == WL_CONNECTED) {
        LOGI("WiFi connected via SD credentials");
    } else {
        LOGW("WiFi connection failed");
    }
#else
    LOGI("Connecting to hardcoded WiFi...");
    wifiConnect();
#endif

#else
    LOGW("WiFi disabled in build config");
#endif

#if MF_ENABLE_SYNC
    LOGI("Network sync enabled");
#else
    LOGW("Network sync disabled");
#endif

    LOGD("Setup complete - entering main loop");
    
    // Legacy log for compatibility
    LOG_S(DEBUG, __FILE__, __LINE__, "Legacy: setup() completed");
}

void loop() {
    LOG_TRACE(MODULE, "Loop entry, counter=%u", counterLoops);
    
    // Run the main reel/screen update
    runReel();
    LOG_TRACE(MODULE, "Reel update done");
    
    // Sync operation on even iterations
    if ((counterLoops & 1u) == 0u) {
#if MF_ENABLE_WIFI && MF_ENABLE_SYNC
        LOG_DEBUG(MODULE, "Sending sync (loop=%u)...", counterLoops);
        
        bool syncResult = sendSync();
        
        if (syncResult) {
            LOG_INFO(MODULE, "Sync sent successfully");
        } else {
            LOG_WARN(MODULE, "Sync failed to send");
        }
        
        delay(2000);
#else
        LOG_DEBUG(MODULE, "Skipping sync - disabled (loop=%u)", counterLoops);
#endif
    }
    
    // Check for counter overflow
    if (counterLoops == 0xFFFF) {
        LOG_WARN(MODULE, "Loop counter overflow imminent!");
    }
    
    counterLoops++;
    
    LOG_TRACE(MODULE, "Loop exit, next counter=%u", counterLoops);
    
    // Periodic memory status every 100 loops
    if ((counterLoops % 100) == 0) {
        LOGD("Memory check - Free heap: %d, Min free: %d", 
             ESP.getFreeHeap(), ESP.getMinFreeHeap());
    }
    
    // Legacy log for compatibility
    LOG_S(DEBUG, __FILE__, __LINE__, "Legacy: loop() iteration done");
}
