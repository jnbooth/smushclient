# SmushClient

A cross-platform MUD game client fashioned after [Nick Gammon's MUSHclient](https://www.gammon.com.au) with MUSHclient API compatibility, built using Rust and the Qt framework.

![image](https://github.com/user-attachments/assets/04e672e7-65c6-46b6-9e12-a9c1a7b254e5)

_Shown above: SmushClient running [Quow's MUSHclient plugin pack](https://quow.co.uk/mushclient.php), which was developed for the MUSHclient API._

| ![image](https://github.com/user-attachments/assets/a61c4d52-f2a5-48f3-a4a0-9e95810d7218) | ![image](https://github.com/user-attachments/assets/22df296d-7192-4b15-812a-2a4e9fb3f8a6) |
| ----------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |

# Local Development

## Prerequisites

- [Rust](https://doc.rust-lang.org/cargo/getting-started/installation.html)
- [Qt framework](https://doc.qt.io/qt-6/get-and-install-qt.html)
- [CMake](https://cmake.org/download/)

## Building for Development

```console
cd smushclient-qt
cmake -S . -B build/Debug
cmake --build build/Debug --config Debug
```

## Building for Release

```console
cd smushclient-qt
cmake -S . -B build/Release
cmake --build build/Relase --config Release
```

## Running

SmushClient looks for resources according to the following directory structure:

| Path             | Description              |
| ---------------- | ------------------------ |
| `<DIR>/lua/`     | Lua scripts              |
| `<DIR>/plugins/` | Client plugins (.XML)    |
| `<DIR>/sounds/`  | Sound files (any format) |
| `<DIR>/worlds/`  | World files              |

# Related Links

- [nickgammon/mushclient](https://github.com/nickgammon/mushclient): [Nick Gammon](https://www.gammon.com.au/)'s legendary MUSHclient application, created in 1995.
- [jnbooth/mxp](https://github.com/jnbooth/mxp): Rust libraries for parsing the [MXP protocol](https://www.zuggsoft.com/zmud/mxp.htm). Comprises the majority of SmushClient's back end.
- [jnbooth/lua-batteries-included](https://github.com/jnbooth/lua-batteries-included): CMake config used by SmushClient for its Lua libraries.
- [KDAB/cxx-qt](https://github.com/KDAB/cxx-qt): FFI system for using Rust libraries in C++ Qt.
- [quow.co.uk](https://quow.co.uk/): Home of Quow's MUSHclient plugin pack for [Discworld MUD](http://discworld.starturtle.net/lpc/), which inspired this project.
