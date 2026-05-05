# `src` Modules

This directory contains the firmware implementations.

## Files

### `main.cpp`

Coordinates serial input and command execution.

Includes:

- system initialization
- input buffer reading
- basic command parser
- execution of filesystem, GPIO, and system operations
- support for scripts with `sh`

### `filesystem.cpp`

Implements the RAM filesystem and manages the global `fs` array.

### `dmesg.cpp`

Implements the circular buffer for system messages.

### `aliases.cpp`

Implements the fixed alias table.

### `utils.cpp`

Groups memory, restart, and type conversion utilities.

## Relation with `include/`

Each `.cpp` file has its main declaration in `include/`:

- `filesystem.cpp` -> `include/filesystem.h`
- `dmesg.cpp` -> `include/dmesg.h`
- `aliases.cpp` -> `include/aliases.h`
- `utils.cpp` -> `include/utils.h`

`main.cpp` includes all those headers because it connects the rest of the system.

## Compilation

Con PlatformIO:

```bash
platformio run
platformio upload
platformio monitor
```
