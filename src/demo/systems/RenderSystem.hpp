#pragma once

#include "engine/ecs/System.hpp"

#include "../components/Sprite.hpp"
#include "../components/Transform.hpp"
#include "../components/Camera.hpp"

class SDL_Renderer;

namespace astd::systems {

class RenderSystem : public ast::System<components::Sprite, components::Transform> {
public:
    RenderSystem(ast::Registry& registry);

    void update(float dt) override;
    void onEntityAdded(ast::Entity entity) override;

private:
    SDL_Renderer* renderer_{};
    ast::Entity playerEntity_{};
    components::Camera* camera_{};
    float screenWidth_ = 800;
    float screenHeight_ = 600;
};

}  // namespace astd::systems