# Title Effects Design v2 (`TITLE_V2.md`)

This document details the design for a sophisticated, integrated effect called `RainAndConvergeEffect` and explains how to select it via configuration. This design supersedes `FALLING.md`.

## 1. High-Level Goal

To create a scene where specific streams of the Cyberrain seamlessly transform into a title. This requires a single effect that is aware of both the general rain and the specific title characters, ensuring a smooth and authentic transition.

## 2. Configuration: Choosing the Animation

To maintain modularity, the user will choose the animation via the `matrix.toml` file. The `ConfigLoader` will be responsible for reading this and constructing the correct effect(s).

### Example 1: Pure Cyberrain Scene

This configuration will create and run only the simple `RainEffect`.

```toml
# matrix.toml
[scene]
animation = "rain" # Choose the simple rain animation

[rain]
duration = 0 # Runs forever
# ... other rain settings ...
```

### Example 2: Rain-to-Title Scene

This configuration will create and run the new, integrated `RainAndConvergeEffect`.

```toml
# matrix.toml
[scene]
animation = "rain_and_converge" # Choose the integrated title animation

[rain_and_converge]
title = "THE MATRIX"

# Rain-specific settings for this animation
rain_duration = 10.0
slantAngle = 10.0
# ... other rain settings ...

# Convergence-specific settings
convergence_duration = 5.0 # Time over which letters appear and lock
convergence_randomness = 0.3 # Higher values increase the variation in arrival times
```

## 3. New Effect: `RainAndConvergeEffect`

This single, powerful effect manages the entire lifecycle of the rain-to-title animation.

### 3.1. Modified Data Structure: `RainStream`

We will add a state and optional target information to the `RainStream` struct that this effect uses.

```cpp
// Inside RainAndConvergeEffect.h
struct RainStream {
    // --- Standard Rain Properties ---
    float x, y, speed;
    int length, maxLength;
    std::vector<char32_t> characters;
    bool hasLeadChar;

    // --- New State & Title Properties ---
    enum class State { NORMAL, CONVERGING, IN_PLACE };
    State state = State::NORMAL;

    char32_t titleChar = U' ';
    float targetY = 0.0f;
};
```

### 3.2. `RainAndConvergeEffect` Class Design

```cpp
class RainAndConvergeEffect : public Effect {
public:
    RainAndConvergeEffect(Config& config);

    void update(const Context& context) override;
    void render(const Context& context) override;
    bool isFinished() const override;

private:
    void initializeStreams(const Context& context);

    Config config_;
    std::vector<RainStream> streams_;
    bool allInPlace_ = false;
};
```

### 3.3. Detailed Logic

1.  **Constructor & Initialization**:
    -   The constructor receives a configuration object containing both rain and title settings.
    -   On the first `update` call, `initializeStreams` is run.
    -   It creates rain streams covering the full terminal width.
    -   It then identifies the streams whose `x` coordinates correspond to the columns of the title characters.
    -   For these specific streams, it sets their `state` to `CONVERGING`, assigns the correct `titleChar` from the config, and calculates their `targetY` (the final row for the title).

2.  **`update(const Context& context)`**:
    -   Loop through every `stream` in `streams_`.
    -   If `stream.state` is `NORMAL`, update its position and shimmer its characters as a standard rain stream.
    -   If `stream.state` is `CONVERGING`:
        -   Update its `y` position downwards.
        -   Force the head of the stream (`stream.characters[0]`) to be the `stream.titleChar`.
        -   The rest of the stream's characters continue to shimmer randomly.
        -   If `stream.y >= stream.targetY`, change its state to `IN_PLACE` and set `stream.y = stream.targetY`.
    -   If `stream.state` is `IN_PLACE`, do nothing.
    -   After the loop, check if all `CONVERGING` streams are now `IN_PLACE`. If so, set the internal `allInPlace_` flag to true.

3.  **`render(const Context& context)`**:
    -   Loop through every `stream` in `streams_`.
    -   If `stream.state` is `NORMAL` or `CONVERGING`, render it as a full, multi-character rain stream. The `CONVERGING` streams will naturally display the title character at their head.
    -   If `stream.state` is `IN_PLACE`, render **only** the `stream.titleChar` at its final position (`stream.x`, `stream.targetY`). Do not render the rest of its tail.

4.  **`isFinished() const`**:
    -   This method returns `true` only when the `allInPlace_` flag is true, signaling that the entire title has been formed and locked in.

This integrated design perfectly achieves the desired visual of characters emerging from the rain while preserving our modular, option-based architecture.
