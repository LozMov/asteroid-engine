#include "PhysicsSystem.hpp"

#include "engine/ecs/Registry.hpp"
#include "../components/Sprite.hpp"

namespace astd::systems {

using ast::Entity;
using namespace components;

constexpr float PIXELS_PER_METER = 100.0f;
constexpr float METERS_PER_PIXEL = 1.0f / PIXELS_PER_METER;
constexpr float DEG_TO_RAD = B2_PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / B2_PI;

PhysicsSystem::PhysicsSystem(ast::Registry& registry, const ast::Vector2& gravity, float timeStep)
    : System(registry) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {gravity.x, gravity.y};
    worldDef.userData = this;
    world_ = b2CreateWorld(&worldDef);
}

PhysicsSystem::~PhysicsSystem() {
    // Destroy the world and all bodies
    if (b2World_IsValid(world_)) {
        b2DestroyWorld(world_);
    }
    world_ = {};
}

void PhysicsSystem::update(float dt) {
    constexpr int SUB_STEPS = 4;
    // Fixed timestep
    accumulator_ += dt;
    while (accumulator_ >= timeStep_) {
        b2World_Step(world_, timeStep_, SUB_STEPS);
        accumulator_ -= timeStep_;
    }

    // b2ContactEvents contactEvents = b2World_GetContactEvents(world_);
    // for (int i = 0; i < contactEvents.beginCount; ++i) {
    //     b2ContactBeginTouchEvent* beginEvent = contactEvents.beginEvents + i;
    //     PhysicsEvent event;
    //     event.shapeIdA = beginEvent->shapeIdA;
    //     event.shapeIdB = beginEvent->shapeIdB;
    //     event.bodyIdA = b2Shape_GetBody(beginEvent->shapeIdA);
    //     event.bodyIdB = b2Shape_GetBody(beginEvent->shapeIdB);
    //     ast::events::EventBus::publish(event);
    // }

    // Sync physics bodies to transforms
    // b2BodyEvents bodyEvents = b2World_GetBodyEvents(world_);
    // for (int i = 0; i < bodyEvents.moveCount; ++i) {
    //     b2BodyMoveEvent* moveEvent = bodyEvents.moveEvents + i;
    //     syncPhysicsToTransform(reinterpret_cast<Entity>(moveEvent->userData));
    // }

    static int test = 0;
    ++test;
    for (auto entity : entities_) {
        syncPhysicsToTransform(entity);
        if (test == 60) {
            auto* r = registry_.get<RigidBody>(entity);
            auto c1 = b2Body_GetPosition(r->handle);
            auto c2 = b2Body_GetWorldCenterOfMass(r->handle);
            AST_DEBUG("Position: ({}, {})", c1.x, c1.y);
            AST_DEBUG("Center of mass: ({}, {})", c2.x, c2.y);
            test = 0;
        }
    }
}

void PhysicsSystem::onEntityAdded(Entity entity) {
    auto* transform = registry_.get<Transform>(entity);
    auto* rigidBody = registry_.get<RigidBody>(entity);
    auto* sprite = registry_.get<Sprite>(entity);
    if (b2Body_IsValid(rigidBody->handle)) {
        // Destroy the body if it already exists
        b2DestroyBody(rigidBody->handle);
    }

    // Calculate physics body position (center of sprite)
    ast::Vector2 physicsPos = transform->position;
    if (sprite) {
        float logicalSizeX = sprite->size.x * transform->scale.x;
        float logicalSizeY = sprite->size.y * transform->scale.y;
        // Get to the center from any origin point
        physicsPos.x += (0.5f - sprite->origin.x) * logicalSizeX;
        physicsPos.y += (0.5f - sprite->origin.y) * logicalSizeY;
    }

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = static_cast<b2BodyType>(rigidBody->bodyType);
    bodyDef.position = {physicsPos.x * METERS_PER_PIXEL, physicsPos.y * METERS_PER_PIXEL};
    bodyDef.rotation = b2MakeRot(transform->rotation * DEG_TO_RAD);
    bodyDef.fixedRotation = rigidBody->fixedRotation;
    bodyDef.linearDamping = rigidBody->linearDamping;
    bodyDef.linearVelocity = {rigidBody->linearVelocity.x, rigidBody->linearVelocity.y};
    bodyDef.angularVelocity = rigidBody->angularVelocity;
    bodyDef.angularDamping = rigidBody->angularDamping;
    bodyDef.gravityScale = rigidBody->gravityScale;
    bodyDef.name = "test";
    bodyDef.userData = reinterpret_cast<void*>(entity);
    rigidBody->handle = b2CreateBody(world_, &bodyDef);
    float halfWidth = rigidBody->size.x * 0.5f * transform->scale.x * METERS_PER_PIXEL;
    float halfHeight = rigidBody->size.y * 0.5f * transform->scale.y * METERS_PER_PIXEL;
    b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.maskBits = static_cast<uint16_t>(rigidBody->collisionCategory);
    shapeDef.userData = reinterpret_cast<void*>(entity);
    // shapeDef.density = rigidBody->mass;
    shapeDef.material.friction = rigidBody->friction;
    shapeDef.material.restitution = rigidBody->restitution;
    shapeDef.isSensor = false;
    shapeDef.enableContactEvents = true;
    shapeDef.filter.maskBits = static_cast<uint16_t>(rigidBody->collisionCategory);
    b2CreatePolygonShape(rigidBody->handle, &shapeDef, &polygon);
}

void PhysicsSystem::onEntityRemoved(Entity entity) {
    auto* rigidBody = registry_.get<RigidBody>(entity);
    if (b2Body_IsValid(rigidBody->handle)) {
        b2DestroyBody(rigidBody->handle);
    }
}

void PhysicsSystem::setGravity(const ast::Vector2& gravity) {
    b2Vec2 b2Gravity;
    b2Gravity.x = gravity.x;
    b2Gravity.y = gravity.y;
    b2World_SetGravity(world_, b2Gravity);
}

ast::Vector2 PhysicsSystem::getGravity() const {
    b2Vec2 gravity = b2World_GetGravity(world_);
    return ast::Vector2(gravity.x, gravity.y);
}

void PhysicsSystem::syncTransformToPhysics(Entity entity) {
    auto* transform = registry_.get<Transform>(entity);
    auto* rigidBody = registry_.get<RigidBody>(entity);
}

void PhysicsSystem::syncPhysicsToTransform(Entity entity) {
    auto* transform = registry_.get<Transform>(entity);
    auto* rigidBody = registry_.get<RigidBody>(entity);
    auto* sprite = registry_.get<Sprite>(entity);
    if (b2Body_IsValid(rigidBody->handle)) {
        b2Transform physicsTransform = b2Body_GetTransform(rigidBody->handle);

        // Get physics body position in pixels (center of sprite)
        ast::Vector2 physicsPos = {physicsTransform.p.x * PIXELS_PER_METER,
                                   physicsTransform.p.y * PIXELS_PER_METER};

        // Calculate transform position (sprite origin point) from center
        if (sprite) {
            float logicalSizeX = sprite->size.x * transform->scale.x;
            float logicalSizeY = sprite->size.y * transform->scale.y;
            transform->position = {physicsPos.x - (0.5f - sprite->origin.x) * logicalSizeX,
                                   physicsPos.y - (0.5f - sprite->origin.y) * logicalSizeY};
        } else {
            // No sprite, use physics position directly
            transform->position = physicsPos;
        }

        transform->rotation = b2Rot_GetAngle(physicsTransform.q) * RAD_TO_DEG;
    }
}

void PhysicsSystem::createRigidBody(Entity entity) {}

void PhysicsSystem::destroyRigidBody(Entity entity) {}

}  // namespace astd::systems