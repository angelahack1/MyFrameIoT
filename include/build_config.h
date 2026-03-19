#pragma once

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

/*
 * Build Configuration for MyFrameIoT
 * 
 * Logging Configuration:
 * ---------------------
 * MF_ENABLE_SERIAL_LOG  - Enable/disable serial output (0=disabled, 1=enabled)
 * MF_LOG_LEVEL          - Compile-time filter: TRACE(6), DEBUG(5), INFO(4), 
 *                         WARN(3), ERROR(2), FATAL(1), NONE(0)
 * MF_LOG_MAX_MESSAGE_LEN - Max formatted message length (default: 128)
 * MF_LOG_SHOW_MODULE    - Show module name in logs (0/1, saves ~4-8 bytes/log)
 * MF_LOG_SHOW_LINE      - Show line numbers in logs (0/1, saves ~4 bytes/log)
 * 
 * To maximize memory savings:
 *   #define MF_ENABLE_SERIAL_LOG 0
 *   // All logging code is completely removed - ZERO overhead
 * 
 * To enable production logging:
 *   #define MF_ENABLE_SERIAL_LOG 1
 *   #define MF_LOG_LEVEL MF_LOG_LEVEL_INFO
 *   // Only INFO and above compiled in, DEBUG/TRACE removed
 */

// Feature toggles
#define MF_ENABLE_SERIAL_LOG 0      // Set to 1 to enable serial logging
#define MF_ENABLE_SCREEN_LOG 0      // Set to 1 to enable screen logging
#define MF_ENABLE_WIFI 1            // WiFi subsystem
#define MF_ENABLE_WIFI_SCAN 0       // WiFi scanning at boot
#define MF_ENABLE_SYNC 1            // Network sync operations
#define MF_READ_WIFI_CREDENTIALS_FROM_SD 1  // Read WiFi from SD card

// Log level (only used if MF_ENABLE_SERIAL_LOG=1)
// Levels: TRACE(6) > DEBUG(5) > INFO(4) > WARN(3) > ERROR(2) > FATAL(1) > NONE(0)
// Lower levels are filtered out at COMPILE TIME - zero runtime cost
#ifndef MF_LOG_LEVEL
  #define MF_LOG_LEVEL MF_LOG_LEVEL_DEBUG
#endif

// Memory tuning (optional, defaults are reasonable)
// #define MF_LOG_MAX_MESSAGE_LEN 128  // Max formatted message size
// #define MF_LOG_SHOW_MODULE 1         // Show module name prefix
// #define MF_LOG_SHOW_LINE 1           // Show line number

#endif
