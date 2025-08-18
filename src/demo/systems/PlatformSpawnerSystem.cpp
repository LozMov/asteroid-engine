#include "PlatformSpawnerSystem.hpp"

#include "engine/Cache.hpp"
#include "../components/Enemy.hpp"
#include "../components/Player.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Sprite.hpp"
#include "../components/Animation.hpp"
#include "../Prefab.hpp"

namespace astd::systems {

using ast::Entity;
using namespace components;

PlatformSpawnerSystem::PlatformSpawnerSystem(ast::Registry& registry)
    : System(registry), randomGenerator_(std::random_device{}()), randomDistribution_(0.0f, 1.0f) {}

void PlatformSpawnerSystem::update(float dt) {
    // Only spawn platforms during gameplay
    if (registry_.context.gameState != ast::Registry::Context::GameState::PLAYING) {
        return;
    }

    for (auto entity : entities_) {
        auto* player = registry_.get<Player>(entity);
        auto* transform = registry_.get<Transform>(entity);

        if (transform) {
            float playerX = transform->position.x;
            float spawnX = playerX + spawnDistance;

            // Check if we need to spawn a new platform
            if (spawnX > lastSpawnX + minPlatformSpacing) {
                float platformWidth = getRandomFloat(minPlatformWidth, maxPlatformWidth);
                float platformSpacing = getRandomFloat(minPlatformSpacing, maxPlatformSpacing);
                float platformY = getRandomFloat(minSpawnY, maxSpawnY);

                spawnPlatform(spawnX, platformY, platformWidth);

                // Update last spawn position
                lastSpawnX = spawnX + platformSpacing;

                AST_INFO("Spawned platform at ({}, {}) with width {}", spawnX, platformY,
                         platformWidth);
            }
        }
    }
}

void PlatformSpawnerSystem::spawnPlatform(float x, float y, float width) {
    std::string platformPrefab = "Platform" + std::to_string(getRandomInt(1, 4));
    auto prefab = registry_.get(platformPrefab);
    if (!prefab) {
        AST_ERROR("Platform prefab {} not found", platformPrefab);
        return;
    }
    float height = getRandomFloat(minPlatformHeight, maxPlatformHeight);
    auto entity = registry_.createEntity();
    registry_.copyAll<Transform, Sprite, RigidBody>(entity, prefab, false);
    auto* transform = registry_.get<Transform>(entity);
    transform->position = {x, y};
    auto* rigidBody = registry_.get<RigidBody>(entity);
    rigidBody->size = {width, height};
    auto* sprite = registry_.get<Sprite>(entity);
    sprite->size = {width, height};
    registry_.forceCheck(entity);

    // Randomly spawn an enemy on the platform
    if (getRandomFloat(0.0f, 1.0f) < enemySpawnChance) {
        float enemyX = x + width * 0.5f;  // Center of platform
        float enemyY = y - 5.0f;          // Above the platform
        spawnEnemy(enemyX, enemyY, width);
    }
}

void PlatformSpawnerSystem::spawnEnemy(float x, float y, float platformWidth) {
    // Create enemy entity
    std::string enemyPrefab = "Enemy";
    auto prefab = registry_.get(enemyPrefab);
    if (!prefab) {
        AST_ERROR("Enemy prefab {} not found", enemyPrefab);
        return;
    }
    auto entity = registry_.createEntity();
    registry_.copyAll<Animation, Transform, Sprite, RigidBody, Enemy>(entity, prefab, false);
    auto* transform = registry_.get<Transform>(entity);
    transform->position = {x, y};
    auto* enemy = registry_.get<Enemy>(entity);
    // Patrol on the platform
    enemy->spawnPosition = {x, y};
    enemy->patrolDistance = platformWidth * 0.4f;
    registry_.forceCheck(entity);

    AST_INFO("Spawned enemy at ({}, {})", x, y);
}

float PlatformSpawnerSystem::getRandomFloat(float min, float max) {
    return min + (max - min) * randomDistribution_(randomGenerator_);
}

int PlatformSpawnerSystem::getRandomInt(int min, int max) {
    return min + (max - min) * randomDistribution_(randomGenerator_);
}

}  // namespace astd::systems