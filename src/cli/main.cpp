#include "engine/Engine.h"
#include "effects/DemoEffect.h"

#include <memory>

int main() {
    Engine engine;
    engine.add_effect(std::make_unique<DemoEffect>());
    engine.run();
    return 0;
}
