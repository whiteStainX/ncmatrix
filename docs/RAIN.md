# Cyberrain Effect Documentation

This document describes the Cyberrain visual effect, its behavior, and how to configure it.

## 1. Overview

The Cyberrain effect emulates the iconic digital rain from The Matrix. It consists of multiple vertical or slanted streams of characters that fall down the screen. Each stream features a bright leading character, followed by a tail of dimmer characters that gradually fade out. The effect is highly configurable, allowing users to control its appearance and behavior.

## 2. Behavior

-   **Falling Streams**: Characters continuously fall down the screen, creating a dynamic rain effect.
-   **Shimmering**: Characters within the streams subtly change over time, adding to the organic feel of the rain.
-   **Slanted Rain**: Streams can be configured to fall at an angle, mimicking a more dynamic visual.
-   **Lifecycle**: The effect can run indefinitely or for a specified duration, after which it will automatically signal its completion to the `ncmatrix` engine.

## 3. Configuration Options

The Cyberrain effect is configured via a `RainConfig` object, typically populated from the root-level `matrix.toml`. The effect looks for a section named `[effect.cyberrain]` and reads the following keys. If a key is omitted, the engine falls back to the `RainConfig` default noted below.

-   `slantAngle` (float, default: `0.0`)
    -   The angle of the rain in degrees. `0.0` results in vertical rain. Positive values will slant the rain to the right, and negative values to the left.
-   `duration` (float, default: `0.0`)
    -   The duration in seconds for which the effect should run. If set to `0.0` or a negative value, the effect will run indefinitely until manually stopped.
-   `minSpeed` (float, default: `5.0`)
    -   The minimum vertical speed for new rain streams, measured in terminal cells per second.
-   `maxSpeed` (float, default: `15.0`)
    -   The maximum vertical speed for new rain streams, measured in terminal cells per second.
-   `minLength` (int, default: `5`)
    -   The minimum length (number of characters) for new rain streams.
-   `maxLength` (int, default: `20`)
    -   The maximum length (number of characters) for new rain streams.
-   `density` (float, default: `0.5`)
    -   Controls the number of rain streams relative to the terminal width. A value of `0.5` means roughly half the terminal columns will have a rain stream. Higher values increase density.
-   `characterSetFile` (string, default: `"katakana.txt"`)
    -   The path to a UTF-8 encoded text file containing the characters to be used for the rain. This file should be located in the `assets/chars/` directory. Each character in the file will be treated as a potential rain character.
-   `leadCharColor` (uint32_t, default: `0xFFFFFFFF`)
    -   The 32-bit RGBA hexadecimal value for the color of the leading character of each rain stream (e.g., `0xFF0000FF` for bright red).
-   `tailColor` (uint32_t, default: `0x00FF00FF`)
    -   The 32-bit RGBA hexadecimal value for the brightest color in a rain stream's tail. The tail will fade from this color to black.

## 4. Example TOML Configuration

```toml
[effect.cyberrain]
slantAngle = 15.0
duration = 30.0
minSpeed = 8.0
maxSpeed = 25.0
minLength = 10
maxLength = 35
density = 0.7
characterSetFile = "numbers.txt"
leadCharColor = 0xFFFFFFAA
tailColor = 0x00AA00FF
```

The comments embedded in `matrix.toml` mirror these descriptions to make it easy to tweak settings inline.
