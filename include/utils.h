#ifndef UTILS_H
#define UTILS_H

// General utility functions

// Get available free memory
int freeMemory();

// Restart the current board
void resetSystem();

// Convert string to integer
int stringToInt(const char* str);

// Convert integer to string
void intToString(int value, char* buffer, int bufferSize);

#endif
