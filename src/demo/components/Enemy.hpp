#pragma once

#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Enemy : ast::Component {
    float patrolSpeed = 5.0f;                     // Pixels per second
    float patrolDistance = 100.0f;                 // How far to patrol from spawn point
    ast::Vector2 spawnPosition;                    // Original spawn position
    ast::Vector2 currentDirection = {1.0f, 0.0f};  // Current movement direction
    float patrolTimer = 0.0f;                      // Timer for patrol behavior
    bool isPatrolling = true;

    // Combat
    int maxHealth = 100;
    int currentHealth = 100;
    float attackDamage = 10.0f;
    float attackRange = 150.0f;
    float attackCooldown = 1.0f;
    float attackTimer = 0.0f;
    int scoreValue = 100;
    bool isDead = false;
};

}  // namespace astd::components