#pragma once

#include "engine/Input.hpp"
#include "engine/ecs/System.hpp"
#include "engine/events/EventSubscriber.hpp"

#include "../components/Player.hpp"

namespace astd::systems {

class InputSystem : public ast::System<components::Player> {
public:
    InputSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override;
    void onEntityAdded(ast::Entity entity) override;
    // void onEvent(const ast::Input::KeyEvent& event) override;

private:
    bool prevJumpPressed_ = false;
};

}  // namespace astd::systems