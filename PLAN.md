# ncmatrix Development Plan

This document outlines the phased development plan to build `ncmatrix`.

### Phase 1: Core Engine & Infrastructure (Complete)

The goal of this phase was to establish a solid foundation for the project.

- [x] Set up the project structure with CMake.
- [x] Integrate `notcurses`, `cxxopts`, and `toml++` libraries.
- [x] Create a basic `Renderer`/`Engine` class to manage the `notcurses` lifecycle.
- [x] Implement a main loop that can handle basic input (quit on 'q').
- [x] Confirm a stable build and run process.

### Phase 2: The Effect System

The goal of this phase is to build the abstract machinery for managing effects.

- **Define `Effect` Interface**: Create an abstract base class `Effect` with virtual methods like `update(Context&)` and `render()`.
- **Define `Context` Struct**: Create the `Context` struct that will be passed to effects, containing terminal dimensions and other shared data.
- **Evolve `Engine`**: Upgrade the `Renderer` into a full `Engine` class that manages a list of `Effect*` pointers.
- **Update Main Loop**: The engine's main loop will iterate through the list of effects, calling `update()` and `render()` on each one every frame.

### Phase 3: The Cyber Rain Effect

With the effect system in place, we will create the first concrete visual effect.

- **Create `RainEffect`**: Implement a new class `RainEffect` that inherits from `Effect`.
- **Implement Rain Logic**: This effect will manage multiple columns of falling characters.
  - Each column will have a "leader" character with a bright color.
  - Trailing characters will have a dimmer, fading color.
  - Columns will reset randomly to create a continuous, dynamic effect.

### Phase 4: Title Effects

This phase implements the title-forming capabilities.

- **Create `ConvergeToTitleEffect`**: This effect will be responsible for making characters appear to stop and form a title.
  - It will need to know the target title and its position.
  - It will animate characters into their final positions.
- **Create `TitleHoldEffect`**: A simpler effect that just displays a static piece of text. This can be used to hold the title on screen after it has been formed.
- **Orchestration**: The `Engine` will be updated to manage the sequence of these effects (e.g., run `RainEffect` and `ConvergeToTitleEffect` simultaneously, then switch to `TitleHoldEffect`).

### Phase 5: Configuration & Polish

This final phase will focus on making the application configurable and robust.

- **Load from TOML**: Implement `toml++` logic to load settings like colors, rain speed, and title text from a configuration file.
- **Command-Line Flags**: Use `cxxopts` to allow overriding file configurations with command-line arguments.
- **Handle Resizing**: Add logic to the `Engine` to gracefully handle terminal window resizing.
- **Code Cleanup**: Add documentation, comments, and refactor as needed to improve code quality.
