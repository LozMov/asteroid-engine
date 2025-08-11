#pragma once

#include "box2d/box2d.h"

#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"

namespace astd::components {

struct RigidBody : ast::Component {
    enum class BodyType { STATIC = 0, KINEMATIC = 1, DYNAMIC = 2 } bodyType = BodyType::DYNAMIC;

    enum class CollisionCategory : uint16_t {
        NONE = 0x0000,
        PLAYER = 0x0001,
        ENEMY = 0x0002,
        PLATFORM = 0x0004,
        COLLECTIBLE = 0x0008,
        OBSTACLE = 0x0010,
        PLAYER_ATTACK = 0x0020,
        ENEMY_ATTACK = 0x0040,
        ONE_WAY_PLATFORM = 0x0080,
        ALL = 0xFFFF
    } collisionCategory = CollisionCategory::ALL;

    b2BodyId handle{};
    ast::Vector2 size;
    ast::Vector2 linearVelocity;
    float angularVelocity;
    float mass = 1.0f;
    float restitution = 0.0f;
    float friction = 0.2f;
    float gravityScale = 1.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    bool fixedRotation = false;
    bool isEnabled = true;
    bool isBullet = false;
};

inline RigidBody::CollisionCategory operator|(RigidBody::CollisionCategory a, RigidBody::CollisionCategory b) {
    return static_cast<RigidBody::CollisionCategory>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

}  // namespace astd::components