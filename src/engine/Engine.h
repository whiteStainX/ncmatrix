#pragma once

#include <memory>
#include <random>
#include <vector>

#include <notcurses/notcurses.h>

#include "Context.h"
#include "Effect.h"

class Engine {
public:
    Engine();
    ~Engine();

    void add_effect(std::unique_ptr<Effect> effect);
    void run();

private:
    void update_context_dimensions();
    void process_input();

    struct notcurses* nc_{nullptr};
    struct ncplane* stdplane_{nullptr};
    Context context_{};
    std::vector<std::unique_ptr<Effect>> effects_{};
    std::mt19937 rng_{};
    bool running_{false};
};
