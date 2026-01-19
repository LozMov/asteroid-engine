#pragma once

#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Component.hpp"
#include "Entity.hpp"
#include "SparseSet.hpp"
#include "SystemBase.hpp"

class SDL_Renderer;

namespace ast {

class Registry {
public:
    using IComponentPool = ISparseSet;

    template <typename T>
    using ComponentPool = SparseSet<T>;

    /// Get a component pointer (nullptr if entity doesn't have the component)
    template <typename T>
    T* get(Entity entity) const {
        auto* pool = getPool<T>();
        if (!pool) {
            return nullptr;
        }
        return const_cast<T*>(pool->get(entity));
    }

    /// Get a component reference (asserts if entity doesn't have the component)
    template <typename T>
    T& getUnchecked(Entity entity) {
        return getOrCreatePool<T>().getUnchecked(entity);
    }

    /// Get a system by type
    template <typename T>
    T* get() const {
        static_assert(std::is_base_of_v<SystemBase, T>, "T must be a System type");
        for (auto& system : systems_) {
            if (auto* systemPtr = dynamic_cast<T*>(system.get())) {
                return systemPtr;
            }
        }
        return nullptr;
    }

    /// Get a prefab entity by name
    Entity get(const std::string& name) const {
        if (auto it = prefabEntities_.find(name); it != prefabEntities_.end()) {
            return it->second;
        }
        return NULL_ENTITY;
    }

    /// Get the component pool for a type
    template <typename T>
    ComponentPool<T>* getPool() {
        auto typeId = Component::getTypeId<T>();
        if (typeId >= componentPools_.size() || !componentPools_[typeId]) {
            return nullptr;
        }
        return static_cast<ComponentPool<T>*>(componentPools_[typeId].get());
    }

    template <typename T>
    const ComponentPool<T>* getPool() const {
        auto typeId = Component::getTypeId<T>();
        if (typeId >= componentPools_.size() || !componentPools_[typeId]) {
            return nullptr;
        }
        return static_cast<const ComponentPool<T>*>(componentPools_[typeId].get());
    }

    /// Get all components of a type for iteration
    template <typename T>
    ComponentPool<T>& getAll() {
        return getOrCreatePool<T>();
    }

    template <typename T>
    const ComponentPool<T>& getAll() const {
        static ComponentPool<T> emptyPool;
        auto* pool = getPool<T>();
        return pool ? *pool : emptyPool;
    }

    // =========================================================================
    // Entity Management
    // =========================================================================

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

    void defer(std::function<void()>&& func) { deferredCommands_.push_back(std::move(func)); }

    /// Construct a component in-place for an entity
    template <typename T, typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        auto& pool = getOrCreatePool<T>();
        T& component = pool.emplace(entity, std::forward<Args>(args)...);
        defer([this, entity]() { onComponentAdded<T>(entity); });
        return component;
    }

    /// Construct a component for a prefab
    template <typename T, typename... Args>
    T& emplace(const std::string& prefabName, Args&&... args) {
        Entity prefab = prefabEntities_[prefabName];
        auto& pool = getOrCreatePool<T>();
        return pool.emplace(prefab, std::forward<Args>(args)...);
    }

    /// Insert an existing component for an entity
    template <typename T>
    T& insert(Entity entity, T component, bool notify = true) {
        auto& pool = getOrCreatePool<T>();
        T& comp = pool.insert(entity, std::move(component));
        if (notify) {
            defer([this, entity]() { onComponentAdded<T>(entity); });
        } else {
            entitySignatures_[entity].set(Component::getTypeId<T>());
        }
        return comp;
    }

    /// Insert a component for a prefab
    template <typename T>
    T& insert(const std::string& prefabName, T component) {
        Entity prefab = prefabEntities_[prefabName];
        auto& pool = getOrCreatePool<T>();
        return pool.insert(prefab, std::move(component));
    }

    /// Copy a component from one entity to another
    template <typename T>
    void copy(Entity entity, Entity other, bool notify = true) {
        if (T* otherComponent = get<T>(other)) {
            insert<T>(entity, *otherComponent, notify);
        }
    }

    /// Copy multiple components from one entity to another
    template <typename... Ts>
    void copyAll(Entity entity, Entity other, bool notify = true) {
        (copy<Ts>(entity, other, notify), ...);
    }

    /// Force an entity to be checked against all systems
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
        if (T* existingSystem = get<T>()) {
            return *existingSystem;
        }
        systems_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        T& system = static_cast<T&>(*systems_.back());
        for (const auto& [entity, signature] : entitySignatures_) {
            if ((signature & system.getSignature()) == system.getSignature()) {
                system.addEntity(entity);
            }
        }
        system.onAttached();
        return system;
    }

    /// Remove an entity and all its components
    void erase(Entity entity) {
        defer([this, entity]() {
            onComponentRemoved(entity);
            entitySignatures_.erase(entity);
        });
    }

    /// Remove a specific component from an entity
    template <typename T>
    void erase(Entity entity) {
        defer([this, entity]() { onComponentRemoved<T>(entity); });
    }

    /// Remove a system by type
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

    /// Remove all components from an entity
    void eraseComponents(Entity entity) {
        defer([this, entity]() { onComponentRemoved(entity); });
    }

    template <typename T>
    bool has(Entity entity) const {
        auto it = entitySignatures_.find(entity);
        if (it == entitySignatures_.end()) {
            return false;
        }
        return it->second.test(Component::getTypeId<T>());
    }

    template <typename... Ts>
    bool hasAll(Entity entity) const {
        return (has<Ts>(entity) && ...);
    }

    /// Iterate over all entities with a specific component
    template <typename T, typename Func>
    void each(Func&& func) {
        if (auto* pool = getPool<T>()) {
            pool->each(std::forward<Func>(func));
        }
    }

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
        // Execute deferred commands
        std::vector<std::function<void()>> processingCommands;
        std::swap(deferredCommands_, processingCommands);
        for (const auto& cmd : processingCommands) {
            cmd();
        }
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
        auto it = entitySignatures_.find(entity);
        if (it == entitySignatures_.end()) {
            return;
        }
        Signature oldSignature = it->second;
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
        if (auto* pool = getPool<T>()) {
            pool->erase(entity);
        }
    }

    void onComponentRemoved(Entity entity) {
        auto it = entitySignatures_.find(entity);
        if (it == entitySignatures_.end()) {
            return;
        }
        Signature oldSignature = it->second;
        entitySignatures_[entity].reset();

        for (auto& system : systems_) {
            if ((system->getSignature() & oldSignature) == system->getSignature()) {
                system->removeEntity(entity);
            }
        }
        for (auto& pool : componentPools_) {
            if (pool) {
                pool->erase(entity);
            }
        }
    }

    template <typename T>
    ComponentPool<T>& getOrCreatePool() {
        auto typeId = Component::getTypeId<T>();
        if (typeId >= componentPools_.size()) {
            componentPools_.resize(typeId + 1);
        }
        if (!componentPools_[typeId]) {
            componentPools_[typeId] = std::make_unique<ComponentPool<T>>();
        }
        return *static_cast<ComponentPool<T>*>(componentPools_[typeId].get());
    }

    std::vector<Entity> expiredEntities_;
    std::vector<std::function<void()>> deferredCommands_;
    std::unordered_map<std::string, Entity> prefabEntities_;
    std::unordered_map<Entity, Signature> entitySignatures_;
    std::vector<std::unique_ptr<IComponentPool>> componentPools_;
    std::vector<std::unique_ptr<SystemBase>> systems_;
    Entity nextEntityId_ = 1;  // NULL_ENTITY is 0
};

}  // namespace ast