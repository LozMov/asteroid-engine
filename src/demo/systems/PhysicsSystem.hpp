#pragma once

#include <box2d/box2d.h>

// #include <unordered_map>

#include "engine/ecs/System.hpp"

#include "../components/RigidBody.hpp"
#include "../components/Transform.hpp"

namespace astd::systems {

class PhysicsSystem : public ast::System<components::RigidBody, components::Transform> {
public:
    PhysicsSystem(ast::Registry& registry, const ast::Vector2& gravity = ast::Vector2(0.0f, 9.8f),
                  float timeStep = 1.0f / 60.0f);

    ~PhysicsSystem();

    void update(float dt) override;
    void onEntityAdded(ast::Entity entity) override;
    void onEntityRemoved(ast::Entity entity) override;

    // Physics world settings
    void setGravity(const ast::Vector2& gravity);
    ast::Vector2 getGravity() const;
    b2WorldId getWorld() const { return world_; }

private:
    void syncTransformToPhysics(ast::Entity entity);
    void syncPhysicsToTransform(ast::Entity entity);
    void createRigidBody(ast::Entity entity);
    void destroyRigidBody(ast::Entity entity);

    // std::unordered_map<ast::Entity, b2BodyId> bodies_;
    b2WorldId world_{};

    float timeStep_ = 1.0 / 60.0f;
    float accumulator_ = 0.0f;
};

}  // namespace astd::systems