#pragma once
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include "build_config.h"

static constexpr bool _debug = false;

typedef enum : uint8_t {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
} level;

#if MF_ENABLE_SERIAL_LOG

void setupLog(void) {
    Serial.begin(9600);
}

void LOG_S(level lvl, const char *stringFrom, int lineNumber, const char *text) {
    if( (!_debug) && (lvl == DEBUG) )
        return;
    switch(lvl) {
        case DEBUG:
            Serial.print(F("[DEBUG] "));
        break;
        case INFO:
            Serial.print(F("[INFO] "));
        break;
        case WARN:
            Serial.print(F("[WARNING] "));
        break;
        case ERROR:
            Serial.print(F("[ERROR] "));
        break;
        case FATAL:
            Serial.print(F("[FATAL] "));
        break;
        default:
            Serial.print(F("[ ] "));
    }
    Serial.print(stringFrom);
    Serial.print('(');
    Serial.print(lineNumber);
    Serial.print(F("): "));
    Serial.println(text);
}

#else

inline void setupLog(void) {}
#define LOG_S(...) ((void)0)

#endif

#if MF_ENABLE_SCREEN_LOG
void LOG_D(level lvl, const char *stringFrom, int lineNumber, const char *text);
#else
#define LOG_D(...) ((void)0)
#endif

#endif