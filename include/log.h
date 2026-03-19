#pragma once
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include "build_config.h"

/*
 * Ultra-Lightweight Logger for ESP32
 * 
 * Memory optimization techniques:
 * - All strings stored in FLASH (F() macro, PSTR())
 * - Compile-time elimination via MF_LOG_LEVEL
 * - Zero-cost when disabled (empty macros)
 * - No dynamic memory allocation
 * - Inlined functions marked with __attribute__((always_inline))
 * - Optional module name prefix (FLASH stored)
 */

// Log levels - compile time filtering
#define MF_LOG_LEVEL_NONE  0
#define MF_LOG_LEVEL_FATAL 1
#define MF_LOG_LEVEL_ERROR 2
#define MF_LOG_LEVEL_WARN  3
#define MF_LOG_LEVEL_INFO  4
#define MF_LOG_LEVEL_DEBUG 5
#define MF_LOG_LEVEL_TRACE 6

// Default log level if not defined
#ifndef MF_LOG_LEVEL
  #if defined(NDEBUG)
    #define MF_LOG_LEVEL MF_LOG_LEVEL_INFO
  #else
    #define MF_LOG_LEVEL MF_LOG_LEVEL_DEBUG
  #endif
#endif

// Size limits to prevent stack overflow
#ifndef MF_LOG_MAX_MESSAGE_LEN
  #define MF_LOG_MAX_MESSAGE_LEN 128
#endif

// Enable/disable serial output (from build_config.h or default)
#ifndef MF_ENABLE_SERIAL_LOG
  #define MF_ENABLE_SERIAL_LOG 0
#endif

// Enable/disable module name prefix (saves ~4-8 bytes per log if disabled)
#ifndef MF_LOG_SHOW_MODULE
  #define MF_LOG_SHOW_MODULE 1
#endif

// Enable/disable line numbers (saves ~4 bytes per log if disabled)  
#ifndef MF_LOG_SHOW_LINE
  #define MF_LOG_SHOW_LINE 1
#endif

// ============================================================
// INTERNAL IMPLEMENTATION - Do not use directly
// ============================================================

namespace LogImpl {
    // Flash-stored level strings - only referenced, never copied to RAM
    static const char LVL_FATAL[] PROGMEM = "F";
    static const char LVL_ERROR[] PROGMEM = "E";
    static const char LVL_WARN[]  PROGMEM = "W";
    static const char LVL_INFO[]  PROGMEM = "I";
    static const char LVL_DEBUG[] PROGMEM = "D";
    static const char LVL_TRACE[] PROGMEM = "T";
    
    static const char SEP_COLON[] PROGMEM = ":";
    static const char SEP_PIPE[]  PROGMEM = "|";
    #if MF_LOG_SHOW_LINE
    static const char SEP_LINE[] PROGMEM = ":";
    #endif

    // Minimal PGM read helper
    inline char readChar(const char* pgmPtr, size_t idx) {
        return pgm_read_byte(pgmPtr + idx);
    }

    // Print a PROGMEM string directly to Serial (no RAM buffer)
    inline void printP(const char* pgmStr) {
        char c;
        while ((c = pgm_read_byte(pgmStr++))) {
            Serial.write(c);
        }
    }

    // Fast path: minimal overhead serial writer
    struct SerialOut {
        static inline void begin() __attribute__((always_inline)) {
            #if MF_ENABLE_SERIAL_LOG
            if (!Serial) Serial.begin(9600);
            #endif
        }
        
        static inline void write(char c) __attribute__((always_inline)) {
            #if MF_ENABLE_SERIAL_LOG
            Serial.write(c);
            #endif
        }
        
        static inline void print(const char* s) __attribute__((always_inline)) {
            #if MF_ENABLE_SERIAL_LOG
            while (*s) write(*s++);
            #endif
        }
        
        static inline void printP(const char* pgm) __attribute__((always_inline)) {
            #if MF_ENABLE_SERIAL_LOG
            char c;
            while ((c = pgm_read_byte(pgm++))) write(c);
            #endif
        }
        
        static inline void println() __attribute__((always_inline)) {
            #if MF_ENABLE_SERIAL_LOG
            Serial.println();
            #endif
        }
    };

    // Format and print variadic message using snprintf_P (flash format string)
    template<typename... Args>
    inline void logFormatted(const char* formatPgm, Args... args) {
        #if MF_ENABLE_SERIAL_LOG
        char buf[MF_LOG_MAX_MESSAGE_LEN];
        snprintf_P(buf, sizeof(buf), (const char*)pgm_read_ptr(&formatPgm), args...);
        print(buf);
        #endif
    }

    // Overload for plain string (no formatting)
    inline void logPlain(const char* msg) {
        #if MF_ENABLE_SERIAL_LOG
        print(msg);
        #endif
    }

    // Module name extractor - extracts filename from __FILE__ at compile time
    // This keeps full paths out of the binary
    template<size_t N>
    constexpr const char* extractModuleName(const char (&path)[N], size_t pos = N - 1) {
        return (pos == 0 || path[pos-1] == '/' || path[pos-1] == '\\') 
               ? &path[pos] 
               : extractModuleName(path, pos - 1);
    }
}

// ============================================================
// PUBLIC API - Use these macros
// ============================================================

// Setup logging (auto-called on first use, but can be explicit)
#define LOG_INIT() LogImpl::SerialOut::begin()

// Raw log output (no level prefix, no module) - always available if logging enabled
#if MF_ENABLE_SERIAL_LOG
    #define LOG_RAW(msg) do { LogImpl::SerialOut::print(msg); } while(0)
    #define LOG_RAW_LN(msg) do { LogImpl::SerialOut::print(msg); LogImpl::SerialOut::println(); } while(0)
#else
    #define LOG_RAW(msg) ((void)0)
    #define LOG_RAW_LN(msg) ((void)0)
#endif

// ============================================================
// LEVEL-SPECIFIC LOG MACROS (with compile-time filtering)
// ============================================================

// TRACE level - most verbose
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_TRACE && MF_ENABLE_SERIAL_LOG
    #define LOG_TRACE(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_TRACE); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_TRACE_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_TRACE); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_TRACE(module, ...) ((void)0)
    #define LOG_TRACE_RAW(msg) ((void)0)
#endif

// DEBUG level
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_DEBUG && MF_ENABLE_SERIAL_LOG
    #define LOG_DEBUG(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_DEBUG); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_DEBUG_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_DEBUG); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_DEBUG(module, ...) ((void)0)
    #define LOG_DEBUG_RAW(msg) ((void)0)
#endif

// INFO level
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_INFO && MF_ENABLE_SERIAL_LOG
    #define LOG_INFO(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_INFO); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_INFO_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_INFO); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_INFO(module, ...) ((void)0)
    #define LOG_INFO_RAW(msg) ((void)0)
#endif

// WARN level
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_WARN && MF_ENABLE_SERIAL_LOG
    #define LOG_WARN(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_WARN); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_WARN_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_WARN); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_WARN(module, ...) ((void)0)
    #define LOG_WARN_RAW(msg) ((void)0)
#endif

// ERROR level
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_ERROR && MF_ENABLE_SERIAL_LOG
    #define LOG_ERROR(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_ERROR); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_ERROR_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_ERROR); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_ERROR(module, ...) ((void)0)
    #define LOG_ERROR_RAW(msg) ((void)0)
#endif

// FATAL level - always compiled in if any logging enabled
#if MF_LOG_LEVEL >= MF_LOG_LEVEL_FATAL && MF_ENABLE_SERIAL_LOG
    #define LOG_FATAL(module, ...) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_FATAL); \
        LogImpl::SerialOut::write(' '); \
        LOG_PRINT_LOCATION(module); \
        LogImpl::logFormatted(PSTR(__VA_ARGS__)); \
        LogImpl::SerialOut::println(); \
    } while(0)
    #define LOG_FATAL_RAW(msg) do { \
        LogImpl::SerialOut::printP(LogImpl::LVL_FATAL); \
        LogImpl::SerialOut::write(' '); \
        LogImpl::SerialOut::print(msg); \
        LogImpl::SerialOut::println(); \
    } while(0)
#else
    #define LOG_FATAL(module, ...) ((void)0)
    #define LOG_FATAL_RAW(msg) ((void)0)
#endif

// ============================================================
// INTERNAL HELPERS
// ============================================================

// Print location: "Module:line" or just "Module" based on config
#if MF_LOG_SHOW_MODULE && MF_LOG_SHOW_LINE
    #define LOG_PRINT_LOCATION(mod) do { \
        LogImpl::SerialOut::print(mod); \
        LogImpl::SerialOut::printP(LogImpl::SEP_LINE); \
        LogImpl::SerialOut::print(__LINE__); \
        LogImpl::SerialOut::printP(LogImpl::SEP_COLON); \
        LogImpl::SerialOut::write(' '); \
    } while(0)
#elif MF_LOG_SHOW_MODULE
    #define LOG_PRINT_LOCATION(mod) do { \
        LogImpl::SerialOut::print(mod); \
        LogImpl::SerialOut::printP(LogImpl::SEP_COLON); \
        LogImpl::SerialOut::write(' '); \
    } while(0)
#elif MF_LOG_SHOW_LINE
    #define LOG_PRINT_LOCATION(mod) do { \
        LogImpl::SerialOut::print(__LINE__); \
        LogImpl::SerialOut::printP(LogImpl::SEP_COLON); \
        LogImpl::SerialOut::write(' '); \
    } while(0)
#else
    #define LOG_PRINT_LOCATION(mod) ((void)0)
#endif

// ============================================================
// CONVENIENCE SHORTCUTS
// ============================================================

// Auto-module extraction macros - use these for automatic module naming
// Example: LOGD("Sensor value: %d", sensorValue);
#define LOGT(...) LOG_TRACE(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)
#define LOGD(...) LOG_DEBUG(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)
#define LOGI(...) LOG_INFO(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)
#define LOGW(...) LOG_WARN(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)
#define LOGE(...) LOG_ERROR(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)
#define LOGF(...) LOG_FATAL(LogImpl::extractModuleName(__FILE__), __VA_ARGS__)

// ============================================================
// LEGACY COMPATIBILITY (keep old interface working)
// ============================================================

typedef enum : uint8_t {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
} level;

static constexpr bool _debug = false;

#if MF_ENABLE_SERIAL_LOG

inline void setupLog(void) {
    LogImpl::SerialOut::begin();
}

// Legacy LOG_S - maps to new macros for efficiency
inline void LOG_S(level lvl, const char *stringFrom, int lineNumber, const char *text) {
    #if MF_ENABLE_SERIAL_LOG
    switch(lvl) {
        case DEBUG: LogImpl::SerialOut::printP(LogImpl::LVL_DEBUG); break;
        case INFO:  LogImpl::SerialOut::printP(LogImpl::LVL_INFO); break;
        case WARN:  LogImpl::SerialOut::printP(LogImpl::LVL_WARN); break;
        case ERROR: LogImpl::SerialOut::printP(LogImpl::LVL_ERROR); break;
        case FATAL: LogImpl::SerialOut::printP(LogImpl::LVL_FATAL); break;
        default:    LogImpl::SerialOut::write('?'); break;
    }
    LogImpl::SerialOut::write(' ');
    LogImpl::SerialOut::print(stringFrom);
    LogImpl::SerialOut::write(':');
    LogImpl::SerialOut::print(lineNumber);
    LogImpl::SerialOut::write(' ');
    LogImpl::SerialOut::print(text);
    LogImpl::SerialOut::println();
    #endif
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

#endif // LOG_H
