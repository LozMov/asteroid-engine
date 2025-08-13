#pragma once

#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Player : ast::Component {
    // enum class Action { IDLE, WALK, RUN, JUMP, FALL } action = Action::IDLE;
    // enum class Direction { DOWN, UP, LEFT, RIGHT } direction = Direction::RIGHT;
    float walkSpeed = 1.0f;
    float runSpeed = 2.0f;
    float airSpeed = 0.5f;
    float jumpImpulse = 0.0f;
    float jumpBufferDuration = 0.1f;
    float coyoteDuration = 0.12f;
    float jumpBufferTimer = 0.1f;
    float coyoteTimer = 0.1f;
    int groundContactCount = 0;
    bool onGround = false;
    bool canJump = false;
    bool wantJump = false;
    bool jumpConsumed = false;
    // TODO Input states
    bool left = false;
    bool right = false;
    bool jump = false;  // Current frame key pressed
    bool jumpPressedEdge = false;  // True only on frame key-down
};

}  // namespace astd::components