#pragma once

#include <random>

#include "engine/ecs/System.hpp"
#include "../components/Player.hpp"
#include "../components/Transform.hpp"

namespace astd::systems {

class PlatformSpawnerSystem : public ast::System<components::Player, components::Transform> {
public:
    PlatformSpawnerSystem(ast::Registry& registry);

    void update(float dt) override;

    float spawnDistance = 800.0f;       // Distance ahead of player to spawn platforms
    float minPlatformWidth = 100.0f;    // Minimum platform width
    float maxPlatformWidth = 300.0f;    // Maximum platform width
    float minSpawnY = 1800.0f;          // Minimum Y position to spawn platforms
    float maxSpawnY = 2000.0f;          // Maximum Y position to spawn platforms
    float minPlatformHeight = 32.0f;    // Minimum platform height
    float maxPlatformHeight = 64.0f;    // Maximum platform height
    float minPlatformSpacing = 150.0f;  // Minimum distance between platforms
    float maxPlatformSpacing = 300.0f;  // Maximum distance between platforms
    float lastSpawnX = 0.0f;            // X position of last spawned platform
    bool shouldSpawnEnemy = true;       // Whether to spawn enemies on platforms
    float enemySpawnChance = 0.7f;      // Probability of spawning an enemy (0.0 to 1.0)

private:
    void spawnPlatform(float x, float y, float width);
    void spawnEnemy(float x, float y, float platformWidth);
    float getRandomFloat(float min, float max);
    int getRandomInt(int min, int max);

    std::mt19937 randomGenerator_;
    std::uniform_real_distribution<float> randomDistribution_;
};

}  // namespace astd::systems