#include <Arduino.h>
#include <string.h>
#include "filesystem.h"
#include "dmesg.h"

// Global filesystem variables
RAMFile fs[MAX_FILES];
char currentPath[PATH_LEN] = "/";

// Initialize the filesystem with directories and devices
void initFS() {
  int d, i;

  // Create root directories: /home and /dev
  const char* dirs[] = {"home", "dev"};
  for (d = 0; d < 2; d++) {
    for (i = 0; i < MAX_FILES; i++) {
      if (!fs[i].active) {
        strncpy(fs[i].name, dirs[d], NAME_LEN - 1);
        fs[i].name[NAME_LEN - 1] = '\0';
        strncpy(fs[i].parentDir, "/", PATH_LEN - 1);
        fs[i].parentDir[PATH_LEN - 1] = '\0';
        fs[i].isDirectory = 1;
        fs[i].active = 1;
        break;
      }
    }
  }

  // Create pin devices: /dev/pin2, /dev/pin3, /dev/pin4
  char devPath[PATH_LEN] = "/dev/";
  const char* pins[] = {"pin2", "pin3", "pin4"};
  for (d = 0; d < 3; d++) {
    for (i = 0; i < MAX_FILES; i++) {
      if (!fs[i].active) {
        strncpy(fs[i].name, pins[d], NAME_LEN - 1);
        fs[i].name[NAME_LEN - 1] = '\0';
        strncpy(fs[i].parentDir, devPath, PATH_LEN - 1);
        fs[i].parentDir[PATH_LEN - 1] = '\0';
        fs[i].isDirectory = 0;
        fs[i].content[0] = '\0';
        fs[i].active = 1;
        break;
      }
    }
  }

  addDmesg(F("Filesystem initialized"));
}

// Search for a file in the filesystem
RAMFile* findFile(const char* name, const char* parentDir) {
  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active && 
        strcmp(fs[i].name, name) == 0 && 
        strcmp(fs[i].parentDir, parentDir) == 0) {
      return &fs[i];
    }
  }
  return NULL;
}

// Create a file or directory
int createFile(const char* name, const char* parentDir, int isDirectory) {
  if (findFile(name, parentDir)) {
    addDmesgRam("Error: File already exists");
    return 0; // File already exists
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
      return 1; // Success
    }
  }

  addDmesgRam("Error: Filesystem full");
  return 0; // Filesystem full
}

// Delete a file
int deleteFile(const char* name, const char* parentDir) {
  RAMFile* file = findFile(name, parentDir);
  if (file) {
    file->active = 0;
    return 1; // Success
  }
  addDmesgRam("Error: File not found");
  return 0; // File not found
}

// List files in a directory
void listFiles(const char* parentDir) {
  int found = 0;
  for (int i = 0; i < MAX_FILES; i++) {
    if (fs[i].active && strcmp(fs[i].parentDir, parentDir) == 0) {
      if (fs[i].isDirectory) {
        Serial.print("[DIR] ");
      } else {
        Serial.print("[FILE] ");
      }
      Serial.println(fs[i].name);
      found = 1;
    }
  }
  if (!found) {
    Serial.println("(empty directory)");
  }
}

// Change directory
int changeDirectory(const char* dirName) {
  RAMFile* file = findFile(dirName, currentPath);
  
  if (!file) {
    addDmesgRam("Error: Directory not found");
    return 0;
  }

  if (!file->isDirectory) {
    addDmesgRam("Error: Not a directory");
    return 0;
  }

  strncpy(currentPath, dirName, PATH_LEN - 1);
  currentPath[PATH_LEN - 1] = '\0';
  return 1;
}

// Get the full path
char* getFullPath(const char* dirName) {
  static char fullPath[PATH_LEN];
  strncpy(fullPath, currentPath, PATH_LEN - 1);
  fullPath[PATH_LEN - 1] = '\0';
  return fullPath;
}
