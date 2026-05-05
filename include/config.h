#ifndef CONFIG_H
#define CONFIG_H

#include "compat.h"

// ============================================
// SYSTEM CONFIGURATION - BoardOS
// ============================================

// System identity
#define BOARD_OS_NAME "BoardOS"
#define BOARD_OS_VERSION "1.0.6"

#if BOARD_ARCH_AVR
#define BOARD_HARDWARE "Arduino UNO"
#define BOARD_ARCH BOARD_ARCH_NAME
#elif BOARD_ARCH_ESP32
#define BOARD_HARDWARE "ESP32"
#define BOARD_ARCH BOARD_ARCH_NAME
#else
#define BOARD_HARDWARE "Unknown"
#define BOARD_ARCH BOARD_ARCH_NAME
#endif

// Memory configuration - Filesystem
#define MAX_FILES 10         
#define NAME_LEN 12         
#define CONTENT_LEN 32      
#define PATH_LEN 16         

// Dmesg configuration (kernel logs)
#define DMESG_LINES 6
#define DMESG_LEN 40

// Alias configuration
#define MAX_ALIASES 4
#define ALIAS_NAME_LEN 6
#define ALIAS_VAL_LEN 20

// Input buffer configuration
#define INPUT_BUFFER_SIZE 32

#endif
