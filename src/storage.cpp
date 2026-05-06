#include "storage.h"

#include <string.h>

#include "aliases.h"
#include "compat.h"
#include "filesystem.h"

#if BOARD_ARCH_RP2040
#include <EEPROM.h>
#endif

#if BOARD_ARCH_ESP32
#include <Preferences.h>
#endif

namespace {

const unsigned long STORAGE_MAGIC = 0x424F5332UL;
const unsigned short STORAGE_VERSION = 2;

struct PersistentState {
  unsigned long magic;
  unsigned short version;
  unsigned short reserved;
  char savedPath[PATH_LEN];
  RAMFile files[MAX_FILES];
  AliasEntry aliasTable[MAX_ALIASES];
};

PersistentState stateBuffer;

#if BOARD_ARCH_ESP32
Preferences preferences;
const char* preferencesNamespace = "boardos";
const char* preferencesKey = "state";
#endif

void captureState() {
  stateBuffer.magic = STORAGE_MAGIC;
  stateBuffer.version = STORAGE_VERSION;
  stateBuffer.reserved = 0;

  strncpy(stateBuffer.savedPath, currentPath, PATH_LEN - 1);
  stateBuffer.savedPath[PATH_LEN - 1] = '\0';

  memcpy(stateBuffer.files, fs, sizeof(fs));
  memcpy(stateBuffer.aliasTable, aliases, sizeof(aliases));
}

void applyState() {
  memcpy(fs, stateBuffer.files, sizeof(fs));
  memcpy(aliases, stateBuffer.aliasTable, sizeof(aliases));
  strncpy(currentPath, stateBuffer.savedPath, PATH_LEN - 1);
  currentPath[PATH_LEN - 1] = '\0';

  if (currentPath[0] == '\0') {
    strncpy(currentPath, "/", PATH_LEN - 1);
    currentPath[PATH_LEN - 1] = '\0';
  }
}

int isStateValid() {
  return stateBuffer.magic == STORAGE_MAGIC && stateBuffer.version == STORAGE_VERSION;
}

}

// Initialize persistent storage backend (EEPROM/Preferences)
void storageInit() {
#if BOARD_ARCH_RP2040
  EEPROM.begin(sizeof(PersistentState));
#endif
}

// Load saved filesystem and alias state from persistent storage
int storageLoadState() {
#if BOARD_ARCH_RP2040
  EEPROM.get(0, stateBuffer);
  if (!isStateValid()) {
    return 0;
  }
  applyState();
  return 1;
#elif BOARD_ARCH_ESP32
  if (!preferences.begin(preferencesNamespace, true)) {
    return 0;
  }

  size_t length = preferences.getBytesLength(preferencesKey);
  if (length != sizeof(PersistentState)) {
    preferences.end();
    return 0;
  }

  if (preferences.getBytes(preferencesKey, &stateBuffer, sizeof(PersistentState)) != sizeof(PersistentState)) {
    preferences.end();
    return 0;
  }

  preferences.end();
  if (!isStateValid()) {
    return 0;
  }

  applyState();
  return 1;
#else
  return 0;
#endif
}

// Save current filesystem and alias state to persistent storage
int storageSaveState() {
  captureState();

#if BOARD_ARCH_RP2040
  EEPROM.put(0, stateBuffer);
  return EEPROM.commit();
#elif BOARD_ARCH_ESP32
  if (!preferences.begin(preferencesNamespace, false)) {
    return 0;
  }

  size_t written = preferences.putBytes(preferencesKey, &stateBuffer, sizeof(PersistentState));
  preferences.end();
  return written == sizeof(PersistentState);
#else
  return 0;
#endif
}

// Clear saved state from persistent storage
void storageResetState() {
  memset(&stateBuffer, 0, sizeof(stateBuffer));

#if BOARD_ARCH_RP2040
  EEPROM.put(0, stateBuffer);
  EEPROM.commit();
#elif BOARD_ARCH_ESP32
  if (preferences.begin(preferencesNamespace, false)) {
    preferences.remove(preferencesKey);
    preferences.end();
  }
#endif
}
