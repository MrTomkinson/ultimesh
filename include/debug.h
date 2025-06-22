#pragma once

#include <Arduino.h>
#include "common_config.h"

/**
 * Convenience macros for guarded debug printing.
 * Use DLOG(fmt, ...) in place of Serial.printf(...)
 * and DPRINT/DPRINTLN for Serial.print/println.
 */

#define DLOG(fmt, ...)                           \
  do {                                           \
    if (getConfigBool("debug_enabled", false)) { \
      Serial.printf((fmt), ##__VA_ARGS__);       \
    }                                            \
  } while (0)

#define DPRINT(x)                                \
  do {                                           \
    if (getConfigBool("debug_enabled", false)) { \
      Serial.print(x);                           \
    }                                            \
  } while (0)

#define DPRINTLN(x)                              \
  do {                                           \
    if (getConfigBool("debug_enabled", false)) { \
      Serial.println(x);                         \
    }                                            \
  } while (0)
