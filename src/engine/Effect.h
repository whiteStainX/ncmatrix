#pragma once

#include "Context.h"

class Effect {
public:
    virtual ~Effect() = default;

    virtual void update(const Context& context) = 0;
    virtual void render(const Context& context) = 0;
};
