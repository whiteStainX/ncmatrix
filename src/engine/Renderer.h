#pragma once

#include <notcurses/notcurses.h>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void draw_text(const char* text);
    void wait_for_quit();

private:
    struct notcurses* nc;
    struct ncplane* stdplane;
};
