#include <Arduino.h>
#include "board.h"
#include "compat.h"

#if BOARD_ARCH_ESP32
#include <esp_system.h>
#include <esp_heap_caps.h>
#endif

namespace {

#if BOARD_ARCH_AVR
const uint8_t demoPinsData[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
#elif BOARD_ARCH_ESP32
const uint8_t demoPinsData[] = {4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
#else
const uint8_t demoPinsData[] = {2, 3, 4, 5};
#endif

}

void boardInit() {}

int boardFreeMemory() {
#if BOARD_ARCH_AVR
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#elif BOARD_ARCH_ESP32
  return (int) heap_caps_get_free_size(MALLOC_CAP_8BIT);
#else
  return -1;
#endif
}

void boardRestart() {
#if BOARD_ARCH_AVR
  void(* resetSystem) (void) = 0;
  resetSystem();
#elif BOARD_ARCH_ESP32
  esp_restart();
#endif
}

void boardWritePwm(uint8_t pin, int value) {
#if BOARD_ARCH_ESP32
  analogWrite(pin, value);
#else
  analogWrite(pin, value);
#endif
}

const char* boardHardwareName() {
#if BOARD_ARCH_AVR
  return "Arduino UNO";
#elif BOARD_ARCH_ESP32
  return "ESP32";
#else
  return "Unknown";
#endif
}

const char* boardArchName() {
#if BOARD_ARCH_AVR
  return "AVR";
#elif BOARD_ARCH_ESP32
  return "ESP32";
#else
  return "Unknown";
#endif
}

const uint8_t* boardDemoPins(size_t* count) {
  if (count != NULL) {
    *count = sizeof(demoPinsData) / sizeof(demoPinsData[0]);
  }
  return demoPinsData;
}
