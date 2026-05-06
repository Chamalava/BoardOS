#ifndef COMPAT_H
#define COMPAT_H

#include <Arduino.h>
#include <string.h>

#if defined(ARDUINO_ARCH_AVR)
#define BOARD_ARCH_AVR 1
#define BOARD_ARCH_NAME "AVR"
#include <avr/pgmspace.h>
#elif defined(ARDUINO_ARCH_ESP32)
#define BOARD_ARCH_ESP32 1
#define BOARD_ARCH_NAME "ESP32"
#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(PICO_RP2040) || defined(TARGET_RP2040) || defined(TARGET_RASPBERRYPI)
#define BOARD_ARCH_RP2040 1
#define BOARD_ARCH_NAME "RP2040"
#else
#define BOARD_ARCH_UNKNOWN 1
#define BOARD_ARCH_NAME "Unknown"
#ifndef PSTR
#define PSTR(str) (str)
#endif

#ifndef strcmp_P
#define strcmp_P strcmp
#endif

#ifndef strncmp_P
#define strncmp_P strncmp
#endif

#ifndef strncpy_P
#define strncpy_P strncpy
#endif

#ifndef snprintf_P
#define snprintf_P snprintf
#endif
#endif

#ifndef BOARD_ARCH_AVR
#define BOARD_ARCH_AVR 0
#endif

#ifndef BOARD_ARCH_ESP32
#define BOARD_ARCH_ESP32 0
#endif

#ifndef BOARD_ARCH_RP2040
#define BOARD_ARCH_RP2040 0
#endif

#ifndef BOARD_ARCH_UNKNOWN
#define BOARD_ARCH_UNKNOWN 0
#endif

#if BOARD_ARCH_RP2040
#if defined(__has_include)
#if __has_include(<RP2040.h>)
#define BOARD_RP2040_HAS_HELPER_API 1
#else
#define BOARD_RP2040_HAS_HELPER_API 0
#endif
#else
#define BOARD_RP2040_HAS_HELPER_API 0
#endif
#else
#define BOARD_RP2040_HAS_HELPER_API 0
#endif

#endif
