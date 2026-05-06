#include <Arduino.h>
#include "board.h"
#include "compat.h"

#if BOARD_ARCH_ESP32
#include <esp_system.h>
#include <esp_heap_caps.h>
#endif

#if BOARD_RP2040_HAS_HELPER_API
#include <RP2040.h>
#endif

namespace {

#if BOARD_ARCH_ESP32
const uint8_t demoPinsData[] = {2, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
#elif BOARD_ARCH_RP2040
const uint8_t demoPinsData[] = {25, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
#else
const uint8_t demoPinsData[] = {2, 3, 4, 5};
#endif

}

// Board-specific initialization (currently empty)
void boardInit() {}

// Query free memory available on the board
int boardFreeMemory() {
#if BOARD_ARCH_ESP32
  return (int) heap_caps_get_free_size(MALLOC_CAP_8BIT);
#elif BOARD_ARCH_RP2040
  #if BOARD_RP2040_HAS_HELPER_API
  return rp2040.getFreeHeap();
  #else
  return -1;
  #endif
#else
  return -1;
#endif
}

// Restart the board
void boardRestart() {
#if BOARD_ARCH_ESP32
  esp_restart();
#elif BOARD_ARCH_RP2040
  #if BOARD_RP2040_HAS_HELPER_API
  rp2040.reboot();
  #else
  NVIC_SystemReset();
  #endif
#endif
}

// Write PWM value to a pin
void boardWritePwm(uint8_t pin, int value) {
#if BOARD_ARCH_ESP32
  analogWrite(pin, value);
#else
  analogWrite(pin, value);
#endif
}

// Get hardware name (e.g., "ESP32", "Raspberry Pi Pico")
const char* boardHardwareName() {
#if BOARD_ARCH_ESP32
  return "ESP32";
#elif BOARD_ARCH_RP2040
  return "Raspberry Pi Pico";
#else
  return "Unknown";
#endif
}

// Get architecture name (e.g., "ESP32", "RP2040")
const char* boardArchName() {
#if BOARD_ARCH_ESP32
  return "ESP32";
#elif BOARD_ARCH_RP2040
  return "RP2040";
#else
  return "Unknown";
#endif
}

// Get the built-in LED pin number for this board
int boardLedPin() {
#if BOARD_ARCH_ESP32
  return 2;
#elif BOARD_ARCH_RP2040
  return 25;
#else
  return -1;
#endif
}

// Get list of available GPIO pins for demonstrations
const uint8_t* boardDemoPins(size_t* count) {
  if (count != NULL) {
    *count = sizeof(demoPinsData) / sizeof(demoPinsData[0]);
  }
  return demoPinsData;
}
