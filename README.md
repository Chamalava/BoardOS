# BoardOS

BoardOS is a command interpreter for Arduino UNO, ESP32, and Raspberry Pi Pico boards (more boards coming soon), with a RAM filesystem, basic system utilities, and simple GPIO control from the serial monitor, using the PlatformIO template as a base.

## Structure

```text
BoardOS/
├── include/
│   ├── board.h
│   ├── compat.h
│   ├── config.h
│   ├── filesystem.h
│   ├── dmesg.h
│   ├── aliases.h
│   ├── storage.h
│   └── utils.h
├── src/
│   ├── board.cpp
│   ├── main.cpp
│   ├── filesystem.cpp
│   ├── dmesg.cpp
│   ├── aliases.cpp
│   ├── storage.cpp
│   ├── utils.cpp
│   └── README.md
├── platformio.ini
└── ARCHITECTURE.md
```

## Main features

- Persistent lightweight filesystem for simple directories and files.
- System event log with `dmesg`.
- Commands for pin reading, writing, and PWM.
- Command aliases.
- Board-aware built-in LED metadata for UNO, ESP32, and Pico.
- Command dispatch table with built-in help.
- Integrated serial editor for small files and scripts.
- Script execution separated by `;`.

## Compilation

Con PlatformIO:

```bash
cd /path/to/BoardOS
platformio run -e uno
platformio upload -e uno
platformio monitor -e uno
```

Available environments:

- `uno`
- `esp32dev`
- `pico` (Earle Philhower Arduino-Pico core)

In VS Code you can also compile and upload with the `PlatformIO IDE` extension.

## Commands

Filesystem:

- `ls`
- `cd [dir|..|/]`
- `pwd`
- `mkdir [name]`
- `touch [file]`
- `cat [file]`
- `echo [text] > [file]`
- `edit [file]`
- `rm [file|dir]`
- `info [file]`
- `find [name]`

GPIO:

- `pinmode [pin] [in/out]`
- `write [pin] [high/low]`
- `read [pin]`
- `gpio [pin] [on/off/toggle]`
- `gpio vixa [count]`
- `pwm [pin] [0-255]`

Sistema:

- `uname`
- `whoami`
- `uptime`
- `df`
- `free`
- `dmesg`
- `clear`
- `reboot`
- `neofetch`

Alias y scripts:

- `alias`
- `alias [name]=[command]`
- `sh [script]`
- `help [command]`
- `slots`

## Configuration

The main limits are in `include/config.h`:

- `BOARD_OS_VERSION`
- `BOARD_OS_NAME`
- `BOARD_ARCH`
- `BOARD_HARDWARE`
- `MAX_FILES`
- `DMESG_LINES`
- `MAX_ALIASES`
- `INPUT_BUFFER_SIZE`

At startup, `/dev/` is populated with board-specific `pinNN` files using the built-in LED pin plus a couple of demo GPIOs for the selected board target.

Persistent storage backend:

- Arduino UNO: `EEPROM`
- ESP32: `Preferences` (NVS flash)
- Raspberry Pi Pico: `EEPROM` emulation on flash

## Example

These examples match the current shell limits: short file names, short commands, and `echo` redirection written without quotes.

Arduino UNO built-in LED on pin `13`:

```text
root@arduino:/# touch uno.sh
OK.
root@arduino:/# echo gpio 13 on > uno.sh
Saved.
root@arduino:/# sh uno.sh
[sh:1] gpio 13 on
GPIO 13 ON
Script finished.
```

Editing a script directly on serial:

```text
root@arduino:/# edit uno.sh
Editing uno.sh
Enter text lines. Single '.' saves. ':q' cancels.
Current content:
gpio 13 on
gpio 13 off
.
Editor saved.
```

ESP32 built-in LED on pin `2`:

```text
root@arduino:/# touch esp.sh
OK.
root@arduino:/# echo gpio 2 on > esp.sh
Saved.
root@arduino:/# sh esp.sh
[sh:1] gpio 2 on
GPIO 2 ON
Script finished.
```

Raspberry Pi Pico built-in LED on pin `25`:

```text
root@arduino:/# touch pico.sh
OK.
root@arduino:/# echo gpio 25 on > pico.sh
Saved.
root@arduino:/# sh pico.sh
[sh:1] gpio 25 on
GPIO 25 ON
Script finished.
```

## Built-in LED Pins

Use these pins if you want to blink the LED already soldered on the board PCB:

- Arduino UNO: `13`
- ESP32 (`esp32dev`): `2`
- Raspberry Pi Pico (`pico`): `25`

## Notes

- Files and aliases are now stored persistently instead of being lost on reboot.
- The current limit is 10 entries in the filesystem.
- The current limit is 4 aliases.
- File contents are still intentionally small because the design now targets tiny persistent storage footprints across UNO, ESP32, and Pico.
- `cd ..` walks one level up instead of always jumping back to root.
- `help` can print all commands or a single command usage line.
- The `pico` environment uses the Earle Philhower Arduino-Pico core through PlatformIO core switching.
- On Raspberry Pi Pico with that core, `free` and `df` use `rp2040.getFreeHeap()` to report free heap bytes.
- For `esp32dev`, many boards use the onboard LED on GPIO `2`, but some ESP32 board variants may wire the PCB LED differently.

## Future features

* [ ] SD / FatFs backend
* [ ] I2C interface
* [ ] Date cmd

Current version: `1.0.6`
