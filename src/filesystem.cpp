#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "filesystem.h"
#include "board.h"
#include "dmesg.h"

RAMFile fs[MAX_FILES];
char currentPath[PATH_LEN] = "/";

namespace {

void clearEntry(RAMFile* entry) {
  entry->name[0] = '\0';
  entry->content[0] = '\0';
  entry->parentDir[0] = '\0';
  entry->isDirectory = 0;
  entry->active = 0;
}

int isValidEntryName(const char* name) {
  if (name == NULL || name[0] == '\0') {
    return 0;
  }

  if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
    return 0;
  }

  if (strlen(name) >= NAME_LEN) {
    return 0;
  }

  for (int i = 0; name[i] != '\0'; i++) {
    if (name[i] == '/') {
      return 0;
    }
  }

  return 1;
}

int buildChildPath(const char* parentDir, const char* name, char* output) {
  if (strcmp(parentDir, "/") == 0) {
    return snprintf(output, PATH_LEN, "/%s/", name) < PATH_LEN;
  }

  return snprintf(output, PATH_LEN, "%s%s/", parentDir, name) < PATH_LEN;
}

void createInitialEntry(const char* name, const char* parentDir, int isDirectory) {
  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs[i].active) {
      strncpy(fs[i].name, name, NAME_LEN - 1);
      fs[i].name[NAME_LEN - 1] = '\0';
      strncpy(fs[i].parentDir, parentDir, PATH_LEN - 1);
      fs[i].parentDir[PATH_LEN - 1] = '\0';
      fs[i].content[0] = '\0';
      fs[i].isDirectory = isDirectory;
      fs[i].active = 1;
      return;
    }
  }
}

void goToParentPath(char* path) {
  int length = strlen(path);

  if (length <= 1) {
    strncpy(path, "/", PATH_LEN - 1);
    path[PATH_LEN - 1] = '\0';
    return;
  }

  if (path[length - 1] == '/') {
    path[length - 1] = '\0';
    length--;
  }

  while (length > 0 && path[length - 1] != '/') {
    path[length - 1] = '\0';
    length--;
  }

  if (length <= 1) {
    strncpy(path, "/", PATH_LEN - 1);
    path[PATH_LEN - 1] = '\0';
  }
}

}

// Initialize filesystem with root directories and device files
void initFS() {
  for (int i = 0; i < MAX_FILES; i++) {
    clearEntry(&fs[i]);
  }

  strncpy(currentPath, "/", PATH_LEN - 1);
  currentPath[PATH_LEN - 1] = '\0';

  createInitialEntry("home", "/", 1);
  createInitialEntry("dev", "/", 1);

  size_t demoPinCount = 0;
  const uint8_t* demoPins = boardDemoPins(&demoPinCount);
  int selectedPins[3];
  int selectedCount = 0;
  int ledPin = boardLedPin();

  if (ledPin >= 0) {
    selectedPins[selectedCount++] = ledPin;
  }

  for (size_t i = 0; i < demoPinCount && selectedCount < 3; i++) {
    int duplicate = 0;

    for (int j = 0; j < selectedCount; j++) {
      if (selectedPins[j] == demoPins[i]) {
        duplicate = 1;
        break;
      }
    }

    if (!duplicate) {
      selectedPins[selectedCount++] = demoPins[i];
    }
  }

  for (int i = 0; i < selectedCount; i++) {
    char deviceName[NAME_LEN];
    snprintf(deviceName, sizeof(deviceName), "pin%d", selectedPins[i]);
    createInitialEntry(deviceName, "/dev/", 0);
  }

  addDmesg(F("Filesystem initialized"));
}

const char* fsCurrentPath() {
  return currentPath;
}

// Search for a file in the filesystem by name and parent directory
RAMFile* findFile(const char* name, const char* parentDir) {
  if (name == NULL || parentDir == NULL) {
    return NULL;
  }

  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active &&
        strcmp(fs[i].name, name) == 0 &&
        strcmp(fs[i].parentDir, parentDir) == 0) {
      return &fs[i];
    }
  }

  return NULL;
}

// Create a new file or directory entry
int createFile(const char* name, const char* parentDir, int isDirectory) {
  if (!isValidEntryName(name) || parentDir == NULL) {
    return 0;
  }

  if (findFile(name, parentDir) != NULL) {
    return 0;
  }

  for (int i = 0; i < MAX_FILES; i++) {
    if (!fs[i].active) {
      strncpy(fs[i].name, name, NAME_LEN - 1);
      fs[i].name[NAME_LEN - 1] = '\0';
      strncpy(fs[i].parentDir, parentDir, PATH_LEN - 1);
      fs[i].parentDir[PATH_LEN - 1] = '\0';
      fs[i].isDirectory = isDirectory;
      fs[i].content[0] = '\0';
      fs[i].active = 1;
      return 1;
    }
  }

  return 0;
}

// Delete a single file or empty directory
int deleteFile(const char* name, const char* parentDir) {
  RAMFile* file = findFile(name, parentDir);
  if (file == NULL) {
    return 0;
  }

  clearEntry(file);
  return 1;
}

// Delete a directory and all its contents recursively
int deleteFileRecursive(const char* name, const char* parentDir) {
  RAMFile* file = findFile(name, parentDir);
  if (file == NULL) {
    return 0;
  }

  if (file->isDirectory) {
    char dirPath[PATH_LEN];
    if (!buildChildPath(parentDir, name, dirPath)) {
      return 0;
    }

    for (int i = 0; i < MAX_FILES; i++) {
      if (fs[i].active && strcmp(fs[i].parentDir, dirPath) == 0) {
        deleteFileRecursive(fs[i].name, dirPath);
      }
    }
  }

  clearEntry(file);
  return 1;
}

// Write content to a file
int writeFileContent(const char* name, const char* parentDir, const char* content) {
  RAMFile* file = findFile(name, parentDir);
  if (file == NULL || file->isDirectory) {
    return 0;
  }

  strncpy(file->content, content, CONTENT_LEN - 1);
  file->content[CONTENT_LEN - 1] = '\0';
  return 1;
}

// Read content from a file
const char* readFileContent(const char* name, const char* parentDir) {
  RAMFile* file = findFile(name, parentDir);
  if (file == NULL || file->isDirectory) {
    return NULL;
  }

  return file->content;
}

// Count active filesystem slots in use
int usedFileSlots() {
  int used = 0;

  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active) {
      used++;
    }
  }

  return used;
}

int printFindMatches(const char* name) {
  int found = 0;

  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active && strcmp(fs[i].name, name) == 0) {
      Serial.print(fs[i].parentDir);
      Serial.println(fs[i].name);
      found = 1;
    }
  }

  return found;
}

void listFiles(const char* parentDir) {
  int found = 0;

  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active && strcmp(fs[i].parentDir, parentDir) == 0) {
      if (fs[i].isDirectory) {
        Serial.print(F("[DIR] "));
      } else {
        Serial.print(F("[FILE] "));
      }
      Serial.println(fs[i].name);
      found = 1;
    }
  }

  if (!found) {
    Serial.println(F("(empty directory)"));
  }
}

void listFilesCompact(const char* parentDir) {
  int found = 0;

  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active && strcmp(fs[i].parentDir, parentDir) == 0) {
      Serial.print(fs[i].name);
      if (fs[i].isDirectory) {
        Serial.print(F("/"));
      }
      Serial.print(F("  "));
      found = 1;
    }
  }

  if (!found) {
    Serial.print(F("(empty)"));
  }

  Serial.println();
}

int changeDirectory(const char* dirName) {
  if (dirName == NULL || dirName[0] == '\0') {
    return 0;
  }

  if (strcmp(dirName, "/") == 0) {
    strncpy(currentPath, "/", PATH_LEN - 1);
    currentPath[PATH_LEN - 1] = '\0';
    return 1;
  }

  if (strcmp(dirName, "..") == 0) {
    goToParentPath(currentPath);
    return 1;
  }

  RAMFile* file = findFile(dirName, currentPath);
  if (file == NULL || !file->isDirectory) {
    return 0;
  }

  char nextPath[PATH_LEN];
  if (!buildChildPath(currentPath, dirName, nextPath)) {
    return 0;
  }

  strncpy(currentPath, nextPath, PATH_LEN - 1);
  currentPath[PATH_LEN - 1] = '\0';
  return 1;
}

char* getFullPath(const char* dirName) {
  static char fullPath[PATH_LEN];

  if (dirName == NULL || dirName[0] == '\0') {
    strncpy(fullPath, currentPath, PATH_LEN - 1);
    fullPath[PATH_LEN - 1] = '\0';
    return fullPath;
  }

  if (!buildChildPath(currentPath, dirName, fullPath)) {
    fullPath[0] = '\0';
  }

  return fullPath;
}
