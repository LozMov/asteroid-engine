#pragma once

#include <SDL3/SDL_rect.h>

#include "Vector2.hpp"

namespace ast {

struct Rect {
    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    constexpr Rect(const Vector2& position, const Vector2& size)
        : x(position.x), y(position.y), w(size.x), h(size.y) {}
    constexpr Rect(const SDL_FRect& rect) : x(rect.x), y(rect.y), w(rect.w), h(rect.h) {}
    constexpr Rect(const SDL_Rect& rect)
        : x(static_cast<float>(rect.x)),
          y(static_cast<float>(rect.y)),
          w(static_cast<float>(rect.w)),
          h(static_cast<float>(rect.h)) {}

    constexpr Rect& operator=(const SDL_FRect& rect) {
        x = rect.x;
        y = rect.y;
        w = rect.w;
        h = rect.h;
        return *this;
    }

    constexpr bool operator==(const Rect& rhs) const {
        return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
    }

    operator SDL_FRect() const { return SDL_FRect{x, y, w, h}; }

    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

}  // namespace ast