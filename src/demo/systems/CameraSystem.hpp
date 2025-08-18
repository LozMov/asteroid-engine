#pragma once

#include "engine/ecs/System.hpp"
#include "../components/Camera.hpp"

namespace astd::systems {

class CameraSystem : public ast::System<components::Camera> {
public:
    CameraSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : entities_) {
            auto* camera = registry_.get<components::Camera>(entity);
            // If camera has a target, follow it
            if (auto* targetTransform = registry_.get<components::Transform>(camera->target)) {
                // Smooth camera following
                ast::Vector2 cameraPos = camera->position;
                // Center the camera on the target
                ast::Vector2 targetPos = targetTransform->position - camera->screenSize / 2.0f;
                ast::Vector2 newPos = cameraPos + (targetPos - cameraPos) * camera->lerp;
                if (newPos.x - cameraPos.x > 2.0f) {
                    camera->position = newPos;
                } else {
                    camera->position = targetPos;  // Snap to target if close enough
                }
                // Clamp camera position to the world bounds
                if (worldWidth > 0.0f) {
                    camera->position.x = std::clamp(camera->position.x, 0.0f, worldWidth - camera->screenSize.x);
                }
                if (worldHeight > 0.0f) {
                    camera->position.y = std::clamp(camera->position.y, 0.0f, worldHeight - camera->screenSize.y);
                }
            }
        }
    }

    float worldWidth = 1920.0f;
    float worldHeight = 1080.0f;
};

}  // namespace astd::systems