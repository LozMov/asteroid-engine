#pragma once

#include "engine/ecs/System.hpp"

#include "../components/Sprite.hpp"
#include "../components/Transform.hpp"

class SDL_Renderer;

namespace astd::systems {

class RenderSystem : public ast::System<components::Sprite, components::Transform> {
public:
    RenderSystem(ast::Registry& registry);

    void update(float dt) override;

private:
    SDL_Renderer* renderer_{};
    float screenWidth_ = 800;
    float screenHeight_ = 600;
};

}  // namespace astd::systems