# Title Effects Design (`TITLE.md`)

This document provides a detailed technical design for the title effects, including `ConvergeToTitleEffect` and `TitleHoldEffect`, and explains how they are managed by the Engine.

## 1. High-Level Goal

The goal is to create a scene where the Cyberrain appears to "resolve" into a title. This requires a mechanism for running multiple effects concurrently and in sequence. The user must be able to control this behavior from the main configuration file.

## 2. Scene Configuration (`matrix.toml`)

To control the sequence of effects, we will introduce a new `[scene]` table in `matrix.toml`. This table will define which effects to load and in what order.

### Example 1: Pure Cyberrain Scene

This configuration will run only the `RainEffect` indefinitely.

```toml
# matrix.toml
[scene]
effects = ["rain"]

[rain]
duration = 0 # Runs forever
# ... other rain settings ...
```

### Example 2: Rain-to-Title Scene

This configuration will run the `RainEffect` and `ConvergeToTitleEffect` at the same time, and once they are finished, it will run the `TitleHoldEffect`.

```toml
# matrix.toml
[scene]
effects = ["rain_and_converge", "hold"]

# This table configures the first stage of the scene
[rain_and_converge]
# Config for the RainEffect part
rain_duration = 10.0 # Rain will stop after 10 seconds

# Config for the ConvergeToTitleEffect part
title = "THE MATRIX"
convergence_delay = 3.0  # Title characters start appearing after 3 seconds
convergence_duration = 5.0 # It takes 5 seconds for all characters to fall into place

# This table configures the second stage of the scene
[hold]
duration = 5.0 # Hold the completed title for 5 seconds
```

## 3. Engine Modifications

The `Engine` will be updated to understand this scene configuration. When `run()` is called, it will:
1.  Load the first effect (or group of effects) specified in the `[scene].effects` array.
2.  Run the effects until their `isFinished()` methods all return `true`.
3.  Once a stage is complete, it will load the next effect(s) from the sequence.
4.  The application exits when the last stage is complete.

## 4. Effect Design: `ConvergeToTitleEffect`

This is the most complex effect. It runs concurrently with `RainEffect` and creates the illusion of characters emerging from the rain.

### Data Structure: `TitleParticle`

Each character in the final title will be an independent particle.

```cpp
struct TitleParticle {
    // Final State
    char32_t finalChar;
    float finalX, finalY;

    // Animation State
    float currentY;
    float speed;
    float delay; // Time before this particle starts falling
    enum { WAITING, FALLING, IN_PLACE } state;
};
```

### Logic

-   **Constructor**: Takes the title string and convergence duration. It calculates the `finalX` and `finalY` for each character to center the title. It then creates a `TitleParticle` for each character, assigning it a random `speed` and a `delay` distributed over the `convergence_duration`.
-   **`update(Context& context)`**: For each particle, it checks its state:
    -   `WAITING`: If `elapsed_time > delay`, switch to `FALLING`.
    -   `FALLING`: Move `currentY` downwards. If `currentY` passes `finalY`, switch to `IN_PLACE` and snap the particle to its final coordinates.
-   **`render(Context& context)`**: For each particle:
    -   `WAITING`: Draw nothing.
    -   `FALLING`: Draw the `finalChar` at `(finalX, currentY)` with a bright color. To enhance the illusion, it can also draw a short, fast-fading trail of random characters above it.
    -   `IN_PLACE`: Draw the `finalChar` at its `finalX, finalY` position with a solid, bright color.
-   **`isFinished()`**: Returns `true` only when all `TitleParticle`s are in the `IN_PLACE` state.

## 5. Effect Design: `TitleHoldEffect`

This is a much simpler effect that runs after `ConvergeToTitleEffect` is finished.

### Logic

-   **Constructor**: Takes the title string and a `duration`.
-   **`update()`**: Does nothing.
-   **`render(Context& context)`**: Calculates the centered position for the entire title string and draws it to the plane.
-   **`isFinished()`**: Returns `true` after its `duration` has elapsed.
