// ============================================
// BoardOS - Main Entry Point
// ============================================

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "config.h"
#include "board.h"
#include "filesystem.h"
#include "dmesg.h"
#include "aliases.h"
#include "storage.h"
#include "utils.h"

namespace {

struct CommandSpec {
  const char* name;
  const char* usage;
  const char* description;
  void (*handler)(const char* command, const char* args);
};

char inputBuffer[INPUT_BUFFER_SIZE] = "";
int inputLen = 0;
int inputOverflow = 0;
int lastLineEndedWithCarriageReturn = 0;
int editorActive = 0;
int editorCreatesFile = 0;
char editorTarget[NAME_LEN] = "";
char editorBuffer[CONTENT_LEN] = "";
int editorLength = 0;

void executeCommand(char* line);
void executeCommandInternal(char* line, int depth);
void runScript(const char* content);
void handleEditorLine(const char* line);

void handleAlias(const char* command, const char* args);
void handleCat(const char* command, const char* args);
void handleCd(const char* command, const char* args);
void handleClear(const char* command, const char* args);
void handleDmesg(const char* command, const char* args);
void handleEdit(const char* command, const char* args);
void handleEcho(const char* command, const char* args);
void handleFind(const char* command, const char* args);
void handleGpio(const char* command, const char* args);
void handleHelp(const char* command, const char* args);
void handleInfo(const char* command, const char* args);
void handleList(const char* command, const char* args);
void handleMakeEntry(const char* command, const char* args);
void handleMemory(const char* command, const char* args);
void handleNeofetch(const char* command, const char* args);
void handlePinMode(const char* command, const char* args);
void handlePwd(const char* command, const char* args);
void handlePwm(const char* command, const char* args);
void handleRead(const char* command, const char* args);
void handleReboot(const char* command, const char* args);
void handleRemove(const char* command, const char* args);
void handleScript(const char* command, const char* args);
void handleSlots(const char* command, const char* args);
void handleUname(const char* command, const char* args);
void handleUptime(const char* command, const char* args);
void handleWhoami(const char* command, const char* args);
void handleWrite(const char* command, const char* args);
void handleSleep(const char* command, const char* args);

const CommandSpec commandTable[] = {
  {"alias", "alias [name]=[command] | alias [name]", "Create or show aliases", handleAlias},
  {"cat", "cat [file]", "Print file contents", handleCat},
  {"cd", "cd [dir|..|/]", "Change current directory", handleCd},
  {"clear", "clear", "Clear the terminal view", handleClear},
  {"df", "df", "Show filesystem slot usage and free RAM", handleMemory},
  {"dmesg", "dmesg", "Print kernel messages", handleDmesg},
  {"edit", "edit [file]", "Open the integrated serial editor", handleEdit},
  {"echo", "echo [text] | echo [text] > [file]", "Print text or save into a file", handleEcho},
  {"find", "find [name]", "Find matching files by name", handleFind},
  {"free", "free", "Show free RAM", handleMemory},
  {"gpio", "gpio [pin] [on/off/toggle] | gpio vixa [count]", "Control GPIO pins or run the LED demo", handleGpio},
  {"help", "help [command]", "Show command help", handleHelp},
  {"info", "info [file]", "Show file information", handleInfo},
  {"ls", "ls", "List the current directory", handleList},
  {"mkdir", "mkdir [name]", "Create a directory", handleMakeEntry},
  {"neofetch", "neofetch", "Print a BoardOS summary", handleNeofetch},
  {"pinmode", "pinmode [pin] [in/out]", "Set GPIO mode", handlePinMode},
  {"pwd", "pwd", "Print the current directory", handlePwd},
  {"pwm", "pwm [pin] [0-255]", "Write a PWM value to a pin", handlePwm},
  {"read", "read [pin]", "Read a GPIO pin", handleRead},
  {"reboot", "reboot", "Restart the current board", handleReboot},
  {"rm", "rm [file|dir]", "Remove a file or directory", handleRemove},
  {"sleep", "sleep [seconds]", "Delay for a number of seconds", handleSleep},
  {"sh", "sh [script]", "Execute a script file", handleScript},
  {"slots", "slots", "Show used filesystem slots", handleSlots},
  {"touch", "touch [name]", "Create an empty file", handleMakeEntry},
  {"uname", "uname", "Print kernel and hardware information", handleUname},
  {"uptime", "uptime", "Print board uptime", handleUptime},
  {"whoami", "whoami", "Print the current user", handleWhoami},
  {"write", "write [pin] [high/low]", "Write a digital pin value", handleWrite}
};

const int commandCount = sizeof(commandTable) / sizeof(commandTable[0]);

void trimWhitespace(char* text) {
  if (text == NULL || text[0] == '\0') {
    return;
  }

  int start = 0;
  while (text[start] == ' ' || text[start] == '\t') {
    start++;
  }

  int end = strlen(text);
  while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t')) {
    end--;
  }

  if (start > 0) {
    memmove(text, text + start, end - start);
  }

  text[end - start] = '\0';
}

void toLowercase(char* text) {
  for (int i = 0; text[i] != '\0'; i++) {
    if (text[i] >= 'A' && text[i] <= 'Z') {
      text[i] = text[i] - 'A' + 'a';
    }
  }
}

const char* nextToken(const char* input, char* token, int tokenSize) {
  while (*input == ' ' || *input == '\t') {
    input++;
  }

  int index = 0;
  while (*input != '\0' && *input != ' ' && *input != '\t') {
    if (index < tokenSize - 1) {
      token[index++] = *input;
    }
    input++;
  }
  token[index] = '\0';

  while (*input == ' ' || *input == '\t') {
    input++;
  }

  return input;
}

int parseUnsignedInt(const char* text, int* value) {
  if (text == NULL || text[0] == '\0') {
    return 0;
  }

  int parsed = 0;
  for (int i = 0; text[i] != '\0'; i++) {
    if (text[i] < '0' || text[i] > '9') {
      return 0;
    }
    parsed = parsed * 10 + (text[i] - '0');
  }

  *value = parsed;
  return 1;
}

int parsePinAndValue(const char* args, int* pin, char* value, int valueSize) {
  char pinText[INPUT_BUFFER_SIZE];
  const char* rest = nextToken(args, pinText, sizeof(pinText));
  if (!parseUnsignedInt(pinText, pin)) {
    return 0;
  }

  nextToken(rest, value, valueSize);
  return value[0] != '\0';
}

int isCurrentDirectory(const char* path) {
  return strcmp(fsCurrentPath(), path) == 0;
}

int resolveDevicePin(const char* name) {
  if (!isCurrentDirectory("/dev/")) {
    return -1;
  }

  if (strncmp(name, "pin", 3) == 0) {
    int pin = 0;
    if (parseUnsignedInt(name + 3, &pin)) {
      return pin;
    }
  }

  if (strcmp(name, "led") == 0) {
    return boardLedPin();
  }

  return -1;
}

int textMeansHigh(const char* text) {
  char lowered[CONTENT_LEN];
  strncpy(lowered, text, sizeof(lowered) - 1);
  lowered[sizeof(lowered) - 1] = '\0';
  trimWhitespace(lowered);
  toLowercase(lowered);

  return strcmp(lowered, "1") == 0 ||
         strcmp(lowered, "high") == 0 ||
         strcmp(lowered, "on") == 0;
}

void printPrompt() {
  Serial.print(F("root@arduino:"));
  Serial.print(fsCurrentPath());
  Serial.print(F("# "));
}

void logError(const char* message) {
  addDmesgTagged("ERR", message);
  Serial.print(F("Error: "));
  Serial.println(message);
}

void printUsage(const char* usage) {
  Serial.print(F("Usage: "));
  Serial.println(usage);
}

void logInfo(const char* message) {
  addDmesgTagged("SYS", message);
}

void resetInputLine() {
  inputLen = 0;
  inputOverflow = 0;
  inputBuffer[0] = '\0';
}

void exitEditor(int saved) {
  editorActive = 0;
  editorCreatesFile = 0;
  editorTarget[0] = '\0';
  editorBuffer[0] = '\0';
  editorLength = 0;

  if (saved) {
    Serial.println(F("Editor saved."));
  } else {
    Serial.println(F("Editor closed."));
  }
}

void printUptimeLine() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  Serial.print(hours);
  Serial.print(F("h "));
  Serial.print(minutes);
  Serial.print(F("m "));
  Serial.print(seconds);
  Serial.println(F("s"));
}

void printMemorySummary(int includeSlots) {
  if (includeSlots) {
    Serial.print(F("Filesystem slots: "));
    Serial.print(usedFileSlots());
    Serial.print(F("/"));
    Serial.println(MAX_FILES);
  }

  Serial.print(F("Free RAM: "));
  Serial.print(freeMemory());
  Serial.println(F(" bytes"));
}

void printBoardLedPin() {
  int ledPin = boardLedPin();
  Serial.print(F("Built-in LED: "));
  if (ledPin >= 0) {
    Serial.print(F("pin "));
    Serial.println(ledPin);
  } else {
    Serial.println(F("n/a"));
  }
}

const CommandSpec* findCommand(const char* name) {
  for (int i = 0; i < commandCount; i++) {
    if (strcmp(commandTable[i].name, name) == 0) {
      return &commandTable[i];
    }
  }

  return NULL;
}

void finishInputLine() {
  Serial.println();

  if (inputOverflow) {
    if (editorActive) {
      logError("Editor line too long.");
    } else {
      logError("Input too long.");
    }
    resetInputLine();
    if (!editorActive) {
      printPrompt();
    }
    return;
  }

  inputBuffer[inputLen] = '\0';
  if (editorActive) {
    handleEditorLine(inputBuffer);
  } else if (inputLen > 0) {
    trimWhitespace(inputBuffer);
    if (inputBuffer[0] != '\0') {
      executeCommand(inputBuffer);
    }
  }

  resetInputLine();
  if (!editorActive) {
    printPrompt();
  }
}

void handleEditorLine(const char* line) {
  if (strcmp(line, ".") == 0) {
    if (editorCreatesFile && findFile(editorTarget, fsCurrentPath()) == NULL) {
      if (!createFile(editorTarget, fsCurrentPath(), 0)) {
        logError("Could not create file.");
        exitEditor(0);
        return;
      }
    }

    if (!writeFileContent(editorTarget, fsCurrentPath(), editorBuffer)) {
      logError("Editor save failed.");
      exitEditor(0);
      return;
    }

    if (!storageSaveState()) {
      logError("Persistent save failed.");
    }

    exitEditor(1);
    return;
  }

  if (strcmp(line, ":q") == 0) {
    exitEditor(0);
    return;
  }

  int required = strlen(line);
  if (editorLength > 0) {
    required++;
  }

  if (editorLength + required >= CONTENT_LEN) {
    logError("File content limit reached.");
    return;
  }

  if (editorLength > 0) {
    editorBuffer[editorLength++] = '\n';
  }

  strncpy(editorBuffer + editorLength, line, CONTENT_LEN - editorLength - 1);
  editorLength += strlen(line);
  editorBuffer[editorLength] = '\0';
}

void processInputChar(char input) {
  if (input == '\n') {
    if (lastLineEndedWithCarriageReturn) {
      lastLineEndedWithCarriageReturn = 0;
      return;
    }
    finishInputLine();
    return;
  }

  if (input == '\r') {
    lastLineEndedWithCarriageReturn = 1;
    finishInputLine();
    return;
  }

  lastLineEndedWithCarriageReturn = 0;

  if (input == 8 || input == 127) {
    if (inputOverflow) {
      return;
    }

    if (inputLen > 0) {
      inputLen--;
      inputBuffer[inputLen] = '\0';
      Serial.print(F("\b \b"));
    }
    return;
  }

  if (input < 32 || input > 126) {
    return;
  }

  if (inputLen >= INPUT_BUFFER_SIZE - 1) {
    inputOverflow = 1;
    return;
  }

  inputBuffer[inputLen++] = input;
  inputBuffer[inputLen] = '\0';
  Serial.print(input);
}

void handlePinMode(const char* command, const char* args) {
  int pin = 0;
  char mode[8];

  if (!parsePinAndValue(args, &pin, mode, sizeof(mode))) {
    printUsage(findCommand(command)->usage);
    return;
  }

  toLowercase(mode);
  if (strcmp(mode, "out") == 0) {
    pinMode(pin, OUTPUT);
    Serial.println(F("Pin set to OUTPUT"));
    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "Pin %d OUTPUT", pin);
    addDmesgRam(buffer);
    return;
  }

  if (strcmp(mode, "in") == 0) {
    pinMode(pin, INPUT_PULLUP);
    Serial.println(F("Pin set to INPUT_PULLUP"));
    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "Pin %d INPUT", pin);
    addDmesgRam(buffer);
    return;
  }

  logError("Mode must be in or out.");
}

void handleWrite(const char* command, const char* args) {
  int pin = 0;
  char value[8];

  if (!parsePinAndValue(args, &pin, value, sizeof(value))) {
    printUsage(findCommand(command)->usage);
    return;
  }

  toLowercase(value);
  if (strcmp(value, "high") != 0 && strcmp(value, "low") != 0) {
    logError("Value must be high or low.");
    return;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, strcmp(value, "high") == 0 ? HIGH : LOW);
  Serial.println(F("Write OK."));

  char buffer[DMESG_LEN];
  snprintf(buffer, sizeof(buffer), "Pin %d %s", pin, strcmp(value, "high") == 0 ? "HIGH" : "LOW");
  addDmesgRam(buffer);
}

void handleRead(const char* command, const char* args) {
  int pin = 0;
  char pinText[INPUT_BUFFER_SIZE];
  nextToken(args, pinText, sizeof(pinText));

  if (!parseUnsignedInt(pinText, &pin)) {
    printUsage(findCommand(command)->usage);
    return;
  }

  int value = digitalRead(pin);
  Serial.print(F("Pin "));
  Serial.print(pin);
  Serial.print(F(" value: "));
  Serial.println(value);

  char buffer[DMESG_LEN];
  snprintf(buffer, sizeof(buffer), "Pin %d read %d", pin, value);
  addDmesgRam(buffer);
}

void handleGpio(const char* command, const char* args) {
  char target[INPUT_BUFFER_SIZE];
  char action[INPUT_BUFFER_SIZE];
  const char* rest = nextToken(args, target, sizeof(target));

  if (target[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  nextToken(rest, action, sizeof(action));
  toLowercase(target);
  toLowercase(action);

  if (strcmp(target, "vixa") == 0) {
    int count = 10;
    if (action[0] != '\0' && !parseUnsignedInt(action, &count)) {
      logError("Count must be numeric.");
      return;
    }

    if (count <= 0) {
      count = 10;
    }

    size_t demoPinCount = 0;
    const uint8_t* demoPins = boardDemoPins(&demoPinCount);
    addDmesg(F("LED disco mode activated"));
    Serial.println(F("LED DISCO MODE!"));

    for (int cycle = 0; cycle < count; cycle++) {
      for (size_t i = 0; i < demoPinCount; i++) {
        pinMode(demoPins[i], OUTPUT);
        digitalWrite(demoPins[i], HIGH);
        delay(50);
        digitalWrite(demoPins[i], LOW);
      }
    }

    Serial.println(F("Disco finished!"));
    addDmesg(F("Disco complete"));
    return;
  }

  int pin = 0;
  if (!parseUnsignedInt(target, &pin) || action[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  pinMode(pin, OUTPUT);
  if (strcmp(action, "on") == 0) {
    digitalWrite(pin, HIGH);
    Serial.print(F("GPIO "));
    Serial.print(pin);
    Serial.println(F(" ON"));
    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "GPIO %d ON", pin);
    addDmesgRam(buffer);
    return;
  }

  if (strcmp(action, "off") == 0) {
    digitalWrite(pin, LOW);
    Serial.print(F("GPIO "));
    Serial.print(pin);
    Serial.println(F(" OFF"));
    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "GPIO %d OFF", pin);
    addDmesgRam(buffer);
    return;
  }

  if (strcmp(action, "toggle") == 0) {
    digitalWrite(pin, !digitalRead(pin));
    Serial.print(F("GPIO "));
    Serial.print(pin);
    Serial.println(F(" toggled"));
    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "GPIO %d toggled", pin);
    addDmesgRam(buffer);
    return;
  }

  logError("Action must be on, off, or toggle.");
}

void handleList(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("ls")->usage);
    return;
  }

  listFilesCompact(fsCurrentPath());
}

void handleMakeEntry(const char* command, const char* args) {
  char name[INPUT_BUFFER_SIZE];
  nextToken(args, name, sizeof(name));
  trimWhitespace(name);

  if (name[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  if (findFile(name, fsCurrentPath()) != NULL) {
    logError("Entry already exists.");
    return;
  }

  if (!createFile(name, fsCurrentPath(), strcmp(command, "mkdir") == 0)) {
    if (strlen(name) >= NAME_LEN) {
      logError("Name too long.");
    } else if (usedFileSlots() >= MAX_FILES) {
      logError("Filesystem is full.");
    } else {
      logError("Invalid entry name.");
    }
    return;
  }

  if (!storageSaveState()) {
    logError("Persistent save failed.");
  }

  Serial.println(F("OK."));
}

void handleCd(const char* command, const char* args) {
  char name[INPUT_BUFFER_SIZE];
  nextToken(args, name, sizeof(name));
  trimWhitespace(name);

  if (name[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  if (changeDirectory(name)) {
    if (!storageSaveState()) {
      logError("Persistent save failed.");
    }
    return;
  }

  RAMFile* file = findFile(name, fsCurrentPath());
  if (file == NULL) {
    logError("Directory not found.");
  } else if (!file->isDirectory) {
    logError("Not a directory.");
  } else {
    logError("Path too long.");
  }
}

void handlePwd(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("pwd")->usage);
    return;
  }

  Serial.println(fsCurrentPath());
}

void handleEcho(const char* command, const char* args) {
  char text[INPUT_BUFFER_SIZE];
  strncpy(text, args, sizeof(text) - 1);
  text[sizeof(text) - 1] = '\0';

  char* redirect = strchr(text, '>');
  if (redirect == NULL) {
    Serial.println(args);
    return;
  }

  *redirect = '\0';
  redirect++;

  char fileName[INPUT_BUFFER_SIZE];
  strncpy(fileName, redirect, sizeof(fileName) - 1);
  fileName[sizeof(fileName) - 1] = '\0';

  trimWhitespace(text);
  trimWhitespace(fileName);

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  if (!writeFileContent(fileName, fsCurrentPath(), text)) {
    RAMFile* file = findFile(fileName, fsCurrentPath());
    if (file == NULL) {
      logError("File not found.");
    } else if (file->isDirectory) {
      logError("Cannot write to a directory.");
    } else {
      logError("Write failed.");
    }
    return;
  }

  Serial.println(F("Saved."));
  if (!storageSaveState()) {
    logError("Persistent save failed.");
  }

  int devicePin = resolveDevicePin(fileName);
  if (devicePin >= 0) {
    pinMode(devicePin, OUTPUT);
    digitalWrite(devicePin, textMeansHigh(text) ? HIGH : LOW);

    char buffer[DMESG_LEN];
    snprintf(buffer, sizeof(buffer), "GPIO %d via echo", devicePin);
    addDmesgRam(buffer);
  }
}

void handleCat(const char* command, const char* args) {
  char fileName[INPUT_BUFFER_SIZE];
  nextToken(args, fileName, sizeof(fileName));

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  const char* content = readFileContent(fileName, fsCurrentPath());
  if (content == NULL) {
    logError("File not found.");
    return;
  }

  Serial.println(content);
}

void handleInfo(const char* command, const char* args) {
  char fileName[INPUT_BUFFER_SIZE];
  nextToken(args, fileName, sizeof(fileName));

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  RAMFile* file = findFile(fileName, fsCurrentPath());
  if (file == NULL) {
    logError("Entry not found.");
    return;
  }

  Serial.print(F("Name: "));
  Serial.println(file->name);
  Serial.print(F("Type: "));
  Serial.println(file->isDirectory ? F("Directory") : F("File"));
  Serial.print(F("Parent: "));
  Serial.println(file->parentDir);
  Serial.print(F("Size: "));
  Serial.print(strlen(file->content));
  Serial.println(F(" bytes"));
}

void handleRemove(const char* command, const char* args) {
  char fileName[INPUT_BUFFER_SIZE];
  nextToken(args, fileName, sizeof(fileName));

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  if (!deleteFileRecursive(fileName, fsCurrentPath())) {
    logError("Entry not found.");
    return;
  }

  if (!storageSaveState()) {
    logError("Persistent save failed.");
  }

  Serial.println(F("Removed."));
}

void handleDmesg(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("dmesg")->usage);
    return;
  }

  printDmesg();
}

void handleEdit(const char* command, const char* args) {
  char fileName[INPUT_BUFFER_SIZE];
  nextToken(args, fileName, sizeof(fileName));

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  RAMFile* file = findFile(fileName, fsCurrentPath());
  if (file != NULL && file->isDirectory) {
    logError("Editor only works with files.");
    return;
  }

  editorCreatesFile = (file == NULL);
  strncpy(editorTarget, fileName, sizeof(editorTarget) - 1);
  editorTarget[sizeof(editorTarget) - 1] = '\0';
  if (file != NULL) {
    strncpy(editorBuffer, file->content, sizeof(editorBuffer) - 1);
    editorBuffer[sizeof(editorBuffer) - 1] = '\0';
  } else {
    editorBuffer[0] = '\0';
  }
  editorLength = strlen(editorBuffer);
  editorActive = 1;

  Serial.print(F("Editing "));
  Serial.println(editorTarget);
  Serial.println(F("Enter text lines. Single '.' saves. ':q' cancels."));
  if (editorBuffer[0] != '\0') {
    Serial.println(F("Current content:"));
    Serial.println(editorBuffer);
  }
}

void handleUptime(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("uptime")->usage);
    return;
  }

  Serial.print(F("up "));
  printUptimeLine();
  addDmesg(F("uptime command"));
}

void handleMemory(const char* command, const char* args) {
  if (args[0] != '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  printMemorySummary(strcmp(command, "df") == 0);
}

void handleWhoami(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("whoami")->usage);
    return;
  }

  Serial.println(F("root"));
}

void handleUname(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("uname")->usage);
    return;
  }

  Serial.println(BOARD_OS_NAME " v" BOARD_OS_VERSION);
  Serial.print(F("Kernel: "));
  Serial.println(boardArchName());
  Serial.print(F("Hardware: "));
  Serial.println(boardHardwareName());
  printBoardLedPin();
  Serial.print(F("RAM: "));
  Serial.print(freeMemory());
  Serial.println(F(" bytes free"));
}

void handleReboot(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("reboot")->usage);
    return;
  }

  Serial.println(F("Rebooting..."));
  addDmesg(F("System reboot"));
  delay(500);
  boardRestart();
}

void handleClear(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("clear")->usage);
    return;
  }

  for (int i = 0; i < 30; i++) {
    Serial.println();
  }
}

void handleScript(const char* command, const char* args) {
  char fileName[INPUT_BUFFER_SIZE];
  nextToken(args, fileName, sizeof(fileName));

  if (fileName[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  const char* content = readFileContent(fileName, fsCurrentPath());
  if (content == NULL) {
    logError("Script not found.");
    return;
  }

  addDmesg(F("sh: running script"));
  runScript(content);
}

void handlePwm(const char* command, const char* args) {
  int pin = 0;
  char valueText[INPUT_BUFFER_SIZE];

  if (!parsePinAndValue(args, &pin, valueText, sizeof(valueText))) {
    printUsage(findCommand(command)->usage);
    return;
  }

  int pwmValue = 0;
  if (!parseUnsignedInt(valueText, &pwmValue)) {
    logError("PWM value must be numeric.");
    return;
  }

  if (pwmValue < 0) {
    pwmValue = 0;
  }
  if (pwmValue > 255) {
    pwmValue = 255;
  }

  pinMode(pin, OUTPUT);
  boardWritePwm(pin, pwmValue);

  Serial.print(F("PWM pin "));
  Serial.print(pin);
  Serial.print(F(" set to "));
  Serial.println(pwmValue);

  char buffer[DMESG_LEN];
  snprintf(buffer, sizeof(buffer), "PWM %d = %d", pin, pwmValue);
  addDmesgRam(buffer);
}

void handleAlias(const char* command, const char* args) {
  (void)command;
  if (args[0] == '\0') {
    listAliases();
    return;
  }

  char aliasArgs[INPUT_BUFFER_SIZE];
  strncpy(aliasArgs, args, sizeof(aliasArgs) - 1);
  aliasArgs[sizeof(aliasArgs) - 1] = '\0';

  char* equals = strchr(aliasArgs, '=');
  if (equals == NULL) {
    trimWhitespace(aliasArgs);
    toLowercase(aliasArgs);
    if (!printAlias(aliasArgs)) {
      logError("Alias not found.");
    }
    return;
  }

  *equals = '\0';
  equals++;

  char aliasName[INPUT_BUFFER_SIZE];
  char aliasValue[INPUT_BUFFER_SIZE];
  strncpy(aliasName, aliasArgs, sizeof(aliasName) - 1);
  aliasName[sizeof(aliasName) - 1] = '\0';
  strncpy(aliasValue, equals, sizeof(aliasValue) - 1);
  aliasValue[sizeof(aliasValue) - 1] = '\0';

  trimWhitespace(aliasName);
  trimWhitespace(aliasValue);
  toLowercase(aliasName);

  if (aliasName[0] == '\0' || aliasValue[0] == '\0') {
    logError("Alias name and value are required.");
    return;
  }

  if (strlen(aliasName) >= ALIAS_NAME_LEN) {
    logError("Alias name too long.");
    return;
  }

  if (!setAlias(aliasName, aliasValue)) {
    logError("Alias table is full.");
    return;
  }

  if (!storageSaveState()) {
    logError("Persistent save failed.");
  }

  Serial.println(F("Alias set."));
}

void handleSlots(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("slots")->usage);
    return;
  }

  Serial.print(F("("));
  Serial.print(usedFileSlots());
  Serial.print(F("/"));
  Serial.print(MAX_FILES);
  Serial.println(F(")"));
}

void handleFind(const char* command, const char* args) {
  char name[INPUT_BUFFER_SIZE];
  nextToken(args, name, sizeof(name));

  if (name[0] == '\0') {
    printUsage(findCommand(command)->usage);
    return;
  }

  if (!printFindMatches(name)) {
    logError("No matches found.");
  }
}

void handleHelp(const char* command, const char* args) {
  (void)command;

  char requested[INPUT_BUFFER_SIZE];
  nextToken(args, requested, sizeof(requested));
  trimWhitespace(requested);
  toLowercase(requested);

  if (requested[0] != '\0') {
    const CommandSpec* found = findCommand(requested);
    if (found == NULL) {
      logError("Command not found.");
      return;
    }

    Serial.print(found->usage);
    Serial.print(F(" - "));
    Serial.println(found->description);
    return;
  }

  Serial.println(F("Commands:"));
  for (int i = 0; i < commandCount; i++) {
    Serial.print(F(" - "));
    Serial.print(commandTable[i].usage);
    Serial.print(F(" : "));
    Serial.println(commandTable[i].description);
  }
}

void handleNeofetch(const char* command, const char* args) {
  (void)command;
  if (args[0] != '\0') {
    printUsage(findCommand("neofetch")->usage);
    return;
  }

  Serial.println(F(" ____                      _  ____   _____ "));
  Serial.println(F("|  _ \\                    | |/ __ \\ / ____|"));
  Serial.println(F("| |_) | ___   __ _ _ __ __| | |  | | (___  "));
  Serial.println(F("|  _ < / _ \\ / _` | '__/ _` | |  | |\\___ \\ "));
  Serial.println(F("| |_) | (_) | (_| | | | (_| | |__| |____) |"));
  Serial.println(F("|____/ \\___/ \\__,_|_|  \\__,_|\\____/|_____/ "));
  Serial.println(BOARD_OS_NAME " v" BOARD_OS_VERSION);
  Serial.print(F("Hardware: "));
  Serial.println(boardHardwareName());
  Serial.print(F("Kernel: "));
  Serial.println(boardArchName());
  printBoardLedPin();
  Serial.print(F("Uptime: "));
  printUptimeLine();
  printMemorySummary(1);
}

void handleSleep(const char* command, const char* args) {
  (void)command;

  char secondsText[INPUT_BUFFER_SIZE];
  nextToken(args, secondsText, sizeof(secondsText));

  if (secondsText[0] == '\0') {
    printUsage(findCommand("sleep")->usage);
    return;
  }

  int seconds = 0;
  if (!parseUnsignedInt(secondsText, &seconds)) {
    logError("Seconds must be numeric.");
    return;
  }

  Serial.print(F("Sleeping for "));
  Serial.print(seconds);
  Serial.println(F(" seconds..."));
  delay(seconds * 1000);
  Serial.println(F("Awake!"));
}

void runScript(const char* content) {
  char line[INPUT_BUFFER_SIZE];
  int lineIndex = 0;
  int lineNumber = 0;
  int overflow = 0;
  int length = strlen(content);

  for (int i = 0; i <= length; i++) {
    char current = (i < length) ? content[i] : ';';

    if (current == ';' || current == '\n' || current == '\r') {
      if (overflow) {
        logError("Script line too long.");
        overflow = 0;
        lineIndex = 0;
        continue;
      }

      if (lineIndex > 0) {
        line[lineIndex] = '\0';
        trimWhitespace(line);
        if (line[0] != '\0') {
          lineNumber++;
          Serial.print(F("[sh:"));
          Serial.print(lineNumber);
          Serial.print(F("] "));
          Serial.println(line);
          executeCommand(line);
        }
        lineIndex = 0;
      }
    } else if (!overflow) {
      if (lineIndex >= INPUT_BUFFER_SIZE - 1) {
        overflow = 1;
      } else {
        line[lineIndex++] = current;
      }
    }
  }

  addDmesg(F("sh: script finished"));
  Serial.println(F("Script finished."));
}

void executeCommand(char* line) {
  executeCommandInternal(line, 0);
}

void executeCommandInternal(char* line, int depth) {
  if (depth > 4) {
    logError("Alias expansion too deep.");
    return;
  }

  trimWhitespace(line);
  if (line[0] == '\0') {
    return;
  }

  char command[INPUT_BUFFER_SIZE];
  char args[INPUT_BUFFER_SIZE];
  const char* rest = nextToken(line, command, sizeof(command));
  strncpy(args, rest, sizeof(args) - 1);
  args[sizeof(args) - 1] = '\0';
  trimWhitespace(args);
  toLowercase(command);

  const CommandSpec* commandSpec = findCommand(command);
  if (commandSpec != NULL) {
    commandSpec->handler(commandSpec->name, args);
    return;
  }

  char aliasValue[INPUT_BUFFER_SIZE];
  if (resolveAlias(command, aliasValue, sizeof(aliasValue))) {
    char expandedLine[INPUT_BUFFER_SIZE];
    strncpy(expandedLine, aliasValue, sizeof(expandedLine) - 1);
    expandedLine[sizeof(expandedLine) - 1] = '\0';

    if (args[0] != '\0') {
      if (strlen(expandedLine) + 1 + strlen(args) >= sizeof(expandedLine)) {
        logError("Expanded alias is too long.");
        return;
      }
      strcat(expandedLine, " ");
      strcat(expandedLine, args);
    }

    executeCommandInternal(expandedLine, depth + 1);
    return;
  }

  logError("Unknown command.");
}

}

void setup() {
  Serial.begin(115200);
  boardInit();
  storageInit();
  initFS();
  initAliases();
  if (!storageLoadState()) {
    storageSaveState();
  }
  delay(1000);

  Serial.println("\n--- " BOARD_OS_NAME " v" BOARD_OS_VERSION " ---");
  Serial.println(F("Type 'help' for commands."));
  logInfo("BoardOS ready");
  printPrompt();
}

void loop() {
  while (Serial.available() > 0) {
    processInputChar((char)Serial.read());
  }
}
