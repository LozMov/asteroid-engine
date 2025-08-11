#include "Prefab.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

#include "engine/Color.hpp"
#include "engine/Log.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Registry.hpp"

#include "components/Animation.hpp"
#include "components/Sprite.hpp"
#include "components/Transform.hpp"
#include "components/Velocity.hpp"

#define GET_PROPERTY(comp, prop) \
    if (compData.contains(#prop)) comp.prop = compData[#prop].get<decltype(comp.prop)>()

namespace ast {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Color, r, g, b, a)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Vector2, x, y)

}  // namespace ast

namespace astd::components {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Animation, frameCount, frameStartIndex,
                                                frameDuration, loop, playing)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Transform, position, rotation, scale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Velocity, x, y)

using nlohmann::json;

static Sprite loadSprite(const std::string& prefabName, const json& compData) {
    if (compData.contains("fileName")) {
        if (compData.contains("frameCountX")) {
            // Sprite sheet
            Sprite sprite(compData["fileName"], compData["frameCountX"], compData["frameCountY"]);
            sprite.frameIndex = compData.value("frameIndex", 0);
            return sprite;
        } else {
            Sprite sprite(compData["fileName"].get<std::string>());
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
            } else if (compName == "Velocity") {
                registry.insert(prefabName, compData.get<Velocity>());
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
    registry.copyAll<Animation, Sprite, Transform, Velocity>(entity, prefab);
    return entity;
}

}  // namespace astd::components