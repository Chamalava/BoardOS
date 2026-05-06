#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "config.h"

// Structure for files in RAM
typedef struct {
  char name[NAME_LEN];
  char content[CONTENT_LEN];
  char parentDir[PATH_LEN];
  int isDirectory;
  int active;
} RAMFile;

// Global filesystem variables
extern RAMFile fs[MAX_FILES];
extern char currentPath[PATH_LEN];

// Filesystem functions
void initFS();
const char* fsCurrentPath();
RAMFile* findFile(const char* name, const char* parentDir);
int createFile(const char* name, const char* parentDir, int isDirectory);
int deleteFile(const char* name, const char* parentDir);
int deleteFileRecursive(const char* name, const char* parentDir);
int writeFileContent(const char* name, const char* parentDir, const char* content);
const char* readFileContent(const char* name, const char* parentDir);
int usedFileSlots();
int printFindMatches(const char* name);
void listFiles(const char* parentDir);
void listFilesCompact(const char* parentDir);
int changeDirectory(const char* dirName);
char* getFullPath(const char* dirName);

#endif
