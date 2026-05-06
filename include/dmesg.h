#ifndef DMESG_H
#define DMESG_H

#include "config.h"

// Structure for kernel messages
typedef struct {
  unsigned long timestamp;
  char message[DMESG_LEN];
} DmesgEntry;

// Global Dmesg variables
extern DmesgEntry dmesg[DMESG_LINES];
extern int dmesgIndex;

// Dmesg functions
void addDmesg(const __FlashStringHelper* msg);
void addDmesgRam(const char* msg);
void addDmesgTagged(const char* tag, const char* msg);
void printDmesg();
void clearDmesg();

#endif
