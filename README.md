# BoardOS

BoardOS is a command interpreter for Arduino- and ESP32-compatible boards (more boards coming soon), with a RAM filesystem, basic system utilities, and simple GPIO control from the serial monitor, using the PlatformIO template as a base.

## Structure

```text
BoardOS/
├── include/
│   ├── config.h
│   ├── filesystem.h
│   ├── dmesg.h
│   ├── aliases.h
│   └── utils.h
├── src/
│   ├── main.cpp
│   ├── filesystem.cpp
│   ├── dmesg.cpp
│   ├── aliases.cpp
│   ├── utils.cpp
│   └── README.md
├── platformio.ini
└── ARCHITECTURE.md
```

## Main features

- RAM filesystem with simple directories and files.
- System event log with `dmesg`.
- Commands for pin reading, writing, and PWM.
- Command aliases.
- Script execution separated by `;`.

## Compilation

Con PlatformIO:

```bash
cd /path/to/BoardOS
platformio run
platformio upload
platformio monitor
```

In VS Code you can also compile and upload with the `PlatformIO IDE` extension.

## Commands

Filesystem:

- `ls`
- `cd [dir]`
- `pwd`
- `mkdir [name]`
- `touch [file]`
- `cat [file]`
- `echo [text] > [file]`
- `rm [file]`
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

## Example

```text
root@arduino:/# mkdir home
OK.
root@arduino:/# cd home
root@arduino:/home/# touch script.sh
OK.
root@arduino:/home/# echo "gpio 13 on; gpio 13 off" > script.sh
Saved.
root@arduino:/home/# sh script.sh
[sh:1] gpio 13 on
GPIO 13 ON
[sh:2] gpio 13 off
GPIO 13 OFF
Script finished.
```

## Notes

- The filesystem is volatile and lives in RAM.
- Data is lost when the board restarts.
- The current limit is 10 entries in the filesystem.
- The current limit is 4 aliases.

## Future features

* [ ] EEPROM support
* [ ] I2C interface
* [ ] Date cmd

Current version: `1.0.6`
