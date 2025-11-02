#include "engine/Renderer.h"

int main() {
    Renderer renderer;
    renderer.draw_text("cyberrain");
    renderer.wait_for_quit();
    return 0;
}