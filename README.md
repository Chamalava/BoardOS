# BoardOS

BoardOS is a command interpreter for Arduino UNO, ESP32, and Raspberry Pi Pico boards (more boards coming soon), with a RAM filesystem, basic system utilities, and simple GPIO control from the serial monitor, using the PlatformIO template as a base.

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

With PlatformIO:

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

System:

- `uname`
- `whoami`
- `uptime`
- `df`
- `free`
- `dmesg`
- `clear`
- `reboot`
- `neofetch`

Aliasing and scripts:

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

- The filesystem is volatile and lives in RAM.
- Data is lost when the board restarts.
- The current limit is 10 entries in the filesystem.
- The current limit is 4 aliases.
- The `pico` environment uses the Earle Philhower Arduino-Pico core through PlatformIO core switching.
- On Raspberry Pi Pico with that core, `free` and `df` use `rp2040.getFreeHeap()` to report free heap bytes.
- For `esp32dev`, many boards use the onboard LED on GPIO `2`, but some ESP32 board variants may wire the PCB LED differently.

## Future features

* [ ] EEPROM support
* [ ] I2C interface
* [ ] Date cmd

Current version: `1.0.6`
