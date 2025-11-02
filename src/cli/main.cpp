#include "engine/Engine.h"
#include "effects/RainEffect.h"

#include <memory>

int main() {
    Engine engine;
    RainConfig config;
    engine.add_effect(std::make_unique<RainEffect>(config));
    engine.run();
    return 0;
}
