#pragma once

#include <cmath>

namespace ast {

struct Vector2 {
    constexpr Vector2() = default;
    constexpr Vector2(float x, float y) : x(x), y(y) {}

    constexpr Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }
    
    constexpr Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }
    
    constexpr Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
    
    constexpr Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }
    
    constexpr Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    constexpr Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    constexpr Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    constexpr Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
    
    constexpr bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }
    
    constexpr bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }
    
    constexpr float magnitude() const {
        return std::sqrt(x * x + y * y);
    }
    
    constexpr float magnitudeSquared() const {
        return x * x + y * y;
    }
    
    constexpr Vector2 normalized() const {
        float mag = magnitude();
        if (mag > 0.0f) {
            return Vector2(x / mag, y / mag);
        }
        return Vector2(0.0f, 0.0f);
    }
    
    constexpr void normalize() {
        float mag = magnitude();
        if (mag > 0.0f) {
            x /= mag;
            y /= mag;
        }
    }
    
    constexpr float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }
    
    constexpr float distance(const Vector2& other) const {
        return (*this - other).magnitude();
    }
    
    constexpr float distanceSquared(const Vector2& other) const {
        return (*this - other).magnitudeSquared();
    }

    float x = 0.0f;
    float y = 0.0f;
};

constexpr Vector2 operator*(float scalar, const Vector2& vector) {
    return vector * scalar;
}

} // namespace ast