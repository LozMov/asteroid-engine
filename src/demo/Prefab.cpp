#include "Prefab.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

#include "engine/Color.hpp"
#include "engine/Log.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Registry.hpp"

#include "components/Animation.hpp"
#include "components/Camera.hpp"
#include "components/RigidBody.hpp"
#include "components/Sprite.hpp"
#include "components/Transform.hpp"
#include "components/Player.hpp"

#define GET_PROPERTY(data, comp, prop) \
    if (data.contains(#prop)) comp.prop = data[#prop].get<decltype(comp.prop)>()

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
    if (str == "KINEMATIC") {
        bodyType = RigidBody::BodyType::KINEMATIC;
    } else if (str == "DYNAMIC") {
        bodyType = RigidBody::BodyType::DYNAMIC;
    } else {
        bodyType = RigidBody::BodyType::STATIC;
    }
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
    } else if (str == "NINE_GRID") {
        scalingMode = Sprite::ScalingMode::NINE_GRID;
    } else {
        scalingMode = Sprite::ScalingMode::DEFAULT;
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Camera, target, position, screenSize, lerp, zoom)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Animation, frameCount, frameStartIndex,
                                                frameDuration, loop, playing)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Transform, position, rotation, scale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RigidBody, bodyType, collisionCategory, name, size,
                                                linearVelocity, angularVelocity, mass, restitution,
                                                friction, gravityScale, linearDamping,
                                                angularDamping, fixedRotation, isEnabled, isBullet)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Player, walkSpeed, runSpeed, airSpeed, jumpImpulse, jumpBufferDuration,
                                                coyoteDuration)

using nlohmann::json;

static Sprite loadSprite(const std::string& prefabName, const json& j) {
    Sprite sprite =
        j.contains("fileName")
            ? j.value("scalingMode", Sprite::ScalingMode::DEFAULT) == Sprite::ScalingMode::NINE_GRID
                  ? Sprite(j["fileName"], j["leftWidth"], j["rightWidth"], j["topHeight"],
                           j["bottomHeight"])
                  : Sprite(j["fileName"].get<std::string>())

            : Sprite(j["color"].get<ast::Color>());
    
    if (prefabName == "Player") {
        AST_INFO("Player sprite loaded: fileName={}, texture valid={}, size=({}, {})", 
                 j.value("fileName", "none"), sprite.texture.handle != nullptr, 
                 sprite.size.x, sprite.size.y);
    }
    if (j.contains("frameCountX") || j.contains("frameCountY")) {
        sprite.calculateFrames(j.value("frameCountX", 1), j.value("frameCountY", 1));
    }
    GET_PROPERTY(j, sprite, size);
    GET_PROPERTY(j, sprite, origin);
    GET_PROPERTY(j, sprite, parallaxFactor);
    GET_PROPERTY(j, sprite, zIndex);
    GET_PROPERTY(j, sprite, frameIndex);
    GET_PROPERTY(j, sprite, scalingMode);
    GET_PROPERTY(j, sprite, flipMode);
    GET_PROPERTY(j, sprite, visible);
    GET_PROPERTY(j, sprite, isBackground);
    return sprite;
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
        auto entity = registry.createPrefab(prefabName);

        for (const auto& [compName, compData] : prefab["components"].items()) {
            if (compName == "Animation") {
                registry.insert(prefabName, compData.get<Animation>());
            } else if (compName == "Sprite") {
                registry.insert(prefabName, loadSprite(prefabName, compData));
            } else if (compName == "Transform") {
                auto transform = compData.get<Transform>();
                if (prefabName == "Player") {
                    AST_INFO("Player transform loaded: pos=({}, {}), scale=({}, {})", 
                             transform.position.x, transform.position.y, 
                             transform.scale.x, transform.scale.y);
                }
                registry.insert(prefabName, transform);
            } else if (compName == "RigidBody") {
                auto rigidBody = compData.get<RigidBody>();
                rigidBody.name = compData.value("name", prefabName);
                registry.insert(prefabName, rigidBody);
            } else if (compName == "Player") {
                auto player = compData.get<Player>();
                registry.insert(prefabName, player);
            } else if (compName == "Camera") {
                auto camera = compData.get<Camera>();
                registry.insert(prefabName, camera);
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
    registry.copyAll<Animation, Sprite, Transform, RigidBody, Player, Camera>(entity, prefab, false);
    registry.forceAdd(entity);
    return entity;
}

}  // namespace astd::components