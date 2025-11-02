# Cyberrain Effect Design (`RAIN.md`)

This document provides a detailed technical design for the `RainEffect` class, which generates the primary "Cyberrain" visual.

## 1. High-Level Description

The Cyberrain effect emulates the iconic digital rain from The Matrix. It consists of multiple vertical or slanted streams of characters that fall down the screen. Each stream has a bright leading character, followed by a tail of dimmer characters that fade to black. The effect is highly configurable, allowing control over speed, color, character sets, and angle.

## 2. Core Data Structures

Two primary structs define the state and configuration of the effect.

### `RainConfig`

This struct holds all user-configurable properties for the rain effect. An instance of this will be passed to the `RainEffect` upon its construction.

```cpp
struct RainConfig {
    // --- Behavior ---
    // The angle of the rain in degrees. 0 is vertical, positive values slant to the right.
    float slantAngle = 0.0f;
    // The duration in seconds the effect should run. A value of 0 or less means it runs indefinitely.
    float duration = 0.0f;

    // --- Stream Properties ---
    // The min/max speed for new streams, in terminal cells per second.
    float minSpeed = 5.0f;
    float maxSpeed = 15.0f;
    // The min/max length for new streams.
    int minLength = 5;
    int maxLength = 20;

    // --- Visuals ---
    // Path to a UTF-8 encoded text file containing the characters to use for the rain.
    std::string characterSetFile = "katakana.txt";
    // The 32-bit RGBA channel for the leading character of a stream.
    uint32_t leadCharColor = 0xFFFFFFFF; // Bright White
    // The 32-bit RGBA channel for the brightest character in a stream's tail.
    uint32_t tailColor = 0x00FF00FF;     // Bright Green

    // --- Internal Data ---
    // This vector is populated at runtime by loading the characterSetFile.
    std::vector<char32_t> characterSet;
};
```

### `RainStream`

This struct represents a single stream of falling characters.

```cpp
struct RainStream {
    // --- State & Position ---
    // The precise column and row of the stream's head. Using float allows for smooth diagonal movement.
    float x, y;
    // The vertical speed of this specific stream, chosen from the config's min/max range.
    float speed;
    // The current length of the stream's tail.
    int length;
    // The maximum length this stream can grow to.
    int maxLength;
    // A flag indicating that the stream has moved off-screen and is ready to be reset.
    bool markedForReset = false;

    // --- Characters & Visuals ---
    // The sequence of characters that make up the stream's tail.
    std::vector<char32_t> characters;
    // Determines if the head of this stream should be rendered with the bright `leadCharColor`.
    bool hasLeadChar = true;
};
```

## 3. Class Definition: `RainEffect`

This class inherits from the base `Effect` and orchestrates the entire Cyberrain visual.

```cpp
// In src/effects/RainEffect.h
class RainEffect : public Effect {
public:
    // Constructor takes a configuration object.
    RainEffect(RainConfig config);

    // Called by the Engine every frame to update stream positions and states.
    void update(const Context& context) override;

    // Called by the Engine every frame to draw the streams to the screen.
    void render(const Context& context) override;

    // Signals to the Engine when the effect is finished (based on the `duration` config).
    bool isFinished() const override;

private:
    // Helper function to initialize or re-initialize a stream's state.
    void resetStream(RainStream& stream, const Context& context);

    RainConfig config_;
    std::vector<RainStream> streams_;

    // The horizontal velocity, calculated once from the slantAngle and stored for efficiency.
    float x_velocity_factor_;

    // The time point when the effect was created, used to check the duration.
    std::chrono::steady_clock::time_point startTime_;
};
```

## 4. Lifecycle and Logic

### Initialization (Constructor)

1.  The `RainConfig` object is copied into `config_`.
2.  The `startTime_` is recorded using `std::chrono::steady_clock::now()`.
3.  The `x_velocity_factor_` is calculated from `config_.slantAngle`.
4.  The `streams_` vector is populated with an initial set of `RainStream` objects. The number of streams can be based on the screen width.
5.  `resetStream()` is called on every stream in the vector to give each one a random starting position, speed, and length.

### `update(const Context& context)`

This method is purely for state management, not drawing.

1.  For each `stream` in `streams_`:
2.  If `stream.markedForReset` is `true`, call `resetStream(stream, context)` and skip to the next stream.
3.  Update stream position based on time delta: `stream.y += stream.speed * context.deltaTime;` and `stream.x += (stream.speed * x_velocity_factor_) * context.deltaTime;`.
4.  With a small probability, randomly change one of the characters in `stream.characters` to a new character from `config_.characterSet`. This creates a subtle "shimmer" effect.
5.  Check if the stream has moved entirely off-screen. If `(stream.y - stream.length) > context.rows`, set `stream.markedForReset = true`.

### `render(const Context& context)`

This method handles all drawing to the `ncplane` provided in the `context`.

1.  The effect's dedicated `ncplane` is cleared.
2.  For each `stream` in `streams_`:
3.  Loop `i` from `0` to `stream.length - 1`.
4.  Calculate the integer screen coordinates for the character: `int screen_y = static_cast<int>(stream.y) - i;`, `int screen_x = static_cast<int>(stream.x);`.
5.  If the coordinates are outside the plane's bounds, skip this character.
6.  **Determine Color**: The character's color is determined by its position (`i`) in the stream. If `i == 0` and `hasLeadChar` is true, use `config_.leadCharColor`. Otherwise, create a gradient that fades from `config_.tailColor` to black as `i` increases. This can be done by decreasing the green and alpha components of the channel for each step `i`.
7.  Draw the character to the plane using `ncplane_putwchar_yx()`.

### `isFinished() const`

1.  If `config_.duration <= 0`, this method always returns `false`.
2.  Otherwise, it calculates the elapsed time since `startTime_`.
3.  It returns `true` if the elapsed time is greater than `config_.duration`, `false` otherwise.

## 5. Context Dependencies

The `RainEffect` requires the following fields to be properly set in the `Context` object it receives from the Engine:

- `ncplane* effect_plane`: A dedicated plane for this effect to draw on.
- `unsigned int rows, cols`: The current dimensions of the terminal.
- `float deltaTime`: The time in seconds since the last `update` call.
- `std::mt19937* rng`: A pointer to a random number generator for all randomization tasks.
