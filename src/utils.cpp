#include <Arduino.h>
#include "utils.h"
#include "board.h"

// Get available free memory
int freeMemory() {
  return boardFreeMemory();
}

// Restart the current board
void resetSystem() {
  boardRestart();
}

// Convert string to integer
int stringToInt(const char* str) {
  int result = 0;
  int sign = 1;
  int i = 0;

  if (str[0] == '-') {
    sign = -1;
    i = 1;
  }

  while (str[i] >= '0' && str[i] <= '9') {
    result = result * 10 + (str[i] - '0');
    i++;
  }

  return result * sign;
}

// Convert integer to string
void intToString(int value, char* buffer, int bufferSize) {
  if (bufferSize <= 0) return;

  int index = 0;
  int num = value;
  int digits = 0;

  if (value < 0) {
    buffer[0] = '-';
    index = 1;
    num = -value;
  }

  // Count digits
  int temp = num;
  if (temp == 0) {
    digits = 1;
  } else {
    while (temp > 0) {
      digits++;
      temp /= 10;
    }
  }

  if (index + digits >= bufferSize) {
    buffer[0] = '\0';
    return;
  }

  // Fill from the end
  if (num == 0) {
    buffer[index] = '0';
    buffer[index + 1] = '\0';
  } else {
    buffer[index + digits] = '\0';
    while (num > 0) {
      buffer[index + digits - 1] = (num % 10) + '0';
      num /= 10;
      digits--;
    }
  }
}
