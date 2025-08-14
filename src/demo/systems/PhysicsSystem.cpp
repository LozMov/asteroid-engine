#include "PhysicsSystem.hpp"

#include "engine/ecs/Registry.hpp"
#include "../components/Player.hpp"
#include "../components/Sprite.hpp"

namespace astd::systems {

using ast::Entity;
using namespace components;

constexpr float PIXELS_PER_METER = 100.0f;
constexpr float METERS_PER_PIXEL = 1.0f / PIXELS_PER_METER;
constexpr float DEG_TO_RAD = B2_PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / B2_PI;

static bool preSolveOneSidedPlatform(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold,
                                     void* context) {
    SDL_assert(b2Shape_IsValid(shapeIdA));
    SDL_assert(b2Shape_IsValid(shapeIdB));
    b2BodyId bodyIdA = b2Shape_GetBody(shapeIdA);
    b2BodyId bodyIdB = b2Shape_GetBody(shapeIdB);
    float sign = 0.0f;
    if (std::strcmp(b2Body_GetName(bodyIdA), "Player") == 0) {
        sign = 1.0f;
    } else if (std::strcmp(b2Body_GetName(bodyIdB), "Player") == 0) {
        sign = -1.0f;
    } else {
        return true;  // Do nothing
    }
    // Normal: (0, 1) when the player is below the platform
    return sign * manifold->normal.y > 0.95f;
}

PhysicsSystem::PhysicsSystem(ast::Registry& registry, const ast::Vector2& gravity, float timeStep)
    : System(registry) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {gravity.x, gravity.y};
    worldDef.userData = this;
    world_ = b2CreateWorld(&worldDef);
    b2World_SetPreSolveCallback(world_, preSolveOneSidedPlatform, nullptr);
}

PhysicsSystem::~PhysicsSystem() {
    // Destroy the world and all bodies
    if (b2World_IsValid(world_)) {
        b2DestroyWorld(world_);
    }
    world_ = {};
}

void PhysicsSystem::update(float dt) {
    for (const auto& event : commandQueue_) {
        applyCommand(event);
    }
    commandQueue_.clear();

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
    auto sensorEvents = b2World_GetSensorEvents(world_);
    for (int i = 0; i < sensorEvents.beginCount; ++i) {
        AST_DEBUG("Sensor begin touch events count: {}", sensorEvents.beginCount);
        const b2SensorBeginTouchEvent& beginEvent = sensorEvents.beginEvents[i];
        auto bodyA = b2Shape_GetBody(beginEvent.sensorShapeId);
        auto bodyB = b2Shape_GetBody(beginEvent.visitorShapeId);
        if (std::strcmp(b2Body_GetName(bodyA), "Player") == 0) {
            auto entity = reinterpret_cast<ast::Entity>(b2Body_GetUserData(bodyA));
            if (auto* player = registry_.get<Player>(entity)) {
                if (std::strstr(b2Body_GetName(bodyB), "Enemy")) {
                    AST_DEBUG("Land on an enemy");
                    // TODO: kill the enemy
                    auto enemyEntity = reinterpret_cast<ast::Entity>(b2Body_GetUserData(bodyB));
                    registry_.markAsExpired(enemyEntity);
                    ast::EventBus::publish(
                        PhysicsCommand{entity,
                                       PhysicsCommand::Type::APPLY_LINEAR_IMPULSE,
                                       {0.0f, player->jumpImpulse * -0.3f}});
                } else {
                    ++player->groundContactCount;
                    AST_DEBUG("Player on ground, contact count: {}", player->groundContactCount);
                    player->onGround = true;
                    player->coyoteTimer = player->coyoteDuration;
                }
            }
        }
    }

    for (int i = 0; i < sensorEvents.endCount; ++i) {
        AST_DEBUG("Sensor end touch events count: {}", sensorEvents.endCount);
        const b2SensorEndTouchEvent& endEvent = sensorEvents.endEvents[i];
        auto bodyA = b2Shape_GetBody(endEvent.sensorShapeId);
        auto bodyB = b2Shape_GetBody(endEvent.visitorShapeId);
        if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) {
            AST_WARN("Invalid body in sensor end event");
            continue;
        }
        if (std::strcmp(b2Body_GetName(bodyA), "Player") == 0) {
            auto entity = reinterpret_cast<ast::Entity>(b2Body_GetUserData(bodyA));
            if (auto* player = registry_.get<Player>(entity)) {
                player->groundContactCount = std::max(0, player->groundContactCount - 1);
                AST_DEBUG("Player left ground, contact count: {}", player->groundContactCount);
                if (player->groundContactCount == 0) {
                    player->onGround = false;
                    player->coyoteTimer = player->coyoteDuration;
                }
            }
        }
    }

    b2BodyEvents bodyEvents = b2World_GetBodyEvents(world_);
    for (int i = 0; i < bodyEvents.moveCount; ++i) {
        const auto& moveEvent = bodyEvents.moveEvents[i];
        syncPhysicsToTransform(reinterpret_cast<Entity>(moveEvent.userData), moveEvent.bodyId,
                               moveEvent.transform);
    }
}

void PhysicsSystem::onEntityAdded(Entity entity) {
    auto* transform = registry_.get<Transform>(entity);
    auto* rigidBody = registry_.get<RigidBody>(entity);
    auto* sprite = registry_.get<Sprite>(entity);
    if (b2Body_IsValid(rigidBody->handle)) {
        // Destroy the body if it already exists
        b2DestroyBody(rigidBody->handle);
        AST_WARN("Existing body destroyed");
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
    bodyDef.linearVelocity = rigidBody->linearVelocity;
    bodyDef.angularVelocity = rigidBody->angularVelocity;
    bodyDef.angularDamping = rigidBody->angularDamping;
    bodyDef.gravityScale = rigidBody->gravityScale;
    bodyDef.name = rigidBody->name.c_str();
    AST_INFO("Creating physics body for entity {}: name={}, pos=({}, {}), type={}", entity,
             bodyDef.name, physicsPos.x, physicsPos.y, static_cast<int>(rigidBody->bodyType));
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
    shapeDef.enableSensorEvents = true;

    if (rigidBody->name == "Player") {
        AST_INFO("Player physics body creation details:");
        AST_INFO("  - Transform: pos=({}, {}), scale=({}, {})", transform->position.x,
                 transform->position.y, transform->scale.x, transform->scale.y);
        AST_INFO("  - Sprite: size=({}, {}), origin=({}, {})", sprite ? sprite->size.x : 0,
                 sprite ? sprite->size.y : 0, sprite ? sprite->origin.x : 0,
                 sprite ? sprite->origin.y : 0);
        AST_INFO("  - RigidBody: size=({}, {}), category={}", rigidBody->size.x, rigidBody->size.y,
                 static_cast<int>(rigidBody->collisionCategory));
        AST_INFO("  - Physics: pos=({}, {}), meters=({}, {})", physicsPos.x, physicsPos.y,
                 physicsPos.x * METERS_PER_PIXEL, physicsPos.y * METERS_PER_PIXEL);
    } else if (rigidBody->name.find("Platform") != std::string::npos) {
        AST_INFO("Platform physics body creation details:");
        AST_INFO("  - Transform: pos=({}, {}), scale=({}, {})", transform->position.x,
                 transform->position.y, transform->scale.x, transform->scale.y);
        AST_INFO("  - Sprite: size=({}, {}), origin=({}, {})", sprite ? sprite->size.x : 0,
                 sprite ? sprite->size.y : 0, sprite ? sprite->origin.x : 0,
                 sprite ? sprite->origin.y : 0);
        AST_INFO("  - RigidBody: size=({}, {}), category={}", rigidBody->size.x, rigidBody->size.y,
                 static_cast<int>(rigidBody->collisionCategory));
        AST_INFO("  - Physics: pos=({}, {}), meters=({}, {})", physicsPos.x, physicsPos.y,
                 physicsPos.x * METERS_PER_PIXEL, physicsPos.y * METERS_PER_PIXEL);
        shapeDef.enablePreSolveEvents = true;
    }
    shapeDef.filter.maskBits = static_cast<uint16_t>(rigidBody->collisionCategory);
    b2CreatePolygonShape(rigidBody->handle, &shapeDef, &polygon);

    // Add player sensor
    // A sensor is a shape that detects overlap but does not produce a response
    if (auto* player = registry_.get<Player>(entity)) {
        AST_INFO("Adding Player component sensor for entity {}", entity);

        // Only add sensor if the body is valid
        if (!b2Body_IsValid(rigidBody->handle)) {
            AST_ERROR("Cannot add player sensor: invalid rigid body");
            return;
        }
        float playerSensorHalfWidth = halfWidth * 0.5f;
        float playerSensorHalfHeight = halfHeight * 0.5f;
        b2Polygon playerSensorShape =
            b2MakeOffsetBox(playerSensorHalfWidth, playerSensorHalfHeight,
                            {0.0f, playerSensorHalfHeight}, b2MakeRot(0.0f));
        b2ShapeDef playerSensorDef = b2DefaultShapeDef();
        playerSensorDef.isSensor = true;
        playerSensorDef.enableSensorEvents = true;
        playerSensorDef.userData = reinterpret_cast<void*>(entity);
        b2Body_SetName(rigidBody->handle, "Player");
        AST_INFO("Adding player sensor to body: {}", b2Body_GetName(rigidBody->handle));
        b2CreatePolygonShape(rigidBody->handle, &playerSensorDef, &playerSensorShape);
        AST_INFO("Player sensor added successfully for entity {}", entity);
    }

    if (rigidBody && rigidBody->name == "Player") {
        AST_INFO("Player physics body setup completed for entity {}", entity);
    }
    b2World_Step(world_, timeStep_, 4);
}

void PhysicsSystem::onEntityRemoved(Entity entity) {
    auto* rigidBody = registry_.get<RigidBody>(entity);
    if (b2Body_IsValid(rigidBody->handle)) {
        b2DestroyBody(rigidBody->handle);
    }
}

void PhysicsSystem::onEvent(const PhysicsCommand& event) { commandQueue_.push_back(event); }

void PhysicsSystem::applyCommand(const PhysicsCommand& event) {
    auto* rigidBody = registry_.get<RigidBody>(event.entity);
    if (!rigidBody || !b2Body_IsValid(rigidBody->handle)) {
        AST_ERROR("Rigid body not found or invalid");
        return;
    }
    switch (event.type) {
        case PhysicsCommand::Type::APPLY_FORCE:
            b2Body_ApplyForce(rigidBody->handle, event.vector,
                              b2Body_GetWorldPoint(rigidBody->handle, event.point), true);
            break;
        case PhysicsCommand::Type::APPLY_TORQUE:
            b2Body_ApplyTorque(rigidBody->handle, event.scalar, true);
            break;
        case PhysicsCommand::Type::APPLY_LINEAR_IMPULSE:
            b2Body_ApplyLinearImpulse(rigidBody->handle, event.vector,
                                      b2Body_GetWorldPoint(rigidBody->handle, event.point), true);
            break;
        case PhysicsCommand::Type::APPLY_ANGULAR_IMPULSE:
            b2Body_ApplyAngularImpulse(rigidBody->handle, event.scalar, true);
            break;
        case PhysicsCommand::Type::SET_LINEAR_VELOCITY_X:
            b2Body_SetLinearVelocity(rigidBody->handle,
                                     {event.scalar, b2Body_GetLinearVelocity(rigidBody->handle).y});
            break;
        case PhysicsCommand::Type::SET_LINEAR_VELOCITY_Y:
            b2Body_SetLinearVelocity(rigidBody->handle,
                                     {b2Body_GetLinearVelocity(rigidBody->handle).x, event.scalar});
            break;
        case PhysicsCommand::Type::SET_LINEAR_VELOCITY:
            b2Body_SetLinearVelocity(rigidBody->handle, event.vector);
            break;
        case PhysicsCommand::Type::SET_ANGULAR_VELOCITY:
            b2Body_SetAngularVelocity(rigidBody->handle, event.scalar);
            break;
    }
}

void PhysicsSystem::setGravity(const ast::Vector2& gravity) { b2World_SetGravity(world_, gravity); }

ast::Vector2 PhysicsSystem::getGravity() const { return b2World_GetGravity(world_); }

void PhysicsSystem::syncTransformToPhysics(Entity entity) {
    auto* transform = registry_.get<Transform>(entity);
    auto* rigidBody = registry_.get<RigidBody>(entity);
}

void PhysicsSystem::syncPhysicsToTransform(Entity entity, b2BodyId bodyId,
                                           b2Transform physicsTransform) {
    auto* transform = registry_.get<Transform>(entity);
    auto* sprite = registry_.get<Sprite>(entity);
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

void PhysicsSystem::createRigidBody(Entity entity) {}

void PhysicsSystem::destroyRigidBody(Entity entity) {}

}  // namespace astd::systems