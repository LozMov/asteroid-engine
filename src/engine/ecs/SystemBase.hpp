#pragma once

#include <algorithm>
#include <bitset>
#include <vector>

#include "Entity.hpp"

namespace ast {

class Registry;

using Signature = std::bitset<64>;

class SystemBase {
public:
    using TypeId = unsigned;

    SystemBase(Registry& registry) : registry_(registry) {}
    virtual ~SystemBase() = default;

    template <typename T>
    static TypeId getTypeId() {
        static TypeId typeId = s_typeId++;
        return typeId;
    }

    virtual void update(float dt) = 0;
    Signature getSignature() const { return signature_; };

    // Called when the system is added to the registry
    virtual void onAttached() {}
    // Called when the system is removed from the registry
    virtual void onDetached() {}
    // Called when an entity is added to the system
    virtual void onEntityAdded(Entity entity) {}
    // Called when an entity is removed from the system
    virtual void onEntityRemoved(Entity entity) {}
    // Called when an optional component is added to an entity
    virtual void onOptionalComponentAdded(Entity entity) {}
    // Called when an optional component is removed from an entity
    virtual void onOptionalComponentRemoved(Entity entity) {}

    void addEntity(Entity entity) {
        entities_.push_back(entity);
        onEntityAdded(entity);
    }

    void removeEntity(Entity entity) {
        auto it = std::find(entities_.begin(), entities_.end(), entity);
        if (it != entities_.end()) {
            std::iter_swap(it, entities_.end() - 1);
            entities_.pop_back();
            onEntityRemoved(entity);
        }
    }

    std::vector<Entity>& getEntities() { return entities_; }

    const std::vector<Entity>& getEntities() const { return entities_; }

    Registry& getRegistry() const { return registry_; }

protected:
    std::vector<Entity> entities_;
    Registry& registry_;
    Signature signature_;

private:
    inline static TypeId s_typeId = 0;
};

}  // namespace ast