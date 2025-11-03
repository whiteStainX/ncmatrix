#pragma once

#include "effects/RainEffect.h"

#include <chrono>
#include <random>
#include <string>
#include <vector>

struct RainAndConvergeConfig {
    RainConfig rainConfig{};
    std::u32string title{};
    float convergenceDuration{5.0f};
    float convergenceRandomness{0.0f};
    unsigned int titleRow{0};
};

class RainAndConvergeEffect : public Effect {
public:
    explicit RainAndConvergeEffect(RainAndConvergeConfig config);

    void update(const Context& context) override;
    void render(const Context& context) override;
    bool isFinished() const override;

private:
    struct ExtendedRainStream : RainStream {
        enum class State { NORMAL, CONVERGING, IN_PLACE };

        State state{State::NORMAL};
        char32_t titleChar{U' '};
        float targetY{0.0f};
        bool isTitleStream{false};
        float convergenceElapsed{0.0f};
        bool allowRespawn{true};
        bool inactive{false};
    };

    void ensure_character_set_loaded();
    void ensure_initialized(const Context& context);
    void initialize_streams(const Context& context);
    void assign_title_streams(const Context& context, std::mt19937& rng);
    void reset_stream(ExtendedRainStream& stream, const Context& context, std::mt19937& rng);
    void update_stream(ExtendedRainStream& stream, const Context& context, float delta, std::mt19937& rng);
    char32_t random_character(std::mt19937& rng) const;
    static std::string encode_utf8(char32_t codepoint);

    RainAndConvergeConfig config_{};
    std::vector<ExtendedRainStream> streams_{};
    float x_velocity_per_unit_y_{0.0f};
    bool initialized_{false};
    unsigned int cached_cols_{0};
    unsigned int cached_rows_{0};
    bool all_in_place_{false};
    bool has_rendered_post_drain_{false};
    std::size_t targeted_streams_{0};
    bool draining_rain_{false};
    bool rain_drained_{false};
};

