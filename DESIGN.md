# ncmatrix Design

This document outlines the high-level architecture for `ncmatrix`, a modern, extensible C++ digital rain effect generator.

## Core Principles

- **Extensibility**: The primary goal is to create a system where new visual effects can be added with minimal changes to the core application.
- **Decoupling**: Components should be self-contained and interact through well-defined interfaces, not by directly accessing each other's internal state.
- **Separation of Concerns**: The core rendering and application lifecycle should be separate from the logic of the visual effects themselves.

## Architecture Components

The system is built from three main components: the **Engine**, the **Context**, and **Effects**.

### 1. Engine

The `Engine` is the heart of the application. It is responsible for:

- **Lifecycle Management**: Initializing, running, and shutting down the `notcurses` library.
- **Main Loop**: Driving the application by processing input, updating state, and rendering frames.
- **Effect Management**: Maintaining a list of active `Effect` objects and orchestrating their execution.
- **Input Handling**: Capturing user input (e.g., quit commands, toggles) and dispatching actions accordingly.
- **Resource Management**: Owns the `notcurses` instance and other global resources.

The `Renderer` class we created is the first iteration of the `Engine`.

### 2. Context

The `Context` is a shared, read-only data structure that is passed to every `Effect` during the update and render cycles. It acts as a service locator and provides effects with the information they need to do their work, such as:

- Terminal dimensions (rows and columns).
- Global configuration settings.
- A shared random number generator (RNG) for deterministic behavior if needed.

This prevents effects from needing direct access to the `Engine` and keeps them decoupled.

### 3. Effect

An `Effect` is a self-contained, modular plugin that implements a specific piece of visual functionality. Each effect will adhere to a common interface (e.g., an abstract base class).

- **Lifecycle**: Effects have a defined lifecycle, with methods like `update(Context&)`, `render()`, and `isFinished()` that are called by the `Engine`. The `isFinished()` method allows an effect to signal that it has completed its work, enabling the Engine to remove it and potentially start another.
- **Isolation**: Each `Effect` is given its own `ncplane` to draw on. This is crucial, as it prevents effects from accidentally drawing over each other and simplifies rendering logic.

Example effects include:
- `RainEffect`: The classic digital rain.
- `ConvergeToTitleEffect`: Characters that stop to form a title.
- `TitleHoldEffect`: A static title display.

### Configuration

- **`cxxopts`**: Used for parsing command-line arguments for runtime configuration.
- **`toml++`**: Used for loading more complex, persistent configuration from files (e.g., `config.toml`). This includes loading resources like character sets from text files in the `assets/chars/` directory.
