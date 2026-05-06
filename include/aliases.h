#ifndef ALIASES_H
#define ALIASES_H

#include "config.h"

// Structure for command aliases
typedef struct {
  char name[ALIAS_NAME_LEN];
  char value[ALIAS_VAL_LEN];
  int active;
} AliasEntry;

// Global alias variables
extern AliasEntry aliases[MAX_ALIASES];

// Alias functions
void initAliases();
int createAlias(const char* name, const char* value);
int setAlias(const char* name, const char* value);
int deleteAlias(const char* name);
AliasEntry* findAlias(const char* name);
int resolveAlias(const char* name, char* output, int outputSize);
int printAlias(const char* name);
void listAliases();

#endif
