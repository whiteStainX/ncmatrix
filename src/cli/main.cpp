#include "cli/ConfigLoader.h"
#include "engine/Engine.h"
#include "effects/RainAndConvergeEffect.h"
#include "effects/RainEffect.h"

#include <cxxopts.hpp>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv) {
    cxxopts::Options options("ncmatrix", "Digital rain effect renderer");
    options.add_options()
        ("c,config", "Path to configuration file", cxxopts::value<std::string>()->default_value("matrix.toml"))
        ("h,help", "Print usage information");

    cxxopts::ParseResult result;
    try {
        result = options.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& ex) {
        std::cerr << "Failed to parse command line: " << ex.what() << '\n';
        std::cout << options.help() << '\n';
        return 1;
    }
    if (result.count("help")) {
        std::cout << options.help() << '\n';
        return 0;
    }

    const std::filesystem::path config_path = result["config"].as<std::string>();
    SceneConfig scene_config = load_scene_config_from_file(config_path);

    Engine engine;
    if (scene_config.animation == AnimationType::RainAndConverge) {
        engine.add_effect(std::make_unique<RainAndConvergeEffect>(std::move(scene_config.rainAndConverge)));
    } else {
        engine.add_effect(std::make_unique<RainEffect>(std::move(scene_config.rain)));
    }
    engine.run();
    return 0;
}
