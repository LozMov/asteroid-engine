#pragma once

#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"
#include "engine/ecs/Entity.hpp"

namespace astd::components {

struct Camera : ast::Component {
    ast::Entity target{};
    ast::Vector2 position;
    ast::Vector2 screenSize;
    float lerp = 0.1f;
    float zoom = 1.0f;
};

}  // namespace astd::components