# `src` Modules

This directory contains the firmware implementations.

## Files

### `main.cpp`

Coordinates serial input and command execution.

Includes:

- system initialization
- input buffer reading with overflow protection
- command table lookup and alias resolution
- execution of filesystem, GPIO, and system operations
- integrated line-based serial editor
- support for scripts with `sh`

### `board.cpp`

Implements board-specific helpers shared by UNO, ESP32, and Raspberry Pi Pico.

Includes:

- hardware name and architecture reporting
- free-memory and reboot helpers
- built-in LED pin metadata
- demo GPIO pin lists

### `storage.cpp`

Persists BoardOS state across reboots.

Includes:

- filesystem snapshot save/load
- alias table save/load
- current path persistence
- board-specific storage backends for UNO, ESP32, and Pico

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
- `board.cpp` -> `include/board.h`
- `storage.cpp` -> `include/storage.h`
- `utils.cpp` -> `include/utils.h`

`main.cpp` includes all those headers because it connects the rest of the system.

## Compilation

Con PlatformIO:

```bash
platformio run -e uno
platformio upload -e uno
platformio monitor -e uno
```

## Adding a New Command

To add a new command to BoardOS:

1. **Declare the handler function** in `main.cpp`:

```cpp
void handleMyCommand(const char* command, const char* args);
```

2. **Add an entry to the command table** (search for `commandTable[]`):

```cpp
const CommandSpec commandTable[] = {
  // ... existing commands ...
  {"mycommand", "mycommand [arg]", "Brief description", handleMyCommand},
};
```

3. **Implement the handler** in `main.cpp`:

```cpp
void handleMyCommand(const char* command, const char* args) {
  // Parse arguments from 'args' string
  // Execute your logic
  // Use Serial.println() to output results
}
```

Key points:

- The command name should be lowercase in the table
- The usage string shows syntax help displayed by `help [command]`
- Arguments come as a single string that you parse as needed
- Use `Serial.print*()` functions for output
- Handlers receive `command` (the command name) and `args` (remaining input)
