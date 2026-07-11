# TinyRat

> A first serious attempt to build a custom embedded platform on RP2040: with UI, applications, USB workflows, and file-based resources.

[Русская версия](./README.md)

---

## About the project

**TinyRat** is an RP2040-based embedded system with its own graphical launcher, modular applications, USB device functionality, and file-backed resources. The device is built on top of a creative BadUSB-style board designed by the RoboLab of the Moscow campus of School 21 (Sber).

You can find their other hardware projects in their [GitHub organization](https://github.com/RoboLab21), and see the detailed schematic of this board [here](https://github.com/Freimor/HRDW-Automaton_v3).

The goal of TinyRat was not to create a single firmware demo, but to build a small standalone platform with its own UI, app launching logic, and real interaction with the outside world via USB and files.

This repository combines several directions in a single project:

- embedded firmware architecture;
- lightweight on-device UI;
- application-based design;
- USB composite workflows;
- storage-backed resource loading;
- keyboard automation workflows.

## Why this was built

This project was designed as a portfolio piece and at the same time as a real learning experience. It is the first serious attempt to work with hardware at this level, so some parts are implemented in a very straightforward way and not always in the most “perfect” manner.

It is important to state this openly: there were probably more correct, more elegant, or more efficient ways to implement some of the solutions. However, this repository shows a live working system, actual integration of multiple subsystems, and an honest attempt to assemble a coherent embedded product rather than a purely toy example.

Anyone is welcome to rethink, refactor, extend, or improve individual parts in their own way. That is part of the idea: the project is meant to be understandable, hackable, and suitable as a base for further development. Some of the hardware capabilities of the board are intentionally left unused, such as pogo pins connected to free MCU GPIOs for adding extra modules, or the built‑in 16 MB of flash. All of this opens up additional room for customization and extensions.

## Features

- **RP2040 firmware** with a modular application lifecycle.
- **Launcher-style UI** for starting applications directly on the device.
- **Resource and file handling** for storing scripts, assets, and application data.
- **USB device mode** with real host interaction scenarios.
- **Composite USB behavior** for keyboard automation and file transfer workflows.
- **TinyUSB-based implementation**, using slightly adapted TinyUSB examples integrated into this project structure.
- **FatFS integration** for file and resource access.
- **Scripted automation workflow** for inject / payload-oriented scenarios.

## Architectural focus

The most interesting part of the project is the combination of several concerns inside a single small device:

- embedded UI and state transitions;
- application architecture;
- USB descriptors and mode behavior;
- filesystem-backed resources;
- practical automation workflows.

This combination is more important here than perfect implementation polish. The value of the project is in demonstrating system thinking across firmware, data storage, small-device UX, and host–device interaction.

## Build

The repository already contains `CMakeLists.txt` and `pico_sdk_import.cmake`, so it is structured as a standard Pico SDK CMake project.

If the Pico SDK and the **Raspberry Pi Pico Project** extension in VS Code are configured correctly, it is usually enough to open the project via the plugin and build it from there.

A typical manual flow for Pico SDK looks like this:

```bash
mkdir build
cd build
cmake ..
make -j
```

If you just want to get started, the compiled `.uf2` file will be located in the `build` directory. Copy this file to the board while it is in BOOT mode, then unpack the `resourses.tar` archive, copy its contents to the root of the SD card, insert the card into the device, and you are ready to use TinyRat.

## Stack

- **MCU:** RP2040
- **LCD controller:** ST7789v
- **Touchscreen controller:** CST816S
- **Language:** C
- **Build system:** CMake + Pico SDK
- **USB stack:** TinyUSB
- **Filesystem:** FatFS
- **Project style:** modular embedded platform with an app-like architecture

## System structure

The project is not a single monolithic script, but a small platform split into several conceptual layers:

- **hardware layer** — drivers for the display, touch controller, SD card and other peripherals;
- **system / firmware layer** — core platform logic, shared services, system loop;
- **UI layer** — windows, layers, resource rendering, screen transitions;
- **application layer** — individual applications launched from the system launcher;
- **storage / USB layer** — file and resource handling, and USB interaction with the host.

## Execution model

On startup, the device initializes core subsystems and enters the main system loop. The base firmware provides applications with a shared runtime: screen, input events, access to resources and the filesystem, as well as USB behavior.

The central entry point for the user is the launcher. It displays available applications as separate UI entities and lets you start them directly on the device, without any external control software.

## UI model

The UI is built around `layer` and `sublayer` structures arranged as a stack. This makes it possible not just to redraw the entire screen from scratch every time, but to assemble it from logical pieces: a base background, overlay elements, confirmation buttons, and other visual blocks.

Any operation that changes the layer tree (`create_layer()`, `add_sublayer()`, `pop_node()`, `collapse_list()`) immediately triggers a redraw (`build_with_res()`), which renders the root node and its sublayers. If `res_name != NULL`, the corresponding image is loaded into `framebuf` and passed to `load_buf()`. Otherwise, a solid-colored rectangle is drawn.

A typical pattern looks like this: the main application screen exists as a base layer, while temporary UI elements — such as a confirmation overlay — are added on top of it as a separate layer with its own button sublayer. When the overlay is closed, the underlying screen conceptually remains the same screen, not a different “mode” rebuilt from scratch.

## Conventions

The project intentionally follows a set of constraints that keep the behavior predictable and help avoid subtle bugs:

- every regular `layer` may have its own `sublayers`;
- a `sublayer` is structurally similar to a regular `layer`, but must not have its own `sublayers`;
- this limitation is intentional, to keep ownership, memory freeing, and UI node lifecycle simple and safe;
- trying to build deeper nesting than one sublayer level may lead to incorrect memory freeing and leaks;
- an overlay layer is expected to logically intercept input, so touches and taps do not “leak through” to the underlying screen;
- applications follow a shared lifecycle pattern: `enter`, event handling, periodic `tick`, and `exit`;
- switching applications is done via `app_open`, which prevents launching the same application on top of itself; if an app needs to be restarted, `app_name.exit()` and `app_name.enter()` should be called, or the application data context (`app_name.ctx`) should be reset inside `app_name.enter()`;
- coordinates reported by the touch controller may differ from the physical touch positions on your display, so they are mapped using empirically tuned corrections;
- when handling events, hardware buttons take precedence over touch input;
- the SD card must be an SDHC card formatted as FAT32 with MBR;
- simultaneous access to the SD card from both the device and the host computer is forbidden due to instability and the risk of filesystem corruption.

## Inject scripts

From the system’s point of view, an inject script is just a plain text file read line by line via FatFS. Each line is either printed as text or interpreted as a keyboard command if it starts with `KEY` (if you really need a literal `KEY` at the beginning of a line, just add a leading space).

Scripts are stored in the directory: `/apps/scen_app/scripts`.

There are two types of lines:

- plain text — the line is sent as a sequence of character key presses;
- `KEY...` command — the line is interpreted as a combination of special keys and/or modifiers.

After each line is sent, the system waits 1500 ms to allow the host OS to process the actions.

Syntax: `KEY <special keys and modifiers>[+<regular characters>]`

Before the `+` sign, the following modifiers are supported:

- `W` → Left GUI / Win / Meta;
- `S` → Left Shift;
- `C` → Left Ctrl;
- `A` → Left Alt.

Before the `+` sign, the following special keys are supported:

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

Important note: each line in the script file ends with `\n`, which is interpreted as an Enter keypress. That means a single logical action should be written on a single line, and you usually do not need a separate explicit `KEY E` just to send Enter.

Examples:

- `KEY W+r` — open the “Run” dialog on Windows;
- `notepad` — type this line into the current input field;
- `Hello From TinyRat!` — type this line (for example into Notepad);
- `KEY C+s` — open the “Save” dialog;
- `KEY E` — press Enter;
- `KEY A4` — close the Notepad window.