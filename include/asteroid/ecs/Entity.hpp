#pragma once

#include <concepts>
#include <cstdint>
#include <limits>

namespace ast {

template <typename T>
concept EntityTraits =
    std::unsigned_integral<typename T::Type> && std::unsigned_integral<decltype(T::VERSION_BITS)> &&
    (T::VERSION_BITS > 0 && T::VERSION_BITS < std::numeric_limits<typename T::Type>::digits);

template <EntityTraits T>
struct EntityInfo {
    using Type = typename T::Type;
    static constexpr Type VERSION_BITS = T::VERSION_BITS;
    static constexpr Type INDEX_BITS = std::numeric_limits<Type>::digits - VERSION_BITS;
    static constexpr Type VERSION_MASK = (Type{1} << VERSION_BITS) - 1;
    static constexpr Type INDEX_MASK = (Type{1} << INDEX_BITS) - 1;
    static constexpr Type MAX_INDEX = INDEX_MASK;
    static constexpr Type MAX_VERSION = VERSION_MASK;

    static constexpr Type makeEntity(Type index, Type version) noexcept {
        return ((version & VERSION_MASK) << INDEX_BITS) | (index & INDEX_MASK);
    }

    static constexpr Type index(Type entity) noexcept { return entity & INDEX_MASK; }

    static constexpr Type version(Type entity) noexcept { return entity >> INDEX_BITS; }
};

struct Entity32 {
    using Type = uint32_t;
    static constexpr Type VERSION_BITS = 12;
};

struct Entity64 {
    using Type = uint64_t;
    static constexpr Type VERSION_BITS = 32;
};

static_assert(EntityTraits<Entity32> && EntityTraits<Entity64>);

using Entity = Entity32::Type;

inline constexpr Entity NULL_ENTITY = 0;

}  // namespace ast