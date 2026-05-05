#include <Arduino.h>
#include <string.h>
#include "aliases.h"
#include "dmesg.h"

// Global alias variables
AliasEntry aliases[MAX_ALIASES];

// Initialize aliases
void initAliases() {
  for (int i = 0; i < MAX_ALIASES; i++) {
    aliases[i].name[0] = '\0';
    aliases[i].value[0] = '\0';
    aliases[i].active = 0;
  }
  addDmesg(F("Aliases system initialized"));
}

// Create a new alias
int createAlias(const char* name, const char* value) {
  if (findAlias(name)) {
    addDmesgRam("Error: Alias already exists");
    return 0;
  }

  for (int i = 0; i < MAX_ALIASES; i++) {
    if (!aliases[i].active) {
      strncpy(aliases[i].name, name, ALIAS_NAME_LEN - 1);
      aliases[i].name[ALIAS_NAME_LEN - 1] = '\0';
      strncpy(aliases[i].value, value, ALIAS_VAL_LEN - 1);
      aliases[i].value[ALIAS_VAL_LEN - 1] = '\0';
      aliases[i].active = 1;
      return 1; // Success
    }
  }

  addDmesgRam("Error: Alias table full");
  return 0; // Alias table full
}

// Delete an alias
int deleteAlias(const char* name) {
  AliasEntry* alias = findAlias(name);
  if (alias) {
    alias->active = 0;
    alias->name[0] = '\0';
    alias->value[0] = '\0';
    return 1; // Success
  }
  addDmesgRam("Error: Alias not found");
  return 0; // Alias not found
}

// Search for an alias
AliasEntry* findAlias(const char* name) {
  for (int i = 0; i < MAX_ALIASES; i++) {
    if (aliases[i].active && strcmp(aliases[i].name, name) == 0) {
      return &aliases[i];
    }
  }
  return NULL;
}

// List all aliases
void listAliases() {
  Serial.println("\n=== ALIASES ===");
  int found = 0;
  for (int i = 0; i < MAX_ALIASES; i++) {
    if (aliases[i].active) {
      Serial.print(aliases[i].name);
      Serial.print(" = ");
      Serial.println(aliases[i].value);
      found = 1;
    }
  }
  if (!found) {
    Serial.println("(no aliases defined)");
  }
  Serial.println("===============\n");
}
