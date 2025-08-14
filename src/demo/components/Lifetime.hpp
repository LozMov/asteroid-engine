#pragma once

#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Lifetime : ast::Component {
    Lifetime() = default;
    Lifetime(float remainingTime) : remainingTime(remainingTime) {}

    float remainingTime = 0.0f;  // Time left before the entity is removed
};

}