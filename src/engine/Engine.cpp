#include "Engine.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

Engine::Engine() {
    notcurses_options opts = {0};
    opts.flags = NCOPTION_SUPPRESS_BANNERS;
    nc_ = notcurses_init(&opts, nullptr);
    if (nc_ == nullptr) {
        std::cerr << "Error initializing notcurses" << std::endl;
        std::exit(1);
    }

    stdplane_ = notcurses_stdplane(nc_);
    rng_ = std::mt19937(std::random_device{}());
    context_.attach(nc_, stdplane_, &rng_);
    update_context_dimensions();
}

Engine::~Engine() {
    if (nc_ != nullptr) {
        notcurses_stop(nc_);
    }
}

void Engine::add_effect(std::unique_ptr<Effect> effect) {
    if (effect) {
        effects_.push_back(std::move(effect));
    }
}

void Engine::run() {
    running_ = true;
    while (running_) {
        update_context_dimensions();

        remove_finished_effects();

        for (const auto& effect : effects_) {
            effect->update(context_);
        }

        remove_finished_effects();

        for (const auto& effect : effects_) {
            effect->render(context_);
        }

        remove_finished_effects();

        notcurses_render(nc_);
        process_input();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Engine::update_context_dimensions() {
    unsigned int rows = 0;
    unsigned int cols = 0;
    ncplane_dim_yx(stdplane_, &rows, &cols);
    context_.rows = rows;
    context_.cols = cols;
}

void Engine::process_input() {
    ncinput input;
    timespec ts{0, 0};

    while (true) {
        char32_t key = notcurses_get(nc_, &ts, &input);
        if (key == 0 || key == static_cast<char32_t>(-1)) {
            break;
        }

        if (key == U'q' || key == U'Q') {
            running_ = false;
            break;
        }
    }
}

void Engine::remove_finished_effects() {
    const auto erase_begin = std::remove_if(
        effects_.begin(),
        effects_.end(),
        [](const std::unique_ptr<Effect>& effect) {
            return effect != nullptr && effect->isFinished();
        });
    effects_.erase(erase_begin, effects_.end());
}
