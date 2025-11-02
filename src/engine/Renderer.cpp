#include "Renderer.h"
#include <iostream>

Renderer::Renderer() {
    notcurses_options opts = {0};
    opts.flags = NCOPTION_SUPPRESS_BANNERS;
    nc = notcurses_init(&opts, NULL);
    if (nc == NULL) {
        // In a real app, you might throw an exception here
        std::cerr << "Error initializing notcurses" << std::endl;
        exit(1);
    }
    stdplane = notcurses_stdplane(nc);
}

Renderer::~Renderer() {
    if (nc) {
        notcurses_stop(nc);
    }
}

void Renderer::draw_text(const char* text) {
    unsigned int y, x;
    ncplane_dim_yx(stdplane, &y, &x);
    ncplane_erase(stdplane);
    ncplane_putstr_yx(stdplane, y / 2, (x - strlen(text)) / 2, text);
    notcurses_render(nc);
}

void Renderer::wait_for_quit() {
    ncinput input;
    while (true) {
        char32_t key = notcurses_get(nc, NULL, &input);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
}
