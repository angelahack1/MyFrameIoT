# Ultra-Lightweight Logger for ESP32

A memory-optimized logging system designed for ESP32 with extremely constrained RAM.

## Features

- **Zero-cost when disabled** - All logging code eliminated at compile time
- **Flash-stored strings** - No RAM usage for log messages
- **Compile-time filtering** - Unwanted levels removed completely
- **No dynamic allocation** - Stack-only operation
- **Variadic formatting** - printf-style formatting support
- **Module prefixes** - Automatic filename extraction
- **Serial debounce** - Safe initialization avoiding bootloader collision

## Quick Start

```cpp
#include "log.h"

void setup() {
    // Initialize with debounce (100ms delay to avoid bootloader collision)
    LOG_INIT();
    
    // Log messages at different levels
    LOGT("Trace: entering function");           // TRACE level
    LOGD("Debug: value=%d", sensorValue);       // DEBUG level  
    LOGI("Info: System started");               // INFO level
    LOGW("Warn: Low memory: %d bytes", free);   // WARN level
    LOGE("Error: Failed to connect");           // ERROR level
    LOGF("Fatal: Out of memory!");              // FATAL level
}
```

## Configuration

Edit `build_config.h`:

```cpp
#define MF_ENABLE_SERIAL_LOG 1              // Enable logging
#define MF_LOG_LEVEL MF_LOG_LEVEL_DEBUG     // Show DEBUG and above
```

### Log Levels (compile-time filter)

| Level  | Value | Description |
|--------|-------|-------------|
| TRACE  | 6     | Most verbose, function entry/exit |
| DEBUG  | 5     | Development diagnostics |
| INFO   | 4     | Normal operation events |
| WARN   | 3     | Potential issues |
| ERROR  | 2     | Recoverable errors |
| FATAL  | 1     | Critical errors |
| NONE   | 0     | Logging disabled |

### Memory Tuning

```cpp
#define MF_LOG_MAX_MESSAGE_LEN 64     // Smaller buffer (default: 128)
#define MF_LOG_SHOW_MODULE 0          // Hide module names (saves ~4-8 bytes/log)
#define MF_LOG_SHOW_LINE 0            // Hide line numbers (saves ~4 bytes/log)
```

## Memory Usage Comparison

| Configuration | Flash | RAM | Notes |
|--------------|-------|-----|-------|
| Logging disabled | 0 B | 0 B | All code removed |
| FATAL only | ~200 B | ~32 B | Minimal strings |
| INFO+ (prod) | ~800 B | ~64 B | Recommended |
| DEBUG+ (dev) | ~1200 B | ~128 B | Full debugging |
| TRACE (verbose) | ~1500 B | ~128 B | Everything |

## Advanced Usage

### Custom Module Names

```cpp
static const char MODULE[] PROGMEM = "SENSOR";
LOG_DEBUG(MODULE, "Reading: %d", value);
```

### Raw Output (no formatting)

```cpp
LOG_INFO_RAW("Static message without format");
LOG_RAW("No prefix, no newline");
LOG_RAW_LN("No prefix, with newline");
```

### Legacy Compatibility

Old-style logs still work:
```cpp
LOG_S(DEBUG, __FILE__, __LINE__, "Legacy message");
```

## Output Format

```
D main.cpp:42: Debug message with formatting
I main.cpp:50: Info: System ready
W main.cpp:55: Warn: Low battery: 15%
E main.cpp:60: Error: Connection timeout
```

Format: `LEVEL module:line: message`

## Serial Port Safety

The logger includes a 100ms debounce delay before Serial initialization to avoid
collision with ESP32 bootloader mode entry. This prevents programming issues.

## Tips for Maximum Memory Savings

1. **Disable in production**: Set `MF_ENABLE_SERIAL_LOG 0`
2. **Use higher log levels**: Set `MF_LOG_LEVEL MF_LOG_LEVEL_WARN`
3. **Reduce message buffer**: Set `MF_LOG_MAX_MESSAGE_LEN 64`
4. **Hide metadata**: Disable `MF_LOG_SHOW_MODULE` and `MF_LOG_SHOW_LINE`
5. **Use short module names**: Define custom 2-4 char module names
6. **Prefer raw logs**: `LOG_INFO_RAW()` skips sprintf formatting
