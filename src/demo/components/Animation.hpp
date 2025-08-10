#pragma once

#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Animation : ast::Component {
    Animation() = default;
    Animation(float frameDuration, bool loop) : frameDuration(frameDuration), loop(loop) {}

    float frameDuration = 0.0f;
    float frameTimeElapsed = 0.0f;
    bool loop = true;
    bool playing = true;
};

}  // namespace astd::components