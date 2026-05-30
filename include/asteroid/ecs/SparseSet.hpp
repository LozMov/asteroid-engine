#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

#include "Entity.hpp"

namespace ast {

/// Type-erased interface for SparseSet
template <EntityTraits ET>
class ISparseSet {
    using Entity = typename ET::Type;

public:
    virtual ~ISparseSet() = default;
    virtual bool contains(Entity) const = 0;
    virtual void erase(Entity) = 0;
    virtual std::size_t size() const = 0;
    virtual void clear() = 0;
};

/// A sparse set data structure for efficient entity-component storage.
template <EntityTraits ET, typename T>
class SparseSet : public ISparseSet<ET> {
    using Entity = typename ET::Type;
    using EInfo = EntityInfo<ET>;

public:

    static constexpr auto INVALID_INDEX = std::numeric_limits<Entity>::max();

    /// Check if the set contains an entity
    bool contains(Entity entity) const override {
        auto idx = EInfo::index(entity);
        if (idx >= sparse_.size() || sparse_[idx] == INVALID_INDEX) {
            return false;
        }
        return dense_[sparse_[idx]] == entity;
    }

    /// Get the number of elements in the set
    std::size_t size() const override { return dense_.size(); }

    /// Check if the set is empty
    bool empty() const { return dense_.empty(); }

    /// Add an entity with a component (in-place construction)
    template <typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        assert(!contains(entity) && "Entity already has this component");

        auto idx = EInfo::index(entity);
        if (idx >= sparse_.size()) {
            sparse_.resize(idx + 1, INVALID_INDEX);
        }
        sparse_[idx] = static_cast<Entity>(dense_.size());
        dense_.push_back(entity);
        return components_.emplace_back(std::forward<Args>(args)...);
    }

    /// Add an entity with an existing component (move/copy)
    T& insert(Entity entity, T component) {
        assert(!contains(entity) && "Entity already has this component");

        auto idx = EInfo::index(entity);
        if (idx >= sparse_.size()) {
            sparse_.resize(idx + 1, INVALID_INDEX);
        }
        sparse_[idx] = static_cast<Entity>(dense_.size());
        dense_.push_back(entity);
        components_.push_back(std::move(component));
        return components_.back();
    }

    /// Remove an entity from the set
    void erase(Entity entity) override {
        if (!contains(entity)) {
            return;
        }

        auto idx = EInfo::index(entity);
        Entity denseIdx = sparse_[idx];
        // Swap-and-pop
        Entity lastEntity = dense_.back();
        dense_[denseIdx] = lastEntity;
        components_[denseIdx] = std::move(components_.back());
        sparse_[EInfo::index(lastEntity)] = denseIdx;
        dense_.pop_back();
        components_.pop_back();
        sparse_[idx] = INVALID_INDEX;
    }

    /// Get a pointer to the component for an entity (nullptr if not found)
    T* get(Entity entity) {
        if (!contains(entity)) {
            return nullptr;
        }
        return &components_[sparse_[EInfo::index(entity)]];
    }

    /// Get a const pointer to the component for an entity (nullptr if not found)
    const T* get(Entity entity) const {
        if (!contains(entity)) {
            return nullptr;
        }
        return &components_[sparse_[EInfo::index(entity)]];
    }

    /// Get a reference to the component (assumes entity exists)
    T& getUnchecked(Entity entity) {
        assert(contains(entity) && "Entity does not have this component");
        return components_[sparse_[EInfo::index(entity)]];
    }

    const T& getUnchecked(Entity entity) const {
        assert(contains(entity) && "Entity does not have this component");
        return components_[sparse_[EInfo::index(entity)]];
    }

    /// Clear all entities and components
    void clear() override {
        sparse_.clear();
        dense_.clear();
        components_.clear();
    }

    // Iterators for range-based for loops over components
    auto begin() { return components_.begin(); }
    auto end() { return components_.end(); }
    auto begin() const { return components_.begin(); }
    auto end() const { return components_.end(); }

    /// Get the dense array of entities
    const std::vector<Entity>& entities() const { return dense_; }

    /// Get the dense array of components
    std::vector<T>& components() { return components_; }
    const std::vector<T>& components() const { return components_; }

    /// Iterate over all entity-component pairs
    template <typename Func>
    void each(Func&& func) {
        for (std::size_t i = 0; i < dense_.size(); ++i) {
            func(dense_[i], components_[i]);
        }
    }

    template <typename Func>
    void each(Func&& func) const {
        for (std::size_t i = 0; i < dense_.size(); ++i) {
            func(dense_[i], components_[i]);
        }
    }

private:
    std::vector<Entity> sparse_;  // Entity -> index in dense array
    std::vector<Entity> dense_;   // Index -> Entity
    std::vector<T> components_;   // Index -> Component
};

}  // namespace ast
