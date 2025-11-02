#include "engine/Engine.h"

#include <memory>
#include <string>

class DemoEffect : public Effect {
public:
    void update(const Context& /*context*/) override {}

    void render(const Context& context) override {
        if (context.root_plane == nullptr) {
            return;
        }

        const std::string text = "cyberrain";
        const unsigned int y = context.rows / 2;
        const unsigned int text_length = static_cast<unsigned int>(text.size());
        unsigned int x = 0;
        if (context.cols > text_length) {
            x = (context.cols - text_length) / 2;
        }

        ncplane_erase(context.root_plane);
        ncplane_putstr_yx(context.root_plane, y, x, text.c_str());
    }
};

int main() {
    Engine engine;
    engine.add_effect(std::make_unique<DemoEffect>());
    engine.run();
    return 0;
}
