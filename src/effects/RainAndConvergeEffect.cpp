#include "effects/RainAndConvergeEffect.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <numbers>
#include <random>
#include <string>

#include <notcurses/notcurses.h>

#include "utils/Utf8.h"

namespace {
constexpr float kDefaultFrameTime = 1.0f / 60.0f;
std::mt19937& resolve_rng(const Context& context, std::mt19937& fallback) {
    if (context.rng != nullptr) {
        return *context.rng;
    }
    return fallback;
}

void decode_rgba(uint32_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = static_cast<uint8_t>((color >> 24U) & 0xFFU);
    g = static_cast<uint8_t>((color >> 16U) & 0xFFU);
    b = static_cast<uint8_t>((color >> 8U) & 0xFFU);
}

} // namespace

RainAndConvergeEffect::RainAndConvergeEffect(RainAndConvergeConfig config)
    : config_(std::move(config)) {
    const float radians = config_.rainConfig.slantAngle * std::numbers::pi_v<float> / 180.0f;
    x_velocity_per_unit_y_ = std::tan(radians);
    ensure_character_set_loaded();
}

void RainAndConvergeEffect::ensure_character_set_loaded() {
    if (!config_.rainConfig.characterSet.empty()) {
        return;
    }

    std::ifstream input(config_.rainConfig.characterSetFile, std::ios::binary);
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            const auto decoded = utf8::decode(line);
            config_.rainConfig.characterSet.insert(config_.rainConfig.characterSet.end(), decoded.begin(), decoded.end());
        }
    }

    if (config_.rainConfig.characterSet.empty()) {
        static constexpr char32_t fallback_chars[] = {
            U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9',
            U'A', U'B', U'C', U'D', U'E', U'F', U'G', U'H', U'I', U'J',
            U'K', U'L', U'M', U'N', U'O', U'P', U'Q', U'R', U'S', U'T',
            U'U', U'V', U'W', U'X', U'Y', U'Z',
            U'a', U'b', U'c', U'd', U'e', U'f', U'g', U'h', U'i', U'j',
            U'k', U'l', U'm', U'n', U'o', U'p', U'q', U'r', U's', U't',
            U'u', U'v', U'w', U'x', U'y', U'z',
            U'@', U'#', U'$', U'%', U'&', U'*'
        };
        config_.rainConfig.characterSet.assign(std::begin(fallback_chars), std::end(fallback_chars));
    }
}

char32_t RainAndConvergeEffect::random_character(std::mt19937& rng) const {
    if (config_.rainConfig.characterSet.empty()) {
        return U' ';
    }

    std::uniform_int_distribution<std::size_t> dist(0, config_.rainConfig.characterSet.size() - 1);
    return config_.rainConfig.characterSet[dist(rng)];
}

std::string RainAndConvergeEffect::encode_utf8(char32_t codepoint) {
    std::string out;
    if (codepoint <= 0x7FU) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FFU) {
        out.push_back(static_cast<char>(0xC0U | ((codepoint >> 6U) & 0x1FU)));
        out.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint <= 0xFFFFU) {
        out.push_back(static_cast<char>(0xE0U | ((codepoint >> 12U) & 0x0FU)));
        out.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint <= 0x10FFFFU) {
        out.push_back(static_cast<char>(0xF0U | ((codepoint >> 18U) & 0x07U)));
        out.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
        out.push_back('?');
    }
    return out;
}

void RainAndConvergeEffect::ensure_initialized(const Context& context) {
    if (context.cols == 0 || context.rows == 0) {
        return;
    }

    if (!initialized_ || context.cols != cached_cols_ || context.rows != cached_rows_) {
        cached_cols_ = context.cols;
        cached_rows_ = context.rows;
        initialize_streams(context);
        initialized_ = true;
    }
}

void RainAndConvergeEffect::initialize_streams(const Context& context) {
    std::mt19937 fallback_rng{std::random_device{}()};
    std::mt19937& rng = resolve_rng(context, fallback_rng);

    streams_.assign(context.cols, {});
    targeted_streams_ = 0;
    has_rendered_post_drain_ = false;
    all_in_place_ = false;
    draining_rain_ = false;
    rain_drained_ = false;

    for (unsigned int col = 0; col < context.cols; ++col) {
        auto& stream = streams_[col];
        stream.x = static_cast<float>(col);
        stream.hasLeadChar = true;
        reset_stream(stream, context, rng);
    }

    assign_title_streams(context, rng);
}

void RainAndConvergeEffect::assign_title_streams(const Context& context, std::mt19937& rng) {
    if (config_.title.empty() || streams_.empty()) {
        targeted_streams_ = 0;
        return;
    }

    const unsigned int title_width = static_cast<unsigned int>(config_.title.size());
    unsigned int start_col = 0;
    if (context.cols > title_width) {
        start_col = (context.cols - title_width) / 2;
    }

    const unsigned int target_row = (config_.titleRow > 0 && config_.titleRow < context.rows)
        ? config_.titleRow
        : context.rows / 2;

    for (unsigned int i = 0; i < title_width; ++i) {
        const char32_t glyph = config_.title[i];
        const unsigned int column = std::min(context.cols - 1, start_col + i);
        if (glyph == U' ') {
            continue;
        }

        if (column >= streams_.size()) {
            continue;
        }

        auto& stream = streams_[column];
        stream.isTitleStream = true;
        stream.state = ExtendedRainStream::State::CONVERGING;
        stream.titleChar = glyph;
        stream.targetY = static_cast<float>(target_row);
        stream.convergenceElapsed = 0.0f;
        stream.allowRespawn = false;
        stream.inactive = false;
        stream.characters.resize(std::max(stream.maxLength, 1));
        if (!stream.characters.empty()) {
            stream.characters[0] = glyph;
        }
        const float distance = stream.targetY - stream.y;
        if (config_.convergenceDuration > 0.0f) {
            const float required_speed = distance / config_.convergenceDuration;
            if (required_speed > 0.0f) {
                const float randomness = std::clamp(config_.convergenceRandomness, 0.0f, 1.0f);
                const float min_multiplier = std::max(0.1f, 1.0f - randomness);
                const float max_multiplier = 1.0f + randomness;
                std::uniform_real_distribution<float> multiplier_dist(min_multiplier, max_multiplier);
                const float multiplier = multiplier_dist(rng);
                stream.speed = required_speed * multiplier;
            }
        }
        targeted_streams_++;
    }
}

void RainAndConvergeEffect::reset_stream(ExtendedRainStream& stream, const Context& context, std::mt19937& rng) {
    const float min_speed = std::min(config_.rainConfig.minSpeed, config_.rainConfig.maxSpeed);
    const float max_speed = std::max(config_.rainConfig.minSpeed, config_.rainConfig.maxSpeed);
    std::uniform_real_distribution<float> speed_dist(min_speed, max_speed);

    const int min_length = std::max(1, std::min(config_.rainConfig.minLength, config_.rainConfig.maxLength));
    const int max_length = std::max(min_length, config_.rainConfig.maxLength);
    std::uniform_int_distribution<int> length_dist(min_length, max_length);

    stream.maxLength = length_dist(rng);
    std::uniform_int_distribution<int> current_length_dist(min_length, stream.maxLength);
    stream.length = current_length_dist(rng);
    stream.speed = speed_dist(rng);

    if (context.rows > 0) {
        std::uniform_real_distribution<float> y_dist(-static_cast<float>(context.rows), 0.0f);
        stream.y = y_dist(rng);
    } else {
        stream.y = 0.0f;
    }

    stream.state = ExtendedRainStream::State::NORMAL;
    stream.isTitleStream = false;
    stream.titleChar = U' ';
    stream.targetY = 0.0f;
    stream.convergenceElapsed = 0.0f;
    stream.hasLeadChar = true;
    stream.allowRespawn = true;
    stream.inactive = false;
    stream.characters.resize(stream.maxLength);
    for (auto& character : stream.characters) {
        character = random_character(rng);
    }
}

void RainAndConvergeEffect::update_stream(ExtendedRainStream& stream, const Context& context, float delta, std::mt19937& rng) {
    if (stream.inactive) {
        return;
    }

    std::uniform_real_distribution<float> shimmer_dist(0.0f, 1.0f);

    switch (stream.state) {
    case ExtendedRainStream::State::NORMAL: {
        stream.y += stream.speed * delta;
        stream.x += stream.speed * x_velocity_per_unit_y_ * delta;

        if (stream.length < stream.maxLength) {
            stream.length = std::min(stream.maxLength, stream.length + 1);
        }

        if (!stream.characters.empty() && shimmer_dist(rng) < 0.1f) {
            std::uniform_int_distribution<std::size_t> index_dist(0, stream.characters.size() - 1);
            const std::size_t index = index_dist(rng);
            stream.characters[index] = random_character(rng);
        }

        if ((stream.y - static_cast<float>(stream.length)) > static_cast<float>(context.rows)) {
            if (!stream.isTitleStream) {
                if (stream.allowRespawn) {
                    reset_stream(stream, context, rng);
                } else {
                    stream.length = 0;
                    stream.inactive = true;
                }
            } else {
                stream.y = -static_cast<float>(stream.length);
            }
        }
        break;
    }
    case ExtendedRainStream::State::CONVERGING: {
        stream.convergenceElapsed += delta;
        stream.y += stream.speed * delta;
        if (!stream.characters.empty()) {
            stream.characters[0] = stream.titleChar;
        }

        if (!stream.characters.empty() && shimmer_dist(rng) < 0.1f) {
            std::uniform_int_distribution<std::size_t> index_dist(0, stream.characters.size() - 1);
            const std::size_t index = index_dist(rng);
            if (index != 0) {
                stream.characters[index] = random_character(rng);
            }
        }

        if (stream.y >= stream.targetY) {
            stream.y = stream.targetY;
            stream.state = ExtendedRainStream::State::IN_PLACE;
        }
        break;
    }
    case ExtendedRainStream::State::IN_PLACE:
        stream.y = stream.targetY;
        break;
    }

    if (context.cols > 0) {
        const float cols_f = static_cast<float>(context.cols);
        while (stream.x < 0.0f) {
            stream.x += cols_f;
        }
        while (stream.x >= cols_f) {
            stream.x -= cols_f;
        }
    }
}

void RainAndConvergeEffect::update(const Context& context) {
    ensure_initialized(context);
    if (streams_.empty() || context.cols == 0) {
        return;
    }

    const float delta = (context.deltaTime > 0.0f) ? context.deltaTime : kDefaultFrameTime;
    std::mt19937 fallback_rng{std::random_device{}()};
    std::mt19937& rng = resolve_rng(context, fallback_rng);

    bool all_targets_in_place = targeted_streams_ > 0;
    bool all_streams_cleared = true;

    for (auto& stream : streams_) {
        if (draining_rain_ && !stream.isTitleStream) {
            stream.allowRespawn = false;
        }

        update_stream(stream, context, delta, rng);
        if (stream.isTitleStream) {
            if (stream.state != ExtendedRainStream::State::IN_PLACE) {
                all_targets_in_place = false;
            }
        }

        if (!stream.isTitleStream) {
            if (!stream.inactive && stream.length > 0) {
                all_streams_cleared = false;
            }
        }
    }

    if (all_targets_in_place && targeted_streams_ > 0 && !all_in_place_) {
        all_in_place_ = true;
        draining_rain_ = true;
    }

    if (draining_rain_ && all_streams_cleared) {
        rain_drained_ = true;
    }
}

void RainAndConvergeEffect::render(const Context& context) {
    if (context.root_plane == nullptr) {
        return;
    }

    ensure_initialized(context);
    if (streams_.empty()) {
        ncplane_erase(context.root_plane);
        return;
    }

    ncplane_erase(context.root_plane);

    uint8_t lead_r = 0, lead_g = 0, lead_b = 0;
    uint8_t tail_r = 0, tail_g = 0, tail_b = 0;
    decode_rgba(config_.rainConfig.leadCharColor, lead_r, lead_g, lead_b);
    decode_rgba(config_.rainConfig.tailColor, tail_r, tail_g, tail_b);

    for (const auto& stream : streams_) {
        if (stream.state == ExtendedRainStream::State::IN_PLACE) {
            if (stream.titleChar == U' ') {
                continue;
            }

            ncplane_set_fg_rgb8(context.root_plane, lead_r, lead_g, lead_b);
            ncplane_on_styles(context.root_plane, NCSTYLE_BOLD);
            const std::string glyph_utf8 = encode_utf8(stream.titleChar);
            ncplane_putegc_yx(context.root_plane, static_cast<int>(stream.targetY), static_cast<int>(stream.x), glyph_utf8.c_str(), nullptr);
            continue;
        }

        if (stream.inactive && !stream.isTitleStream) {
            continue;
        }

        const int available_chars = std::min(stream.length, static_cast<int>(stream.characters.size()));
        for (int i = 0; i < available_chars; ++i) {
            const int screen_y = static_cast<int>(stream.y) - i;
            const float horizontal_offset = static_cast<float>(i) * x_velocity_per_unit_y_;
            const float raw_screen_x = stream.x - horizontal_offset;
            int screen_x = static_cast<int>(std::round(raw_screen_x));

            if (screen_y < 0 || screen_y >= static_cast<int>(context.rows)) {
                continue;
            }

            if (context.cols > 0) {
                const int cols_int = static_cast<int>(context.cols);
                while (screen_x < 0) {
                    screen_x += cols_int;
                }
                while (screen_x >= cols_int) {
                    screen_x -= cols_int;
                }
            }

            if (screen_x < 0 || screen_x >= static_cast<int>(context.cols)) {
                continue;
            }

            if (i == 0 && stream.hasLeadChar) {
                ncplane_set_fg_rgb8(context.root_plane, lead_r, lead_g, lead_b);
                ncplane_on_styles(context.root_plane, NCSTYLE_BOLD);
            } else {
                const float t = static_cast<float>(i) / std::max(1, stream.length - 1);
                const float base = (1.0f - t);
                const uint8_t r = static_cast<uint8_t>(static_cast<float>(tail_r) * base);
                const uint8_t g = static_cast<uint8_t>(static_cast<float>(tail_g) * base);
                const uint8_t b = static_cast<uint8_t>(static_cast<float>(tail_b) * base);
                ncplane_set_fg_rgb8(context.root_plane, r, g, b);
                ncplane_off_styles(context.root_plane, NCSTYLE_BOLD);
            }

            const char32_t glyph_code = stream.characters.empty() ? U' ' : stream.characters[static_cast<std::size_t>(std::min(i, available_chars - 1))];
            const std::string glyph_utf8 = encode_utf8(glyph_code);
            ncplane_putegc_yx(context.root_plane, screen_y, screen_x, glyph_utf8.c_str(), nullptr);
        }
    }

    if (rain_drained_) {
        has_rendered_post_drain_ = true;
    }

    ncplane_off_styles(context.root_plane, NCSTYLE_BOLD);
}

bool RainAndConvergeEffect::isFinished() const {
    if (config_.rainConfig.duration > 0.0f) {
        return false;
    }
    if (targeted_streams_ == 0) {
        return false;
    }
    return rain_drained_ && has_rendered_post_drain_;
}

