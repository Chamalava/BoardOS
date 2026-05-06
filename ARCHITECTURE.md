# BoardOS Architecture

This document summarizes how the project is divided and what responsibility each module has.

Supported boards in the current PlatformIO configuration:

- Arduino UNO (`env:uno`)
- ESP32 Dev Module (`env:esp32dev`)
- Raspberry Pi Pico (`env:pico`, Earle Philhower Arduino-Pico core)

## Overview

```text
BoardOS/
├── platformio.ini
├── README.md
├── ARCHITECTURE.md
├── include/
│   ├── config.h
│   ├── filesystem.h
│   ├── dmesg.h
│   ├── aliases.h
│   └── utils.h
└── src/
    ├── main.cpp
    ├── filesystem.cpp
    ├── dmesg.cpp
    ├── aliases.cpp
    ├── utils.cpp
    └── README.md
```

## Modules

### `include/config.h`

Defines constants shared across the whole project:

- kernel version and name
- filesystem limits
- input buffer size
- `dmesg` capacity
- maximum number of aliases

### `include/filesystem.h` y `src/filesystem.cpp`

They implement a simple RAM filesystem. Each entry uses the `RAMFile` structure, with name, content, parent directory, and status flags.

Responsibilities:

- initialize `/home` and `/dev`
- create and delete entries
- search for files by name and directory
- list the contents of a directory

### `include/dmesg.h` y `src/dmesg.cpp`

They store short system messages in a circular buffer.

Responsibilities:

- record messages from FLASH or RAM
- associate each message with a timestamp in seconds
- show or clear the history

### `include/aliases.h` y `src/aliases.cpp`

They maintain a fixed alias table for commands.

Responsibilities:

- initialize the table
- create and delete aliases
- search for aliases by name
- list active aliases

### `include/utils.h` y `src/utils.cpp`

They contain general system utilities.

Responsibilities:

- query free memory
- expose the reset pointer
- basic conversions between string and integer

### `src/main.cpp`

It is the input and coordination layer.

Responsibilities:

- initialize serial, filesystem, and aliases
- read commands from the serial monitor
- split the command and arguments
- execute filesystem, GPIO, and system operations
- interpret scripts with `sh`

## Dependency flow

```text
main.cpp
├── config.h
├── filesystem.h
├── dmesg.h
├── aliases.h
└── utils.h

filesystem.cpp -> filesystem.h, dmesg.h
dmesg.cpp      -> dmesg.h
aliases.cpp    -> aliases.h, dmesg.h
utils.cpp      -> utils.h
```

`main.cpp` depends on all modules because it concentrates the command interface.

## Common adjustments

Change version:

```cpp
#define BOARD_OS_VERSION "1.0.7"
```

Increase the number of files:

```cpp
#define MAX_FILES 20
```

Increase alias capacity:

```cpp
#define MAX_ALIASES 8
```

## Add a module

1. Create the header in `include/`.
2. Create the implementation in `src/`.
3. Include the header in `src/main.cpp`.
4. Initialize or invoke the new module from `setup()` or `executeCommand()`.

Minimal example:

```cpp
// include/mymodule.h
void myModuleInit();
void myModuleFunc();
```

```cpp
// src/mymodule.cpp
#include "mymodule.h"

void myModuleInit() {}
void myModuleFunc() {}
```

```cpp
// src/main.cpp
#include "mymodule.h"
```
