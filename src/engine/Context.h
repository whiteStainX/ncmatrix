#pragma once

#include <random>

#include <notcurses/notcurses.h>

struct Context {
    unsigned int rows{0};
    unsigned int cols{0};
    struct notcurses* nc{nullptr};
    struct ncplane* root_plane{nullptr};
    std::mt19937* rng{nullptr};

    void attach(struct notcurses* nc_instance, struct ncplane* plane, std::mt19937* rng_engine) {
        nc = nc_instance;
        root_plane = plane;
        rng = rng_engine;
    }
};
