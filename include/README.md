# `include` - Header Files

This directory contains all header files that define the public interfaces and data structures for BoardOS modules.

## Core Headers

### `config.h`

Global configuration and system constants.

Defines:
- System name and version (`BOARD_OS_NAME`, `BOARD_OS_VERSION`)
- Filesystem limits (`MAX_FILES`, `NAME_LEN`, `CONTENT_LEN`)
- Memory constraints (`DMESG_LINES`, `MAX_ALIASES`, `INPUT_BUFFER_SIZE`)
- Board-specific hardware strings

### `compat.h`

Platform compatibility macros and hardware detection.

Handles:
- Board detection flags (`BOARD_ARCH_UNO`, `BOARD_ARCH_ESP32`, `BOARD_ARCH_RP2040`)
- Board-specific definitions and abstractions
- Ensures portable code across Arduino, ESP32, and Pico

### `filesystem.h` and `src/filesystem.cpp`

RAM-based filesystem implementation.

Provides:
- Directory and file creation, deletion, and listing
- File read and write operations
- Current working directory tracking
- Boot-time initialization of `/home` and `/dev`

Main structures:
- `RAMFile`: Stores name, content, type, and parent reference

### `dmesg.h` and `src/dmesg.cpp`

Circular buffer for system messages and kernel events.

Provides:
- Record messages with timestamps
- Circular storage to prevent overflow
- Print or clear message history

### `aliases.h` and `src/aliases.cpp`

Command alias lookup table.

Provides:
- Create and delete custom command aliases
- Search aliases by name
- List all active aliases
- Fixed-size table (configurable via `MAX_ALIASES`)

### `board.h` and `src/board.cpp`

Board-specific abstraction layer.

Provides:
- Hardware name and CPU architecture reporting
- Free memory queries
- Reboot function
- Built-in LED pin metadata for each board
- Demo pin lists for GPIO testing

### `storage.h` and `src/storage.cpp`

Persistent state management across reboots.

Handles:
- Filesystem snapshot save and load
- Alias table persistence
- Current working directory restoration
- Board-specific EEPROM or Flash backends

### `utils.h` and `src/utils.cpp`

General utility functions.

Provides:
- Free memory calculation
- Reset/reboot pointer
- String to integer conversion
- Type checking and conversion helpers

## Header Structure

Each header follows a standard pattern:

```cpp
#ifndef MODULENAME_H
#define MODULENAME_H

// Type definitions, function declarations, and macros

#endif
```

This prevents multiple inclusion of the same header during compilation.
