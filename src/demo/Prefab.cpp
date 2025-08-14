#include "Prefab.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

#include "engine/Audio.hpp"
#include "engine/Cache.hpp"
#include "engine/Color.hpp"
#include "engine/Log.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Registry.hpp"

#include "components/Components.hpp"
#include "systems/PhysicsSystem.hpp"

#define GET_PROPERTY(data, comp, prop) \
    if (data.contains(#prop)) (comp).prop = data[#prop].get<decltype((comp).prop)>()

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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Player, walkSpeed, runSpeed, airSpeed, jumpImpulse,
                                                jumpBufferDuration, coyoteDuration)

using nlohmann::json;

// TODO
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
                 j.value("fileName", "none"), sprite.texture.handle != nullptr, sprite.size.x,
                 sprite.size.y);
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
                // if (prefabName == "Player") {
                //     AST_INFO("Player transform loaded: pos=({}, {}), scale=({}, {})",
                //              transform.position.x, transform.position.y,
                //              transform.scale.x, transform.scale.y);
                // }
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
    registry.copyAll<Animation, Sprite, Transform, RigidBody, Player, Camera>(entity, prefab,
                                                                              false);
    registry.forceCheck(entity);
    return entity;
}

std::vector<Prefab::EntityData> Prefab::loadLevel(ast::Registry& registry,
                                                  const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        AST_ERROR("Failed to load level from file '{}'", fileName);
        return {};
    }

    json levelData;
    file >> levelData;

    std::vector<EntityData> entities;

    // Load level properties
    if (levelData.contains("properties")) {
        auto& props = levelData["properties"];
        // Handle level-wide properties like gravity, background color, etc.
        if (props.contains("gravity")) {
            if (auto* physicsSystem = registry.get<systems::PhysicsSystem>()) {
                physicsSystem->setGravity(props["gravity"].get<ast::Vector2>());
            }
        }
        if (props.contains("backgroundMusic")) {
            std::string fileName = props["backgroundMusic"].value("fileName", "background.wav");
            int loops = props["backgroundMusic"].value("loops", 0);
            float gain = props["backgroundMusic"].value("gain", 1.0f);  
            ast::Audio::getInstance().playSound(fileName, loops);
            ast::Audio::getInstance().setSoundGain(fileName, gain);
        }
    }

    // Load entities
    if (!levelData.contains("entities")) {
        AST_ERROR("No entities found in level file '{}'", fileName);
        return entities;
    }

    for (const auto& entityData : levelData["entities"]) {
        std::string prefabName = entityData["prefab"];
        auto entity = registry.createEntity();
        auto prefab = registry.get(prefabName);
        registry.copyAll<Animation, Sprite, Transform, RigidBody, Player, Camera>(entity, prefab,
                                                                              false);

        // Override properties if specified
        if (entityData.contains("overrides")) {
            const auto& overrides = entityData["overrides"];

            if (overrides.contains("Animation")) {
                auto* animation = registry.get<Animation>(entity);
                if (animation) {
                    const auto& aData = overrides["Animation"];
                    GET_PROPERTY(aData, *animation, frameCount);
                    GET_PROPERTY(aData, *animation, frameStartIndex);
                    GET_PROPERTY(aData, *animation, frameDuration);
                    GET_PROPERTY(aData, *animation, loop);
                    GET_PROPERTY(aData, *animation, playing);
                }
            }

            if (overrides.contains("Transform")) {
                auto* transform = registry.get<Transform>(entity);
                if (transform) {
                    const auto& tData = overrides["Transform"];
                    GET_PROPERTY(tData, *transform, position);
                    GET_PROPERTY(tData, *transform, rotation);
                    GET_PROPERTY(tData, *transform, scale);
                }
            }

            if (overrides.contains("Sprite")) {
                // Cannot change sprite constructors 
                if (auto* sprite = registry.get<Sprite>(entity)) {
                    const auto& sData = overrides["Sprite"];
                    GET_PROPERTY(sData, *sprite, size);
                    GET_PROPERTY(sData, *sprite, origin);
                    GET_PROPERTY(sData, *sprite, parallaxFactor);
                    GET_PROPERTY(sData, *sprite, scalingMode);
                    GET_PROPERTY(sData, *sprite, isBackground);
                    GET_PROPERTY(sData, *sprite, isFrame);
                    GET_PROPERTY(sData, *sprite, frameIndex);
                    GET_PROPERTY(sData, *sprite, zIndex);
                    GET_PROPERTY(sData, *sprite, flipMode);
                    GET_PROPERTY(sData, *sprite, visible);
                    GET_PROPERTY(sData, *sprite, frameCountX);
                    GET_PROPERTY(sData, *sprite, frameCountY);
                    
                    // Special case for fileName
                    if (sData.contains("fileName")) {
                        sprite->texture = ast::Cache::getTexture(sData["fileName"].get<std::string>());
                    }
                }
            }

            if (overrides.contains("RigidBody")) {
                auto* rigidBody = registry.get<RigidBody>(entity);
                if (rigidBody) {
                    const auto& rbData = overrides["RigidBody"];
                    GET_PROPERTY(rbData, *rigidBody, bodyType);
                    GET_PROPERTY(rbData, *rigidBody, collisionCategory);
                    GET_PROPERTY(rbData, *rigidBody, name);
                    GET_PROPERTY(rbData, *rigidBody, size);
                    GET_PROPERTY(rbData, *rigidBody, linearVelocity);
                    GET_PROPERTY(rbData, *rigidBody, angularVelocity);
                    GET_PROPERTY(rbData, *rigidBody, mass);
                    GET_PROPERTY(rbData, *rigidBody, restitution);
                    GET_PROPERTY(rbData, *rigidBody, friction);
                    GET_PROPERTY(rbData, *rigidBody, gravityScale);
                    GET_PROPERTY(rbData, *rigidBody, linearDamping);
                    GET_PROPERTY(rbData, *rigidBody, angularDamping);
                    GET_PROPERTY(rbData, *rigidBody, fixedRotation);
                    GET_PROPERTY(rbData, *rigidBody, isEnabled);
                    GET_PROPERTY(rbData, *rigidBody, isBullet);
                }
            }

            if (overrides.contains("Player")) {
                auto* player = registry.get<Player>(entity);
                if (player) {
                    const auto& pData = overrides["Player"];
                    GET_PROPERTY(pData, *player, walkSpeed);
                    GET_PROPERTY(pData, *player, runSpeed);
                    GET_PROPERTY(pData, *player, airSpeed);
                    GET_PROPERTY(pData, *player, jumpImpulse);
                    GET_PROPERTY(pData, *player, jumpBufferDuration);
                    GET_PROPERTY(pData, *player, coyoteDuration);
                }
            }
        }

        // Check after overrides to ensure all components are valid
        registry.forceCheck(entity);
        entities.push_back({prefabName, entity});
        AST_DEBUG("Created entity from prefab '{}' (id: {})", prefabName, entity);
    }

    AST_INFO("Level loaded with {} entities", entities.size());
    return entities;
}

}  // namespace astd::components