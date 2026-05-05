#include <Arduino.h>
#include <string.h>
#include "compat.h"
#include "dmesg.h"

// Global Dmesg variables
DmesgEntry dmesg[DMESG_LINES];
int dmesgIndex = 0;

// Add kernel message from FLASH memory
void addDmesg(const __FlashStringHelper* msg) {
  if (dmesgIndex >= DMESG_LINES) dmesgIndex = 0;
  dmesg[dmesgIndex].timestamp = millis() / 1000;
#if BOARD_ARCH_AVR
  strncpy_P(dmesg[dmesgIndex].message, (PGM_P)msg, DMESG_LEN - 1);
#else
  strncpy(dmesg[dmesgIndex].message, reinterpret_cast<const char*>(msg), DMESG_LEN - 1);
#endif
  dmesg[dmesgIndex].message[DMESG_LEN - 1] = '\0';
  dmesgIndex++;
}

// Add kernel message from RAM
void addDmesgRam(const char* msg) {
  if (dmesgIndex >= DMESG_LINES) dmesgIndex = 0;
  dmesg[dmesgIndex].timestamp = millis() / 1000;
  strncpy(dmesg[dmesgIndex].message, msg, DMESG_LEN - 1);
  dmesg[dmesgIndex].message[DMESG_LEN - 1] = '\0';
  dmesgIndex++;
}

// Show all kernel messages
void printDmesg() {
  Serial.println("\n=== KERNEL MESSAGES (DMESG) ===");
  for (int i = 0; i < DMESG_LINES; i++) {
    if (dmesg[i].message[0] != '\0') {
      Serial.print("[");
      Serial.print(dmesg[i].timestamp);
      Serial.print("s] ");
      Serial.println(dmesg[i].message);
    }
  }
  Serial.println("================================\n");
}

// Clear all kernel messages
void clearDmesg() {
  for (int i = 0; i < DMESG_LINES; i++) {
    dmesg[i].message[0] = '\0';
    dmesg[i].timestamp = 0;
  }
  dmesgIndex = 0;
  addDmesg(F("Dmesg cleared"));
}
