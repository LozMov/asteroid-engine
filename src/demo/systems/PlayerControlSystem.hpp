#pragma once

#include "engine/ecs/System.hpp"
#include "../components/Player.hpp"

namespace astd::systems {

class PlayerControlSystem : public ast::System<components::Player> {
public:
    PlayerControlSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override;
};

}  // namespace astd::systems
