#include "RainEffect.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <numbers>
#include <random>
#include <string>

#include <notcurses/notcurses.h>

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

std::string encode_utf8(char32_t codepoint) {
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
} // namespace

RainEffect::RainEffect(RainConfig config)
    : config_(std::move(config)),
      start_time_(std::chrono::steady_clock::now()) {
    const float radians = config_.slantAngle * std::numbers::pi_v<float> / 180.0f;
    x_velocity_per_unit_y_ = std::tan(radians);
    ensure_character_set_loaded();
}

void RainEffect::ensure_character_set_loaded() {
    if (!config_.characterSet.empty()) {
        return;
    }

    std::ifstream input(config_.characterSetFile);
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            for (unsigned char ch : line) {
                config_.characterSet.push_back(static_cast<char32_t>(ch));
            }
        }
    }

    if (config_.characterSet.empty()) {
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
        config_.characterSet.assign(std::begin(fallback_chars), std::end(fallback_chars));
    }
}

char32_t RainEffect::random_character(std::mt19937& rng) const {
    if (config_.characterSet.empty()) {
        return U' ';
    }

    std::uniform_int_distribution<std::size_t> dist(0, config_.characterSet.size() - 1);
    return config_.characterSet[dist(rng)];
}

void RainEffect::ensure_initialized(const Context& context) {
    if (context.cols == 0 || context.rows == 0) {
        return;
    }

    const unsigned int desired_streams = std::max(1U, static_cast<unsigned int>(context.cols * config_.density));
    if (streams_.size() != desired_streams) {
        streams_.resize(desired_streams);
        initialized_ = false;
    }

    if (!initialized_) {
        for (auto& stream : streams_) {
            stream.markedForReset = true;
        }
        initialized_ = true;
    }

    for (auto& stream : streams_) {
        if (stream.markedForReset) {
            resetStream(stream, context);
        }
    }
}

void RainEffect::resetStream(RainStream& stream, const Context& context) {
    std::mt19937 fallback_rng{std::random_device{}()};
    std::mt19937& rng = resolve_rng(context, fallback_rng);

    const float min_speed = std::min(config_.minSpeed, config_.maxSpeed);
    const float max_speed = std::max(config_.minSpeed, config_.maxSpeed);
    std::uniform_real_distribution<float> speed_dist(min_speed, max_speed);

    const int min_length = std::max(1, std::min(config_.minLength, config_.maxLength));
    const int max_length = std::max(min_length, config_.maxLength);
    std::uniform_int_distribution<int> length_dist(min_length, max_length);

    stream.maxLength = length_dist(rng);
    std::uniform_int_distribution<int> current_length_dist(min_length, stream.maxLength);
    stream.length = current_length_dist(rng);

    stream.speed = speed_dist(rng);

    if (context.cols > 0) {
        std::uniform_real_distribution<float> x_dist(0.0f, static_cast<float>(std::max(1U, context.cols) - 1U));
        stream.x = x_dist(rng);
    } else {
        stream.x = 0.0f;
    }

    if (context.rows > 0) {
        std::uniform_real_distribution<float> y_dist(-static_cast<float>(context.rows), 0.0f);
        stream.y = y_dist(rng);
    } else {
        stream.y = 0.0f;
    }

    stream.markedForReset = false;
    stream.hasLeadChar = true;
    stream.characters.resize(stream.maxLength);
    for (auto& character : stream.characters) {
        character = random_character(rng);
    }
}

void RainEffect::update(const Context& context) {
    ensure_initialized(context);
    if (streams_.empty() || context.cols == 0) {
        return;
    }

    const float delta = (context.deltaTime > 0.0f) ? context.deltaTime : kDefaultFrameTime;
    std::mt19937 fallback_rng{std::random_device{}()};
    std::mt19937& rng = resolve_rng(context, fallback_rng);
    std::uniform_real_distribution<float> shimmer_dist(0.0f, 1.0f);

    for (auto& stream : streams_) {
        if (stream.markedForReset) {
            resetStream(stream, context);
            continue;
        }

        stream.y += stream.speed * delta;
        stream.x += stream.speed * x_velocity_per_unit_y_ * delta;

        if (context.cols > 0) {
            const float cols_f = static_cast<float>(context.cols);
            while (stream.x < 0.0f) {
                stream.x += cols_f;
            }
            while (stream.x >= cols_f) {
                stream.x -= cols_f;
            }
        }

        if (stream.length < stream.maxLength) {
            stream.length = std::min(stream.maxLength, stream.length + 1);
        }

        if (!stream.characters.empty()) {
            stream.characters[0] = random_character(rng);
        }

        if (!stream.characters.empty() && shimmer_dist(rng) < 0.1f) {
            std::uniform_int_distribution<std::size_t> index_dist(0, stream.characters.size() - 1);
            const std::size_t index = index_dist(rng);
            stream.characters[index] = random_character(rng);
        }

        if ((stream.y - static_cast<float>(stream.length)) > static_cast<float>(context.rows)) {
            stream.markedForReset = true;
        }
    }
}

void RainEffect::render(const Context& context) {
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
    decode_rgba(config_.leadCharColor, lead_r, lead_g, lead_b);
    decode_rgba(config_.tailColor, tail_r, tail_g, tail_b);

    for (const auto& stream : streams_) {
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
                const uint8_t r = static_cast<uint8_t>(static_cast<float>(tail_r) * (1.0f - t));
                const uint8_t g = static_cast<uint8_t>(static_cast<float>(tail_g) * (1.0f - t));
                const uint8_t b = static_cast<uint8_t>(static_cast<float>(tail_b) * (1.0f - t));
                ncplane_set_fg_rgb8(context.root_plane, r, g, b);
                ncplane_off_styles(context.root_plane, NCSTYLE_BOLD);
            }

            const char32_t glyph_code = stream.characters.empty() ? U' ' : stream.characters[static_cast<std::size_t>(std::min(i, available_chars - 1))];
            const std::string glyph_utf8 = encode_utf8(glyph_code);
            ncplane_putegc_yx(context.root_plane, screen_y, screen_x, glyph_utf8.c_str(), nullptr);
        }
    }

    ncplane_off_styles(context.root_plane, NCSTYLE_BOLD);
}

bool RainEffect::isFinished() const {
    if (config_.duration <= 0.0f) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const float elapsed = std::chrono::duration<float>(now - start_time_).count();
    return elapsed >= config_.duration;
}

