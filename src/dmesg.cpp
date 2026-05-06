#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "compat.h"
#include "dmesg.h"

DmesgEntry dmesg[DMESG_LINES];
int dmesgIndex = 0;

namespace {

void writeDmesgMessage(const char* message) {
  if (dmesgIndex >= DMESG_LINES) {
    dmesgIndex = 0;
  }

  dmesg[dmesgIndex].timestamp = millis() / 1000;
  strncpy(dmesg[dmesgIndex].message, message, DMESG_LEN - 1);
  dmesg[dmesgIndex].message[DMESG_LEN - 1] = '\0';
  dmesgIndex++;
}

}

// Record a message from FLASH memory (saves RAM)
void addDmesg(const __FlashStringHelper* msg) {
  char buffer[DMESG_LEN];

#if BOARD_ARCH_AVR
  strncpy_P(buffer, (PGM_P)msg, DMESG_LEN - 1);
#else
  strncpy(buffer, reinterpret_cast<const char*>(msg), DMESG_LEN - 1);
#endif

  buffer[DMESG_LEN - 1] = '\0';
  writeDmesgMessage(buffer);
}

// Record a message from RAM
void addDmesgRam(const char* msg) {
  writeDmesgMessage(msg);
}

// Record a message with a tag prefix
void addDmesgTagged(const char* tag, const char* msg) {
  if (tag == NULL || tag[0] == '\0') {
    addDmesgRam(msg);
    return;
  }

  char buffer[DMESG_LEN];
  snprintf(buffer, sizeof(buffer), "%s: %s", tag, msg);
  writeDmesgMessage(buffer);
}

// Print all kernel messages to serial output
void printDmesg() {
  Serial.println(F("=== KERNEL MESSAGES ==="));
  int printed = 0;

  for (int i = 0; i < DMESG_LINES; i++) {
    int index = (dmesgIndex + i) % DMESG_LINES;
    if (dmesg[index].message[0] != '\0') {
      Serial.print(F("["));
      Serial.print(dmesg[index].timestamp);
      Serial.print(F("s] "));
      Serial.println(dmesg[index].message);
      printed = 1;
    }
  }

  if (!printed) {
    Serial.println(F("(empty)"));
  }
}

// Clear all kernel messages and reset the buffer
void clearDmesg() {
  for (int i = 0; i < DMESG_LINES; i++) {
    dmesg[i].message[0] = '\0';
    dmesg[i].timestamp = 0;
  }

  dmesgIndex = 0;
  addDmesg(F("Dmesg cleared"));
}
