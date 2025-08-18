#pragma once

#include <string>
#include <vector>

namespace ast {
class Registry;
}

namespace astd {

// Prefab can be used to define a template for creating entities
// with predefined components and data.
class Prefab {
public:
    static std::vector<std::string> loadPrefabsFromFile(ast::Registry& registry,
                                                        const std::string& fileName);
    static ast::Entity createEntityFromPrefab(ast::Registry& registry,
                                              const std::string& prefabName);

    struct EntityData {
        std::string prefab;
        ast::Entity entity;
    };

    static std::vector<EntityData> loadLevel(ast::Registry& registry, const std::string& fileName);
};

}  // namespace astd