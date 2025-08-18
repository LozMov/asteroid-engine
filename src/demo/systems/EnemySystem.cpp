#include "EnemySystem.hpp"

#include "engine/Log.hpp"
#include "engine/events/EventBus.hpp"
#include "PhysicsSystem.hpp"

namespace astd::systems {

using ast::Entity;
using namespace components;

EnemySystem::EnemySystem(ast::Registry& registry) : System(registry) {}

void EnemySystem::update(float dt) {
    for (auto entity : entities_) {
        auto* enemy = registry_.get<Enemy>(entity);
        auto* transform = registry_.get<Transform>(entity);
        updatePatrolBehavior(entity, enemy, transform);
    }
}

void EnemySystem::updatePatrolBehavior(Entity entity, Enemy* enemy, Transform* transform) {
    if (!enemy->isPatrolling) {
        return;
    }

    // Calculate distance from spawn position
    float distanceFromSpawn = std::abs(transform->position.x - enemy->spawnPosition.x);

    if (distanceFromSpawn >= enemy->patrolDistance) {
        enemy->currentDirection.x *= -1.0f;
    }

    // Apply movement
    float velocityX = enemy->currentDirection.x * enemy->patrolSpeed;
    ast::EventBus::publish(
        PhysicsCommand{entity, PhysicsCommand::Type::SET_LINEAR_VELOCITY_X, {}, velocityX, {}});
}

}  // namespace astd::systems