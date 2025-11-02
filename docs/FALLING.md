# Bridging the Visual Gap: The Falling Title Effect (`FALLING.md`)

This document outlines the gap between the current and desired visual for the `ConvergeToTitleEffect` and provides a detailed plan to implement the final, polished version.

## 1. The Gap: Current vs. Desired Effect

-   **Current State**: The `ConvergeToTitleEffect` currently renders only the single, final character of the title as it falls into place (e.g., the letter 'M'). While functionally correct, this looks like individual letters dropping from the sky, not like they are emerging from the digital rain.

-   **Desired Effect**: Each falling title character should act as the **leader** of its own short, temporary rain stream. As the final character falls, it should have a brief, fading trail of random characters above it. This creates the illusion that a specific stream from the Cyberrain is slowing down and resolving into a letter before locking into place.

## 2. Detailed Implementation Plan

To achieve the desired effect, we need to modify the `ConvergeToTitleEffect` to give each `TitleParticle` its own mini-trail. This involves changes to the data structure, the update logic, and a significant overhaul of the render method.

### 2.1. Data Structure Modification (`TitleParticle`)

First, we must update the `TitleParticle` struct in `ConvergeToTitleEffect.h` to include data for its trail.

**Action**: Add a vector for trail characters and a trail length to the `TitleParticle` struct.

```cpp
// In ConvergeToTitleEffect.h
struct TitleParticle {
    // ... (existing members) ...

    // New members for the trail
    int trailLength{3}; // A short, 3-character trail
    std::vector<char32_t> trailChars;
};
```

### 2.2. Initialization Logic (`ensure_particles_initialized`)

When a particle is created, we need to initialize its trail.

**Action**: In `ensure_particles_initialized`, after creating a particle, resize its `trailChars` vector and populate it with random characters. This can be done by calling the `random_character` helper from the `RainEffect` (we may need to expose a similar utility).

### 2.3. Update Logic (`update` method)

To make the trail feel alive, it should shimmer just like the main Cyberrain.

**Action**: In the `update` method, when a particle is in the `FALLING` state, add logic to randomly change one of the characters in its `trailChars` vector with a low probability on each frame.

### 2.4. Render Logic Overhaul (`render` method)

This is the most critical change. We need to replace the current single-character rendering with logic that draws the leader and its trail.

**Action**: Overhaul the `render` method with the following logic:

1.  For each `particle` that is `FALLING` or `IN_PLACE`:
2.  **Draw the Leader**: Draw the `particle.finalChar` at its current position (`finalX`, `currentY`) with the brightest color (e.g., white or bright green).
3.  **If the particle is `FALLING`**, proceed to draw its trail:
4.  Loop `i` from `1` to `particle.trailLength`.
5.  Calculate the trail character's position: `trail_y = particle.currentY - i`.
6.  **Calculate Color**: Determine the color for this trail segment. It should fade rapidly. For example, the character at `i=1` could be 50% of the `tailColor`, and the character at `i=3` could be 10%.
7.  Get the random character from `particle.trailChars[i-1]`.
8.  Draw the trail character at `(particle.finalX, trail_y)` with the calculated faded color.

## 3. Code-Level Blueprint

Here is a pseudo-code blueprint for the new `render` method:

```cpp
// In ConvergeToTitleEffect::render()

for (const auto& particle : particles_) {
    if (particle.state == TitleParticle::State::Waiting) {
        continue;
    }

    // 1. Draw the Leader Character
    const int leader_x = static_cast<int>(particle.finalX);
    const int leader_y = static_cast<int>(particle.currentY);
    // ... (check bounds) ...
    ncplane_set_fg_rgb8(context.root_plane, 0xEE, 0xFF, 0xEE); // Bright leader color
    ncplane_putegc_yx(context.root_plane, leader_y, leader_x, encode_utf8(particle.finalChar).c_str(), nullptr);

    // 2. If falling, draw the trail
    if (particle.state == TitleParticle::State::Falling) {
        for (int i = 1; i <= particle.trailLength; ++i) {
            const int trail_y = leader_y - i;
            // ... (check bounds) ...

            // Calculate a fast fade
            const float fade_factor = 1.0f - (static_cast<float>(i) / (particle.trailLength + 1));
            const uint8_t r = static_cast<uint8_t>(0x00 * fade_factor);
            const uint8_t g = static_cast<uint8_t>(0xAA * fade_factor); // Fading from dark green
            const uint8_t b = static_cast<uint8_t>(0x00 * fade_factor);
            ncplane_set_fg_rgb8(context.root_plane, r, g, b);

            // Get a random char from the particle's trail
            char32_t trail_char = particle.trailChars[i-1];
            ncplane_putegc_yx(context.root_plane, trail_y, leader_x, encode_utf8(trail_char).c_str(), nullptr);
        }
    }
}
```

By following this plan, the `ConvergeToTitleEffect` will produce a much more visually appealing and authentic effect that aligns with our original design goals.
