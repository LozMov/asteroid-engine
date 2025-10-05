#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <string>

#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Component.hpp"
#include "engine/ecs/System.hpp"
#include "engine/Vector2.hpp"

// Test components for benchmarking
namespace benchmark_components {

struct Position : ast::Component {
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
};

struct Velocity : ast::Component {
    float x = 0.0f;
    float y = 0.0f;

    Velocity() = default;
    Velocity(float x, float y) : x(x), y(y) {}
};

struct Health : ast::Component {
    int current = 100;
    int max = 100;

    Health() = default;
    Health(int current, int max) : current(current), max(max) {}
};

struct Transform : ast::Component {
    ast::Vector2 position;
    ast::Vector2 scale{1.0f, 1.0f};
    float rotation = 0.0f;

    Transform() = default;
    Transform(const ast::Vector2& pos) : position(pos) {}
};

struct Sprite : ast::Component {
    std::string textureId;
    bool visible = true;

    Sprite() = default;
    Sprite(const std::string& id) : textureId(id) {}
};

struct AI : ast::Component {
    float behaviorTimer = 0.0f;
    int state = 0;

    AI() = default;
    AI(int initialState) : state(initialState) {}
};

}  // namespace benchmark_components

// Test systems for benchmarking
namespace benchmark_systems {

class MovementSystem
    : public ast::System<benchmark_components::Position, benchmark_components::Velocity> {
public:
    MovementSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : getEntities()) {
            auto* pos = getRegistry().get<benchmark_components::Position>(entity);
            auto* vel = getRegistry().get<benchmark_components::Velocity>(entity);
            if (pos && vel) {
                pos->x += vel->x * dt;
                pos->y += vel->y * dt;
            }
        }
    }
};

class HealthSystem : public ast::System<benchmark_components::Health> {
public:
    HealthSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : getEntities()) {
            auto* health = getRegistry().get<benchmark_components::Health>(entity);
            if (health) {
                health->current = std::min(health->current, health->max);
            }
        }
    }
};

class RenderSystem
    : public ast::System<benchmark_components::Transform, benchmark_components::Sprite> {
public:
    RenderSystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : getEntities()) {
            auto* transform = getRegistry().get<benchmark_components::Transform>(entity);
            auto* sprite = getRegistry().get<benchmark_components::Sprite>(entity);
            if (transform && sprite) {
                // Simulate rendering logic
                benchmark::DoNotOptimize(transform->position);
                benchmark::DoNotOptimize(sprite->textureId);
            }
        }
    }
};

class AISystem : public ast::System<benchmark_components::AI, benchmark_components::Position> {
public:
    AISystem(ast::Registry& registry) : System(registry) {}

    void update(float dt) override {
        for (auto entity : getEntities()) {
            auto* ai = getRegistry().get<benchmark_components::AI>(entity);
            auto* pos = getRegistry().get<benchmark_components::Position>(entity);
            if (ai && pos) {
                ai->behaviorTimer += dt;
                // Simulate AI behavior
                if (ai->behaviorTimer > 1.0f) {
                    ai->state = (ai->state + 1) % 4;
                    ai->behaviorTimer = 0.0f;
                }
            }
        }
    }
};

}  // namespace benchmark_systems

// Helper functions for benchmarks
std::vector<ast::Entity> createEntities(ast::Registry& registry, size_t count) {
    std::vector<ast::Entity> entities;
    entities.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        entities.push_back(registry.createEntity());
    }
    return entities;
}

void addRandomComponents(ast::Registry& registry, const std::vector<ast::Entity>& entities) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-1000.0f, 1000.0f);
    std::uniform_real_distribution<float> velDist(-100.0f, 100.0f);
    std::uniform_int_distribution<int> healthDist(50, 200);

    for (auto entity : entities) {
        // Add random components
        if (gen() % 2 == 0) {
            registry.emplace<benchmark_components::Position>(entity, posDist(gen), posDist(gen));
        }
        if (gen() % 2 == 0) {
            registry.emplace<benchmark_components::Velocity>(entity, velDist(gen), velDist(gen));
        }
        if (gen() % 3 == 0) {
            registry.emplace<benchmark_components::Health>(entity, healthDist(gen),
                                                           healthDist(gen));
        }
        if (gen() % 4 == 0) {
            registry.emplace<benchmark_components::Transform>(
                entity, ast::Vector2(posDist(gen), posDist(gen)));
        }
        if (gen() % 5 == 0) {
            registry.emplace<benchmark_components::Sprite>(entity,
                                                           "texture_" + std::to_string(gen() % 10));
        }
        if (gen() % 6 == 0) {
            registry.emplace<benchmark_components::AI>(entity, gen() % 4);
        }
    }
}

// Benchmarks
static void BM_EntityCreation(benchmark::State& state) {
    for (auto _ : state) {
        ast::Registry registry;
        for (int i = 0; i < state.range(0); ++i) {
            benchmark::DoNotOptimize(registry.createEntity());
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_ComponentAddition(benchmark::State& state) {
    for (auto _ : state) {
        ast::Registry registry;
        auto entities = createEntities(registry, state.range(0));

        for (auto entity : entities) {
            registry.emplace<benchmark_components::Position>(entity, 1.0f, 2.0f);
            registry.emplace<benchmark_components::Velocity>(entity, 3.0f, 4.0f);
            registry.emplace<benchmark_components::Health>(entity, 100, 100);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_ComponentRetrieval(benchmark::State& state) {
    ast::Registry registry;
    auto entities = createEntities(registry, state.range(0));

    // Add components to all entities
    for (auto entity : entities) {
        registry.emplace<benchmark_components::Position>(entity, 1.0f, 2.0f);
        registry.emplace<benchmark_components::Velocity>(entity, 3.0f, 4.0f);
        registry.emplace<benchmark_components::Health>(entity, 100, 100);
    }

    for (auto _ : state) {
        for (auto entity : entities) {
            auto* pos = registry.get<benchmark_components::Position>(entity);
            auto* vel = registry.get<benchmark_components::Velocity>(entity);
            auto* health = registry.get<benchmark_components::Health>(entity);
            benchmark::DoNotOptimize(pos);
            benchmark::DoNotOptimize(vel);
            benchmark::DoNotOptimize(health);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_SystemUpdate(benchmark::State& state) {
    ast::Registry registry;
    auto entities = createEntities(registry, state.range(0));

    // Add components to entities
    for (auto entity : entities) {
        registry.emplace<benchmark_components::Position>(entity, 1.0f, 2.0f);
        registry.emplace<benchmark_components::Velocity>(entity, 3.0f, 4.0f);
        registry.emplace<benchmark_components::Health>(entity, 100, 100);
    }

    // Attach systems
    auto& movementSystem = registry.attach<benchmark_systems::MovementSystem>(registry);
    auto& healthSystem = registry.attach<benchmark_systems::HealthSystem>(registry);

    for (auto _ : state) {
        registry.update(0.016f);  // 60 FPS delta time
    }
    state.SetComplexityN(state.range(0));
}

static void BM_EntityDeletion(benchmark::State& state) {
    for (auto _ : state) {
        ast::Registry registry;
        auto entities = createEntities(registry, state.range(0));

        // Add components
        for (auto entity : entities) {
            registry.emplace<benchmark_components::Position>(entity, 1.0f, 2.0f);
            registry.emplace<benchmark_components::Velocity>(entity, 3.0f, 4.0f);
        }

        // Delete entities
        for (auto entity : entities) {
            registry.erase(entity);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_ComponentCopying(benchmark::State& state) {
    ast::Registry registry;
    auto entities = createEntities(registry, state.range(0));

    // Create a template entity with all components
    auto templateEntity = registry.createEntity();
    registry.emplace<benchmark_components::Position>(templateEntity, 1.0f, 2.0f);
    registry.emplace<benchmark_components::Velocity>(templateEntity, 3.0f, 4.0f);
    registry.emplace<benchmark_components::Health>(templateEntity, 100, 100);
    registry.emplace<benchmark_components::Transform>(templateEntity, ast::Vector2(5.0f, 6.0f));

    for (auto _ : state) {
        for (auto entity : entities) {
            registry.copyAll<benchmark_components::Position, benchmark_components::Velocity,
                             benchmark_components::Health, benchmark_components::Transform>(
                entity, templateEntity);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_PrefabOperations(benchmark::State& state) {
    for (auto _ : state) {
        ast::Registry registry;

        // Create prefabs
        for (int i = 0; i < state.range(0); ++i) {
            std::string prefabName = "prefab_" + std::to_string(i);
            auto prefabEntity = registry.createPrefab(prefabName);
            registry.emplace<benchmark_components::Position>(prefabName, 1.0f, 2.0f);
            registry.emplace<benchmark_components::Velocity>(prefabName, 3.0f, 4.0f);
            registry.emplace<benchmark_components::Health>(prefabName, 100, 100);
        }

        // Retrieve prefabs
        for (int i = 0; i < state.range(0); ++i) {
            std::string prefabName = "prefab_" + std::to_string(i);
            auto entity = registry.get(prefabName);
            benchmark::DoNotOptimize(entity);
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_MixedOperations(benchmark::State& state) {
    for (auto _ : state) {
        ast::Registry registry;

        // Attach systems
        auto& movementSystem = registry.attach<benchmark_systems::MovementSystem>(registry);
        auto& healthSystem = registry.attach<benchmark_systems::HealthSystem>(registry);
        auto& renderSystem = registry.attach<benchmark_systems::RenderSystem>(registry);
        auto& aiSystem = registry.attach<benchmark_systems::AISystem>(registry);

        // Create entities with random components
        auto entities = createEntities(registry, state.range(0));
        addRandomComponents(registry, entities);

        // Simulate game loop
        for (int frame = 0; frame < 10; ++frame) {
            registry.update(0.016f);

            // Randomly add/remove entities
            if (frame % 3 == 0 && !entities.empty()) {
                auto entityToRemove = entities.back();
                entities.pop_back();
                registry.erase(entityToRemove);
            }

            if (frame % 2 == 0) {
                auto newEntity = registry.createEntity();
                entities.push_back(newEntity);
                addRandomComponents(registry, {newEntity});
            }
        }
    }
    state.SetComplexityN(state.range(0));
}

static void BM_SystemQueryPerformance(benchmark::State& state) {
    ast::Registry registry;
    auto entities = createEntities(registry, state.range(0));

    // Add components to entities
    for (auto entity : entities) {
        registry.emplace<benchmark_components::Position>(entity, 1.0f, 2.0f);
        registry.emplace<benchmark_components::Velocity>(entity, 3.0f, 4.0f);
        registry.emplace<benchmark_components::Health>(entity, 100, 100);
        registry.emplace<benchmark_components::Transform>(entity, ast::Vector2(5.0f, 6.0f));
        registry.emplace<benchmark_components::Sprite>(entity, "texture_1");
        registry.emplace<benchmark_components::AI>(entity, 0);
    }

    // Attach systems
    auto& movementSystem = registry.attach<benchmark_systems::MovementSystem>(registry);
    auto& healthSystem = registry.attach<benchmark_systems::HealthSystem>(registry);
    auto& renderSystem = registry.attach<benchmark_systems::RenderSystem>(registry);
    auto& aiSystem = registry.attach<benchmark_systems::AISystem>(registry);

    for (auto _ : state) {
        // Query system entities
        auto& movementEntities = movementSystem.getEntities();
        auto& healthEntities = healthSystem.getEntities();
        auto& renderEntities = renderSystem.getEntities();
        auto& aiEntities = aiSystem.getEntities();

        benchmark::DoNotOptimize(movementEntities);
        benchmark::DoNotOptimize(healthEntities);
        benchmark::DoNotOptimize(renderEntities);
        benchmark::DoNotOptimize(aiEntities);
    }
    state.SetComplexityN(state.range(0));
}

// Register benchmarks
BENCHMARK(BM_EntityCreation)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_ComponentAddition)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_ComponentRetrieval)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_SystemUpdate)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_EntityDeletion)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_ComponentCopying)->RangeMultiplier(10)->Range(10, 10000)->Complexity();
BENCHMARK(BM_PrefabOperations)->RangeMultiplier(10)->Range(10, 1000)->Complexity();
BENCHMARK(BM_MixedOperations)->RangeMultiplier(10)->Range(10, 1000)->Complexity();
BENCHMARK(BM_SystemQueryPerformance)->RangeMultiplier(10)->Range(10, 10000)->Complexity();

BENCHMARK_MAIN();