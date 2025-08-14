#include "PlayerControlSystem.hpp"

#include "engine/Vector2.hpp"
#include "engine/events/EventBus.hpp"
#include "PhysicsSystem.hpp"
#include "../components/Animation.hpp"
#include "../components/Player.hpp"

namespace astd::systems {

void PlayerControlSystem::update(float dt) {
    for (auto entity : entities_) {
        auto* player = registry_.get<components::Player>(entity);

        // Update timers
        if (player->coyoteTimer > 0.0f) {
            // AST_DEBUG("Coyote timer: {}", player->coyoteTimer);
            player->coyoteTimer -= dt;
        }
        if (player->jumpBufferTimer > 0.0f) {
            // AST_DEBUG("Jump buffer timer: {}", player->jumpBufferTimer);
            player->jumpBufferTimer -= dt;
        }

        if (player->jumpPressedEdge) {
            player->jumpBufferTimer = player->jumpBufferDuration;
            player->wantJump = true;
            player->jumpConsumed = false;
        }

        // Horizontal movement
        float velocityX = 0.0f;
        if (player->left) {
            velocityX = player->onGround ? -player->walkSpeed : -player->airSpeed;
            // Or flip the sprite
            if (auto* animation = registry_.get<components::Animation>(entity)) {
                animation->frameStartIndex = 4;
                animation->playing = true;
            }
        } else if (player->right) {
            velocityX = player->onGround ? player->walkSpeed : player->airSpeed;
            if (auto* animation = registry_.get<components::Animation>(entity)) {
                animation->frameStartIndex = 8;
                animation->playing = true;
            }
        } else {
            if (auto* animation = registry_.get<components::Animation>(entity)) {
                // animation->frameStartIndex = 2;
                animation->playing = false;
            }
        }
        if (velocityX != 0.0f) {
            ast::EventBus::publish(PhysicsCommand{entity, PhysicsCommand::Type::SET_LINEAR_VELOCITY_X, {}, velocityX, {}});
        }

        // Jump
        bool canJump = player->onGround || player->coyoteTimer > 0.0f;
        if (player->wantJump && !player->jumpConsumed && player->jumpBufferTimer > 0.0f &&
            canJump) {
            // Consume jump intent
            player->jumpConsumed = true;
            player->wantJump = false;
            player->onGround = false;  // No double jump
            ast::EventBus::publish(PhysicsCommand{entity, PhysicsCommand::Type::APPLY_LINEAR_IMPULSE, {0.0f, -player->jumpImpulse}, {}, {}});
        }

        if (player->onGround && player->jumpBufferTimer > 0.0f && !player->jumpConsumed) {
            player->jumpConsumed = true;
            player->wantJump = false;
            ast::EventBus::publish(PhysicsCommand{entity, PhysicsCommand::Type::APPLY_LINEAR_IMPULSE, {0.0f, -player->jumpImpulse}, {}, {}});
            player->onGround = false;
        }
    }
}

}  // namespace astd::systems