#pragma once

#include <box2d/box2d.h>

#include <vector>

#include "engine/ecs/System.hpp"
#include "engine/events/EventSubscriber.hpp"

#include "../components/RigidBody.hpp"
#include "../components/Transform.hpp"

namespace astd::systems {

struct PhysicsCommand {
    ast::Entity entity;
    enum class Type {
        APPLY_FORCE,
        APPLY_TORQUE,
        APPLY_LINEAR_IMPULSE,
        APPLY_ANGULAR_IMPULSE,
        SET_LINEAR_VELOCITY,
        SET_LINEAR_VELOCITY_X,
        SET_LINEAR_VELOCITY_Y,
        SET_ANGULAR_VELOCITY,
        SET_POSITION,
        SET_ROTATION,
    } type;
    ast::Vector2 vector;
    float scalar = 1.0f;
    ast::Vector2 point;
};

class PhysicsSystem : public ast::System<components::RigidBody, components::Transform>,
                      ast::EventSubscriber<PhysicsCommand> {
public:
    PhysicsSystem(ast::Registry& registry, const ast::Vector2& gravity = ast::Vector2(0.0f, 9.8f),
                  float timeStep = 1.0f / 60.0f);

    ~PhysicsSystem();

    void update(float dt) override;
    void onEntityAdded(ast::Entity entity) override;
    void onEntityRemoved(ast::Entity entity) override;
    void onEvent(const PhysicsCommand& event) override;
    // Physics world settings
    void setGravity(const ast::Vector2& gravity);
    ast::Vector2 getGravity() const;
    b2WorldId getWorld() const { return world_; }

private:
    void syncTransformToPhysics(ast::Entity entity);
    void syncPhysicsToTransform(ast::Entity entity, b2BodyId bodyId, b2Transform physicsTransform);
    void createRigidBody(ast::Entity entity);
    void destroyRigidBody(ast::Entity entity);
    void applyCommand(const PhysicsCommand& event);

    std::vector<PhysicsCommand> commandQueue_;
    b2WorldId world_{};

    float timeStep_ = 1.0 / 60.0f;
    float accumulator_ = 0.0f;
};

}  // namespace astd::systems