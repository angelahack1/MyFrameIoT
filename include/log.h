#pragma once
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

bool _debug = false;

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
} level;

void setupLog(void) {
    Serial.begin(9600);
}

void LOG_S(level lvl, char *stringFrom, int lineNumber, char *text) {
    if( (!_debug) && (lvl == DEBUG) )
        return;
    char outputBuffer[150] = "\0";
    char tempBuffer[10] = "\0";

    switch(lvl) {
        case DEBUG:
            strcat(outputBuffer, "[DEBUG] ");
        break;
        case INFO:
            strcat(outputBuffer, "[INFO] ");
        break;
        case WARN:
            strcat(outputBuffer, "[WARNING] ");
        break;
        case ERROR:
            strcat(outputBuffer, "[ERROR] ");
        break;
        case FATAL:
            strcat(outputBuffer, "[FATAL] ");
        break;
        default:
            strcat(outputBuffer, "[ ] ");
    }
    strcat(outputBuffer, stringFrom);
    strcat(outputBuffer, "(");
    itoa(lineNumber, tempBuffer, 10);
    strcat(outputBuffer, tempBuffer);
    strcat(outputBuffer, "): ");
    strcat(outputBuffer, text);
    Serial.println(outputBuffer);
}

#endif