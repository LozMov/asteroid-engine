#pragma once

#include "engine/ecs/System.hpp"
#include "../components/Enemy.hpp"
#include "../components/Transform.hpp"
#include "../components/RigidBody.hpp"

namespace astd::systems {

class EnemySystem
    : public ast::System<components::Enemy, components::Transform, components::RigidBody> {
public:
    EnemySystem(ast::Registry& registry);

    void update(float dt) override;

private:
    void updatePatrolBehavior(ast::Entity entity, components::Enemy* enemy,
                              components::Transform* transform);
};

}  // namespace astd::systems