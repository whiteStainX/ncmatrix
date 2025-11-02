#pragma once

#include "effects/RainEffect.h"

#include <filesystem>

RainConfig load_rain_config_from_file(const std::filesystem::path& path);
