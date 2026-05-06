#include <Arduino.h>
#include <string.h>
#include "aliases.h"
#include "dmesg.h"

AliasEntry aliases[MAX_ALIASES];

// Initialize the aliases table
void initAliases() {
  for (int i = 0; i < MAX_ALIASES; i++) {
    aliases[i].name[0] = '\0';
    aliases[i].value[0] = '\0';
    aliases[i].active = 0;
  }
  addDmesg(F("Aliases system initialized"));
}

// Create a new alias (fails if name already exists)
int createAlias(const char* name, const char* value) {
  if (name == NULL || value == NULL || name[0] == '\0') {
    return 0;
  }

  if (findAlias(name) != NULL) {
    return 0;
  }

  for (int i = 0; i < MAX_ALIASES; i++) {
    if (!aliases[i].active) {
      strncpy(aliases[i].name, name, ALIAS_NAME_LEN - 1);
      aliases[i].name[ALIAS_NAME_LEN - 1] = '\0';
      strncpy(aliases[i].value, value, ALIAS_VAL_LEN - 1);
      aliases[i].value[ALIAS_VAL_LEN - 1] = '\0';
      aliases[i].active = 1;
      return 1;
    }
  }

  return 0;
}

// Create or update an alias
int setAlias(const char* name, const char* value) {
  if (name == NULL || value == NULL || name[0] == '\0') {
    return 0;
  }

  AliasEntry* existing = findAlias(name);
  if (existing != NULL) {
    strncpy(existing->value, value, ALIAS_VAL_LEN - 1);
    existing->value[ALIAS_VAL_LEN - 1] = '\0';
    return 1;
  }

  return createAlias(name, value);
}

// Delete an alias by name
int deleteAlias(const char* name) {
  AliasEntry* alias = findAlias(name);
  if (alias == NULL) {
    return 0;
  }

  alias->active = 0;
  alias->name[0] = '\0';
  alias->value[0] = '\0';
  return 1;
}

// Find an alias entry by name
AliasEntry* findAlias(const char* name) {
  if (name == NULL) {
    return NULL;
  }

  for (int i = 0; i < MAX_ALIASES; i++) {
    if (aliases[i].active && strcmp(aliases[i].name, name) == 0) {
      return &aliases[i];
    }
  }

  return NULL;
}

// Resolve an alias name to its command value
int resolveAlias(const char* name, char* output, int outputSize) {
  AliasEntry* alias = findAlias(name);
  if (alias == NULL || output == NULL || outputSize <= 0) {
    return 0;
  }

  strncpy(output, alias->value, outputSize - 1);
  output[outputSize - 1] = '\0';
  return 1;
}

int printAlias(const char* name) {
  AliasEntry* alias = findAlias(name);
  if (alias == NULL) {
    return 0;
  }

  Serial.print(alias->name);
  Serial.print(F("='"));
  Serial.print(alias->value);
  Serial.println(F("'"));
  return 1;
}

void listAliases() {
  int found = 0;

  for (int i = 0; i < MAX_ALIASES; i++) {
    if (aliases[i].active) {
      printAlias(aliases[i].name);
      found = 1;
    }
  }

  if (!found) {
    Serial.println(F("No aliases."));
  }
}
