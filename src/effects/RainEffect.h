#pragma once

#include "engine/Effect.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

struct RainConfig {
    // The angle of the rain in degrees. 0 is vertical.
    float slantAngle{0.0f};
    // Duration in seconds. 0 or less means indefinite.
    float duration{0.0f};

    float minSpeed{5.0f};
    float maxSpeed{15.0f};
    int minLength{5};
    int maxLength{20};

    std::string characterSetFile{"katakana.txt"};
    uint32_t leadCharColor{0xFFFFFFFF};
    uint32_t tailColor{0x00FF00FF};

    std::vector<char32_t> characterSet{};
};

struct RainStream {
    float x{0.0f};
    float y{0.0f};
    float speed{0.0f};
    int length{0};
    int maxLength{0};
    bool markedForReset{false};
    std::vector<char32_t> characters{};
    bool hasLeadChar{true};
};

class RainEffect : public Effect {
public:
    explicit RainEffect(RainConfig config);

    void update(const Context& context) override;
    void render(const Context& context) override;
    bool isFinished() const override;

private:
    void ensure_initialized(const Context& context);
    void resetStream(RainStream& stream, const Context& context);
    char32_t random_character(std::mt19937& rng) const;
    void ensure_character_set_loaded();

    RainConfig config_;
    std::vector<RainStream> streams_{};
    float x_velocity_per_unit_y_{0.0f};
    std::chrono::steady_clock::time_point start_time_{};
    bool initialized_{false};
};

