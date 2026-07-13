<img width="1774" height="887" alt="TinyRat" src="https://github.com/user-attachments/assets/5cbf5522-1456-4b99-a7e1-0f2b17d5837b" />
# TinyRat

> A first serious attempt to build a custom embedded platform on RP2040: with UI, applications, USB workflows, and file-based resources.

[Русская версия](./README.md)

---

## About the project

**TinyRat** is an RP2040-based embedded system with its own graphical launcher, modular applications, USB device functionality, and resources loaded from a filesystem. It is built on top of a creative BadUSB-based board designed by the RoboLab of the Moscow campus of School 21 by Sber.

You can find their other interesting devices in their [repository](https://github.com/RoboLab21), and view the detailed board schematic [here](https://github.com/Freimor/HRDW-Automaton_v3).

The idea behind this project was not to create a single demo firmware module, but to build a small standalone platform with its own UI, application launching logic, and real interaction with the outside world through USB and files.

## Why this was made

This project was intended both as a portfolio repository and as a real learning experience. It is my first serious attempt to work with hardware at this level, so some parts are implemented rather directly, and some decisions were probably not the best possible ones.

It is important for me to state this honestly: there were almost certainly more correct, cleaner, or more efficient ways to implement some of these solutions. Still, this repository shows a working system, real integration of multiple subsystems, and an attempt to build a coherent embedded product rather than an abstract "toy example".

Anyone is welcome to rethink, rework, extend, or improve individual parts in their own way. That is also part of the idea: the project should remain understandable and suitable for further development. Some of the board capabilities were also intentionally left unused, such as pogo-pin contacts connected to free MCU GPIOs for integrating additional modules, as well as the built-in 16 MB memory. All of this leaves even more room for customization and extension.

## Capabilities

- **Composite USB device (HID + MSC)** — TinyRat is always visible as a keyboard (and mouse), while access to the SD card is enabled only in disk mode.
- **File and resource handling via FatFS** — scripts, images, and other data are read from the filesystem instead of being fully baked into the firmware.
- **Scenario application (`scen_app`)** — displays a list of text-based scenarios from the SD card, allowing the user to scroll through them, choose one, and launch the required automaton scenario.
- **Interactive inject mode** — executes the selected scenario by reading it line by line from a file and sending commands to the host as a USB HID keyboard.

## Architectural focus

The most interesting part of the project is the combination of several tasks inside one small device:

- embedded UI and state transitions;
- application architecture;
- USB descriptors and mode behavior;
- filesystem-backed resources;
- practical BadUSB scenarios.

For this project, that combination matters more than perfect implementation polish.

## Build

The repository already contains `CMakeLists.txt` and `pico_sdk_import.cmake`, so if the Pico SDK and the **Raspberry Pi Pico Project** extension in VS Code are configured correctly, it should be enough to open the project through the plugin and build it from there.

A typical manual Pico SDK flow should also look familiar:

```bash
mkdir build
cd build
cmake ..
make -j
```

If you just want to get started, there is already a `.uf2` file in the `extra` folder. Copy it to the board while it is connected in BOOT mode, then unpack the `resourses.tar` archive, copy its contents to the root of the SD card, insert the card, and the device is ready to use.

## Stack

- **MCU:** RP2040
- **LCD controller:** ST7789v
- **Touchscreen controller:** CST816S
- **Language:** C
- **Build:** CMake + Pico SDK
- **USB stack:** TinyUSB
- **Filesystem:** FatFS
- **Project style:** modular embedded platform with an app-like architecture

## System structure

The project is organized not as one large script, but as a small platform split into several layers:

- **hardware layer** — drivers for the display, touch controller, SD card, and other peripheral components;
- **system / firmware layer** — core platform logic, shared services, and the main system loop;
- **UI layer** — windows, layers, resource rendering, and screen transitions;
- **application layer** — separate applications launched from the launcher;
- **storage / USB layer** — handling of files, resources, and interaction with the host over USB.

## Runtime logic

After startup, the device initializes the core subsystems and enters the main system loop. The base firmware provides applications with a shared runtime: screen, input events, access to resources and the filesystem, as well as USB behavior.

The main entry point for the user is the launcher. It displays the available applications as separate interface entities and allows them to be launched directly on the device, without external control software.

## UI model

The UI is built around `layer` and `sublayer` structures, with their list implemented as a stack. This makes it possible not just to redraw the entire screen every time, but to compose it from separate logical parts: the base background, overlay elements, confirmation buttons, and other visual blocks.

Any operation that changes the layer tree (`create_layer()`, `add_sublayer()`, `pop_node()`, `collapse_list()`) immediately triggers a redraw through `build_with_res()`, which renders the root node and its sublayers. If `res_name != NULL`, an image is loaded into `framebuf` and passed to `load_buf()`; otherwise, a colored rectangle is drawn.

A typical scenario looks like this: the main application screen exists as a base layer, while temporary interface elements — for example, a confirm overlay — are added on top of it as a separate layer with its own button sublayer. When the overlay is closed, the underlying screen conceptually remains the same screen rather than being rebuilt as a completely different mode.

If necessary, a number of functions for direct rendering to the display can also be used.

## Adopted conventions

The project contains several intentionally adopted constraints that simplify behavior and help avoid mistakes:

- every regular `layer` may have its own `sublayer`;
- a `sublayer` is structurally similar to a normal `layer`, but must not have its own `sublayers`;
- this limitation is intentional, so that ownership, memory cleanup, and the lifecycle of UI nodes remain manageable;
- attempting to build nesting deeper than one level may lead to incorrect deallocation and memory leaks;
- an overlay layer is expected to logically capture input, so touches do not pass through to the screen below;
- applications use a shared lifecycle architecture: enter, event handling, periodic tick, and exit;
- application switching is performed through `app_open`, which does not allow launching an application on top of itself; if an application needs to be restarted, `app_name.exit()` and `app_name.enter()` should be used, or the state stored in `app_name.ctx` should be reset inside `app_name.enter()`;
- coordinates returned by the touch controller may differ from the real touch positions on a given display, so they are mapped using empirically selected correction values;
- when handling events, hardware buttons have priority over touch input;
- the SD card must be SDHC and formatted as FAT32 MBR;
- simultaneous access to the SD card by both the device and the connected computer is forbidden because of instability and the risk of filesystem corruption.

## Inject scenarios

From the system point of view, a scenario is a plain text file read line by line through FatFS.

Scenarios are stored in the directory: `/apps/scen_app/scripts`.

There are two types of lines:
- Plain text — the line is sent as a sequence of character key presses.
- `KEY...` command — the line is interpreted as a set of special keys and/or modifiers (if you need a literal text sequence starting with `KEY`, simply put a space at the beginning of the line).

After each line is sent, the system waits 1500 ms so the host has time to process the action.

Syntax: `KEY <special keys and modifiers>[+<regular characters>]`

Before `+`, the following modifiers are supported:

- `W` → Left GUI / Win / Meta;
- `S` → Left Shift;
- `C` → Left Ctrl;
- `A` → Left Alt.

Before `+`, the following special keys are also supported:

- `1..0` → F1..F10;
- `-` → F11;
- `=` → F12;
- `E` → Enter;
- `e` → Escape;
- `T` → Tab;
- `D` → Delete;
- `B` → Backspace;
- `s` → Space;
- `I` → Insert;
- `P` → Print Screen;
- `{` → Arrow Up;
- `}` → Arrow Down;
- `[` → Arrow Left;
- `]` → Arrow Right;
- `^` → Page Up;
- `v` → Page Down;
- `H` → Home;
- `N` → End;
- `X` → Caps Lock;
- `M` → Menu.

Important note: every line in the text scenario file ends with `\n` when moving to the next line, which is interpreted as an ENTER key press. Because of that, one complete action should be written on a single line, and sending a separate ENTER command is usually not required (in command-line interfaces, `\n` is ignored).

Examples:

- `KEY W+r` — opens the "Run" dialog in Windows
- `notepad` — types the string into the current window
- `Hello From TinyRat!` — types the string into Notepad
- `KEY C+s` — opens the save dialog
- `KEY E` — presses the ENTER key
- `KEY A4` — closes the Notepad window

## Resources

To create your own resources, use `conv2fullscreen` and `conv2small` from the `extra` folder. These utilities are intended to convert full-screen and smaller images respectively into the resource format used by the project — a sequence of RGB565 pixels.

Supported input formats are `.jpg`, `.jpeg`, and `.png`.

- **Usage:** run `./conv2small resource.png resource.res` in the command line
- **Important:** for full-screen resources, images should be prepared in advance and resized to `240x280`
- For `conv2small`, other dimensions may be used as long as they are smaller than `240x280`
