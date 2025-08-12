#pragma once

#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Transform : ast::Component {
    Transform(const ast::Vector2& position = ast::Vector2(), float rotation = 0.0f,
              const ast::Vector2& scale = ast::Vector2(1.0f, 1.0f))
        : position(position), rotation(rotation), scale(scale) {}

    /// The position of the sprite's origin point (where the sprite should be drawn)
    ast::Vector2 position;
    ast::Vector2 scale = ast::Vector2(1.0f, 1.0f);
    float rotation = 0.0f;
};

}  // namespace astd::components