#include "Prefab.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

#include "engine/Color.hpp"
#include "engine/Log.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Registry.hpp"

#include "components/Animation.hpp"
#include "components/RigidBody.hpp"
#include "components/Sprite.hpp"
#include "components/Transform.hpp"

#define GET_PROPERTY(comp, prop) \
    if (compData.contains(#prop)) comp.prop = compData[#prop].get<decltype(comp.prop)>()

namespace ast {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Color, r, g, b, a)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Vector2, x, y)

}  // namespace ast

namespace astd::components {

void to_json(nlohmann::json& j, const RigidBody::BodyType& bodyType) {
    switch (bodyType) {
        case RigidBody::BodyType::STATIC:
            j = "STATIC";
            break;
        case RigidBody::BodyType::KINEMATIC:
            j = "KINEMATIC";
            break;
        case RigidBody::BodyType::DYNAMIC:
            j = "DYNAMIC";
            break;
    }
}

void from_json(const nlohmann::json& j, RigidBody::BodyType& bodyType) {
    std::string str = j.get<std::string>();
    if (str == "STATIC")
        bodyType = RigidBody::BodyType::STATIC;
    else if (str == "KINEMATIC")
        bodyType = RigidBody::BodyType::KINEMATIC;
    else if (str == "DYNAMIC")
        bodyType = RigidBody::BodyType::DYNAMIC;
    else
        bodyType = RigidBody::BodyType::STATIC;
}

void to_json(nlohmann::json& j, const RigidBody::CollisionCategory& category) {
    switch (category) {
        case RigidBody::CollisionCategory::NONE:
            j = "NONE";
            break;
        case RigidBody::CollisionCategory::PLAYER:
            j = "PLAYER";
            break;
        case RigidBody::CollisionCategory::ENEMY:
            j = "ENEMY";
            break;
        case RigidBody::CollisionCategory::PLATFORM:
            j = "PLATFORM";
            break;
        case RigidBody::CollisionCategory::COLLECTIBLE:
            j = "COLLECTIBLE";
            break;
        case RigidBody::CollisionCategory::OBSTACLE:
            j = "OBSTACLE";
            break;
        case RigidBody::CollisionCategory::PLAYER_ATTACK:
            j = "PLAYER_ATTACK";
            break;
        case RigidBody::CollisionCategory::ENEMY_ATTACK:
            j = "ENEMY_ATTACK";
            break;
        case RigidBody::CollisionCategory::ONE_WAY_PLATFORM:
            j = "ONE_WAY_PLATFORM";
            break;
        case RigidBody::CollisionCategory::ALL:
            j = "ALL";
            break;
    }
}

void from_json(const nlohmann::json& j, RigidBody::CollisionCategory& category) {
    std::string str = j.get<std::string>();
    if (str == "NONE")
        category = RigidBody::CollisionCategory::NONE;
    else if (str == "PLAYER")
        category = RigidBody::CollisionCategory::PLAYER;
    else if (str == "ENEMY")
        category = RigidBody::CollisionCategory::ENEMY;
    else if (str == "PLATFORM")
        category = RigidBody::CollisionCategory::PLATFORM;
    else if (str == "COLLECTIBLE")
        category = RigidBody::CollisionCategory::COLLECTIBLE;
    else if (str == "OBSTACLE")
        category = RigidBody::CollisionCategory::OBSTACLE;
    else if (str == "PLAYER_ATTACK")
        category = RigidBody::CollisionCategory::PLAYER_ATTACK;
    else if (str == "ENEMY_ATTACK")
        category = RigidBody::CollisionCategory::ENEMY_ATTACK;
    else if (str == "ONE_WAY_PLATFORM")
        category = RigidBody::CollisionCategory::ONE_WAY_PLATFORM;
    else if (str == "ALL")
        category = RigidBody::CollisionCategory::ALL;
    else
        throw std::runtime_error("Invalid CollisionCategory: " + str);
}

void from_json(const nlohmann::json& j, Sprite::ScalingMode& scalingMode) {
    std::string str = j.get<std::string>();
    if (str == "TILED") {
        scalingMode = Sprite::ScalingMode::TILED;
    } else if (str == "STRETCH") {
        scalingMode = Sprite::ScalingMode::STRETCH;
    } else {
        scalingMode = Sprite::ScalingMode::NONE;
    }
}

void from_json(const nlohmann::json& j, Sprite::FlipMode& flipMode) {
    std::string str = j.get<std::string>();
    if (str == "HORIZONTAL") {
        flipMode = Sprite::FlipMode::HORIZONTAL;
    } else if (str == "VERTICAL") {
        flipMode = Sprite::FlipMode::VERTICAL;
    } else {
        flipMode = Sprite::FlipMode::NONE;
    }
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Animation, frameCount, frameStartIndex,
                                                frameDuration, loop, playing)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Transform, position, rotation, scale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RigidBody, bodyType, collisionCategory, size,
                                                linearVelocity, angularVelocity, mass, restitution,
                                                friction, gravityScale, linearDamping,
                                                angularDamping, fixedRotation, isEnabled, isBullet)

using nlohmann::json;

static Sprite loadSprite(const std::string& prefabName, const json& compData) {
    if (compData.contains("fileName")) {
        if (compData.contains("frameCountX")) {
            // Sprite sheet
            Sprite sprite(compData["fileName"], compData["frameCountX"], compData["frameCountY"]);
            sprite.frameIndex = compData.value("frameIndex", 0);
            GET_PROPERTY(sprite, scalingMode);
            GET_PROPERTY(sprite, flipMode);
            return sprite;
        } else {
            Sprite sprite(compData["fileName"].get<std::string>());
            GET_PROPERTY(sprite, scalingMode);
            GET_PROPERTY(sprite, flipMode);
            return sprite;
        }
    } else if (compData.contains("color")) {
        Sprite sprite(compData["color"].get<ast::Color>());
        return sprite;
    }
    return Sprite{};
}

std::vector<std::string> Prefab::loadPrefabsFromFile(ast::Registry& registry,
                                                     const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        AST_ERROR("Failed to load prefabs from file '{}'", fileName);
        return {};
    }

    std::vector<std::string> loadedPrefabs;
    nlohmann::json prefabsJson;
    file >> prefabsJson;

    for (const auto& prefab : prefabsJson) {
        std::string prefabName = prefab["name"];
        AST_INFO("Creating prefab: '{}'", prefabName);

        auto entity = registry.createPrefab(prefabName);

        for (const auto& [compName, compData] : prefab["components"].items()) {
            if (compName == "Animation") {
                registry.insert(prefabName, compData.get<Animation>());
            } else if (compName == "Sprite") {
                auto sprite = loadSprite(prefabName, compData);
                GET_PROPERTY(sprite, origin);
                GET_PROPERTY(sprite, logicalSize);
                GET_PROPERTY(sprite, frameIndex);
                GET_PROPERTY(sprite, visible);
                GET_PROPERTY(sprite, zIndex);
                registry.insert(prefabName, sprite);
            } else if (compName == "Transform") {
                registry.insert(prefabName, compData.get<Transform>());
            } else if (compName == "RigidBody") {
                registry.insert(prefabName, compData.get<RigidBody>());
            } else {
                AST_WARN("Unknown component type: {}", compName);
            }
        }
        AST_INFO("Prefab loaded: '{}'", prefabName);
        loadedPrefabs.push_back(prefabName);
    }
    return loadedPrefabs;
}

ast::Entity Prefab::createEntityFromPrefab(ast::Registry& registry, const std::string& prefabName) {
    auto entity = registry.createEntity();
    auto prefab = registry.get(prefabName);
    registry.copyAll<Animation, Sprite, Transform, RigidBody>(entity, prefab);
    return entity;
}

}  // namespace astd::components