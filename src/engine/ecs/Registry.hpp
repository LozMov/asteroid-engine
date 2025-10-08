#pragma once

#include <array>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Component.hpp"
#include "Entity.hpp"
#include "SystemBase.hpp"

class SDL_Renderer;

namespace ast {

class Registry {
public:
    constexpr static unsigned MAX_ENTITIES = 2000;
    constexpr static unsigned MAX_COMPONENT_TYPES = 64;
    constexpr static unsigned MAX_SYSTEM_TYPES = 64;

    // Get a component
    template <typename T>
    T* get(Entity entity) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        // auto storage = componentStorages_.find(Component::getTypeId<T>());
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        auto it = storage.find(entity);
        if (it != storage.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    // Get a system
    template <typename T>
    T* get() {
        static_assert(std::is_base_of_v<SystemBase, T>, "T must be a System type");
        for (auto& system : systems_) {
            if (auto* systemPtr = dynamic_cast<T*>(system.get())) {
                return systemPtr;
            }
        }
        return nullptr;
    }

    // Get a prefab
    Entity get(const std::string& name) {
        auto it = prefabEntities_.find(name);
        if (it != prefabEntities_.end()) {
            return it->second;
        }
        return NULL_ENTITY;  // Return a null entity if not found
    }

    // Get all components of a type
    template <typename T>
    const auto& getAll() {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        return componentStorages_[Component::getTypeId<T>()];
    }

    Entity createEntity() {
        Entity entity = nextEntityId_++;
        entitySignatures_.emplace(entity, Signature{});
        return entity;
    }

    Entity createPrefab(const std::string& name) {
        Entity entity = nextEntityId_++;
        prefabEntities_[name] = entity;
        return entity;
    }

    // Construct a component
    template <typename T, typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        storage[entity] = std::make_unique<T>(std::forward<Args>(args)...);
        onComponentAdded<T>(entity);
        return static_cast<T&>(*storage[entity]);
    }

    // Construct a component for a prefab
    template <typename T, typename... Args>
    T& emplace(const std::string& prefabName, Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        Entity prefab = prefabEntities_[prefabName];
        storage[prefab] = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T&>(*storage[prefab]);
    }

    // Insert a component
    template <typename T>
    T& insert(Entity entity, T component, bool notify = true) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        storage[entity] = std::make_unique<T>(std::move(component));
        if (notify) {
            onComponentAdded<T>(entity);
        } else {
            // Update the entity signature without notifying systems
            entitySignatures_[entity].set(Component::getTypeId<T>());
        }
        return static_cast<T&>(*storage[entity]);
    }

    // Insert a component for a prefab
    template <typename T>
    T& insert(const std::string& prefabName, T component) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        Entity prefab = prefabEntities_[prefabName];
        storage[prefab] = std::make_unique<T>(std::move(component));
        return static_cast<T&>(*storage[prefab]);
    }

    template <typename T>
    void copy(Entity entity, Entity other, bool notify = true) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        // Copy construct the component from the other entity
        if (T* otherComponent = get<T>(other)) {
            insert<T>(entity, *otherComponent, notify);
        }
    }

    template <typename... Ts>
    void copyAll(Entity entity, Entity other, bool notify = true) {
        (copy<Ts>(entity, other, notify), ...);
    }

    // Add an entity to all systems that match its signature
    void forceCheck(Entity entity) {
        for (auto& system : systems_) {
            if ((system->getSignature() & entitySignatures_[entity]) == system->getSignature()) {
                system->addEntity(entity);
            }
        }
    }

    template <typename T, typename... Args>
    T& attach(Args&&... args) {
        static_assert(std::is_base_of_v<SystemBase, T>, "T must be a System type");
        // Check if we already have a system of this type
        if (T* existingSystem = get<T>()) {
            return *existingSystem;
        }
        // Create and add the new system
        systems_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        T& system = static_cast<T&>(*systems_.back());
        // Add the entities that match the system's signature
        for (const auto& [entity, signature] : entitySignatures_) {
            if ((signature & system.getSignature()) == system.getSignature()) {
                system.addEntity(entity);
            }
        }
        system.onAttached();
        return system;
    }

    void erase(Entity entity) {
        onComponentRemoved(entity);
        entitySignatures_.erase(entity);
        // Remove the components after notifying systems, so they can access
        // the components in onEntityRemoved()
        for (auto& storage : componentStorages_) {
            storage.erase(entity);
        }
    }

    template <typename T>
    void erase(Entity entity) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component type");
        auto& storage = componentStorages_[Component::getTypeId<T>()];
        onComponentRemoved<T>(entity);
        storage.erase(entity);
    }

    template <typename T>
    void erase() {
        static_assert(std::is_base_of_v<SystemBase, T>, "T must be a System type");
        auto it = std::find_if(systems_.begin(), systems_.end(),
                               [](const std::unique_ptr<SystemBase>& system) {
                                   return dynamic_cast<T*>(system.get()) != nullptr;
                               });
        if (it != systems_.end()) {
            systems_.erase(it);
        }
    }

    // Remove all components associated with the entity
    void eraseComponents(Entity entity) {
        onComponentRemoved(entity);
        // TODO
        for (auto& storage : componentStorages_) {
            storage.erase(entity);
        }
    }

    template <typename T>
    bool has(Entity entity) const {
        auto id = Component::getTypeId<T>();
        return entitySignatures_.at(entity).test(id);
    }

    // Checks if the entity has all specified component types by verifying presence in each
    // component storage
    template <typename... Ts>
    bool hasAll(Entity entity) const {
        return (has<Ts>(entity) && ...);
    }

    // Expose systems for inspection
    const std::vector<std::unique_ptr<SystemBase>>& getSystems() const { return systems_; }

    void markAsExpired(Entity entity) { expiredEntities_.push_back(entity); }

    // Update all systems
    void update(float dt) {
        for (auto& system : systems_) {
            system->update(dt);
        }
        // Clean up expired entities
        for (Entity entity : expiredEntities_) {
            erase(entity);
        }
        expiredEntities_.clear();
    }

    struct Context {
        enum class GameState { PLAYING, GAME_OVER, PAUSED, QUIT };
        GameState gameState = GameState::PLAYING;
        Entity playerEntity = NULL_ENTITY;
        float screenWidth = 800.0f;
        float screenHeight = 600.0f;
        SDL_Renderer* renderer;
    };

    Context context;

private:
    template <typename T>
    void onComponentAdded(Entity entity) {
        auto typeId = Component::getTypeId<T>();
        Signature oldSignature = entitySignatures_[entity];
        if (oldSignature[typeId]) {
            // Signature does not change, no need to update
            return;
        }
        entitySignatures_[entity].set(typeId);

        for (auto& system : systems_) {
            if ((system->getSignature() & oldSignature) != system->getSignature()) {
                // The entity did not match the system's signature before, but does now
                if ((system->getSignature() & entitySignatures_[entity]) ==
                    system->getSignature()) {
                    system->addEntity(entity);
                }
            } else {
                // The entity already matched the system's signature
                system->onOptionalComponentAdded(entity);
            }
        }
    }

    template <typename T>
    void onComponentRemoved(Entity entity) {
        auto typeId = Component::getTypeId<T>();
        Signature oldSignature = entitySignatures_[entity];
        if (!oldSignature[typeId]) {
            // Component does not exist, no need to update signature
            return;
        }
        entitySignatures_[entity].reset(typeId);

        for (auto& system : systems_) {
            if ((system->getSignature() & oldSignature) == system->getSignature()) {
                // The entity matched the system's signature before, but does not now
                if ((system->getSignature() & entitySignatures_[entity]) !=
                    system->getSignature()) {
                    system->removeEntity(entity);
                } else {
                    // The entity still matches the system's signature
                    system->onOptionalComponentRemoved(entity);
                }
            }
        }
    }

    void onComponentRemoved(Entity entity) {
        Signature oldSignature = entitySignatures_[entity];
        entitySignatures_[entity].reset();

        for (auto& system : systems_) {
            if ((system->getSignature() & oldSignature) == system->getSignature()) {
                system->removeEntity(entity);
            }
        }
    }

    std::vector<Entity> expiredEntities_;
    std::unordered_map<std::string, Entity> prefabEntities_;
    std::unordered_map<Entity, Signature> entitySignatures_;
    // std::unordered_map<Component::TypeId, std::unordered_map<Entity, std::unique_ptr<Component>>>
    //     componentStorages_;
    std::array<std::unordered_map<Entity, std::unique_ptr<Component>>, MAX_COMPONENT_TYPES>
        componentStorages_;
    std::vector<std::unique_ptr<SystemBase>> systems_;
    Entity nextEntityId_ = 1;  // NULL_ENTITY is 0
};

}  // namespace ast