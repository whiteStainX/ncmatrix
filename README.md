# ncmatrix

A modern, extensible C++ digital rain effect generator for the terminal, inspired by cmatrix and The Matrix.

## Background and Goal

The original `cmatrix` is a classic, but its design is not optimized for extension. This project is a complete rewrite in modern C++20 with the primary goal of creating a highly modular and extensible system for generating terminal-based visual effects.

The core idea is to support a pluggable "Effect" architecture, making it trivial to add new capabilities beyond the standard digital rain. The initial driving use case is to create custom video openings by having characters from the rain converge to form a title.

## Features

- **Extensible by Design**: A clean, decoupled architecture allows for easily adding new visual effects.
- **Modern C++**: Written in C++20.
- **High-Performance**: Uses the `notcurses` library for high-performance, GPU-accelerated (where available) terminal graphics.
- **Configurable**: Control effects, colors, and timing via TOML configuration files and command-line arguments.

## Usage (Intended)

The application is designed to be controlled via a TOML configuration file and command-line overrides.

```bash
# Run with a specific configuration file
./build/ncmatrix --config my_scene.toml

# Override the title text from the command line
./build/ncmatrix --title "Hello, World!"

# Run the default digital rain effect
./build/ncmatrix
```

## Building from Source

### Dependencies

- A C++20 compatible compiler (e.g., GCC, Clang)
- [CMake](https://cmake.org/) (version 3.16+)
- [Notcurses](https://github.com/dankamongmen/notcurses)
- [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

### Build Steps

```bash
# Configure the project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build the executable
cmake --build build
```

The final executable will be located at `build/ncmatrix`.
