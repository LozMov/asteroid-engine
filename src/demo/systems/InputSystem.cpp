#include "InputSystem.hpp"

#include "engine/Input.hpp"

namespace astd::systems {

void InputSystem::update(float dt) {
    bool isJumpPressed = ast::Input::isKeyPressed(ast::Input::Scancode::SPACE);
    for (auto entity : entities_) {
        auto* player = registry_.get<components::Player>(entity);
        player->jumpPressedEdge = false;
        player->left = ast::Input::isKeyPressed(ast::Input::Scancode::A);
        player->right = ast::Input::isKeyPressed(ast::Input::Scancode::D);
        player->jump = isJumpPressed;
        if (isJumpPressed && !prevJumpPressed_) {
            player->jumpPressedEdge = true;
        }
    }
    prevJumpPressed_ = isJumpPressed;
}

void InputSystem::onEntityAdded(ast::Entity entity) {
}

// void InputSystem::onEvent(const ast::Input::KeyEvent& event) {
// }

}  // namespace astd::systems