#pragma once

#include "engine/ecs/System.hpp"
#include "../components/Lifetime.hpp"

namespace astd::systems {

class EntityCleanupSystem : public ast::System<components::Lifetime> {
public:
    EntityCleanupSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : entities_) {
            auto* lifetime = registry_.get<components::Lifetime>(entity);
            if ((lifetime->remainingTime -= dt) <= 0.0f) {
                registry_.markAsExpired(entity);
            }
        }
    }
};

}  // namespace astd::systems