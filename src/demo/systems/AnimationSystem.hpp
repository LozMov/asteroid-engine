#pragma once

#include "engine/ecs/Registry.hpp"
#include "engine/ecs/System.hpp"

#include "../components/Animation.hpp"
#include "../components/Sprite.hpp"

namespace astd::systems {

class AnimationSystem : public ast::System<components::Animation, components::Sprite> {
public:
    AnimationSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : entities_) {
            auto* animation = registry_.get<components::Animation>(entity);
            if (animation->playing) {
                auto* sprite = registry_.get<components::Sprite>(entity);
                animation->frameTimeElapsed += dt;
                if (animation->frameTimeElapsed >= animation->frameDuration) {
                    animation->frameTimeElapsed = 0.0f;
                    int sequenceEndIndex =
                        (animation->frameCount == -1
                             ? sprite->frameCount
                             : animation->frameCount + animation->frameStartIndex);
                    if (++sprite->frameIndex >= sequenceEndIndex) {
                        if (animation->loop) {
                            sprite->frameIndex = animation->frameStartIndex;
                        } else {
                            // Stop at the last frame if not looping
                            sprite->frameIndex = sequenceEndIndex - 1;
                            animation->playing = false;
                        }
                    }
                }
            }
        }
    }
};

}  // namespace astd::systems