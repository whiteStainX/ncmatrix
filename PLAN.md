# ncmatrix Development Plan

This document outlines the phased development plan to build `ncmatrix`.

### Phase 1: Core Engine & Infrastructure

- [x] Set up the project structure with CMake.
- [x] Integrate `notcurses`, `cxxopts`, and `toml++` libraries.
- [x] Create a basic `Engine` class to manage the `notcurses` lifecycle.
- [x] Implement a main loop that can handle basic input (quit on 'q').
- [x] Confirm a stable build and run process.

### Phase 2: The Effect System

The goal of this phase is to build the abstract machinery for managing effects.

- [x] Define `Effect` interface with `update()` and `render()`.
- [x] Define `Context` struct for shared data.
- [x] Evolve the initial `Renderer` into a full `Engine` that manages a list of effects.
- [x] Update the `Engine`'s main loop to iterate and call `update()`/`render()` on all effects.
- [x] Add `isFinished()` to the base `Effect` interface.
- [x] Update the `Engine` to check `isFinished()` and remove completed effects from the active list.

### Phase 3: The Cyberrain Effect

With the effect system in place, we will create the first concrete visual effect.

- [ ] Create `RainEffect.h` and `RainEffect.cpp`.
- [ ] Define the `RainStream` and `RainConfig` data structures inside `RainEffect.h`.
- [ ] Implement the basic vertical rain logic in `update()` (positional changes) and `render()` (drawing).
- [ ] Implement color gradients for the stream tails (e.g., fading from green to black).
- [ ] Add a `duration` property to the effect's configuration and implement the `isFinished()` logic.
- [ ] Add a `slantAngle` property to the configuration to enable slanted rain.

### Phase 4: Title Effects

This phase implements the title-forming capabilities.

- [ ] Create `ConvergeToTitleEffect` responsible for animating characters into a final title position.
- [ ] Create `TitleHoldEffect` for displaying a static title on screen.
- [ ] Update the `Engine` to manage sequences of effects (e.g., running Cyberrain, then Converge, then Hold).

### Phase 5: Configuration & Polish

This final phase will focus on making the application configurable and robust.

- [ ] Implement loading of character sets from user-specified text files via TOML config.
- [ ] Implement loading of `RainConfig` and other effect settings from a TOML file.
- [ ] Implement command-line argument parsing with `cxxopts` to override TOML settings.
- [ ] Ensure the `Engine` gracefully handles terminal window resizing.
- [ ] Final code cleanup, documentation, and performance review.