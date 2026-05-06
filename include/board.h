#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>

void boardInit();
int boardFreeMemory();
void boardRestart();
void boardWritePwm(uint8_t pin, int value);
const char* boardHardwareName();
const char* boardArchName();
int boardLedPin();
const uint8_t* boardDemoPins(size_t* count);

#endif
