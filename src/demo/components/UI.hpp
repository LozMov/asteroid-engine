#pragma once

#include <string>

#include "engine/Color.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"

class SDL_Texture;

namespace astd::components {

struct UI : public ast::Component {
    enum class Type { ICON, TEXT, INDICATOR };

    UI() = default;
    UI(const std::string& text, const ast::Color& color, const ast::Vector2& position)
        : type(Type::TEXT), text(text), color(color), position(position) {}
    UI(float value, const ast::Vector2& position, const ast::Vector2& size)
        : type(Type::INDICATOR), value(value), position(position), size(size) {}

    Type type = Type::ICON;
    ast::Vector2 position;
    ast::Vector2 size;
    ast::Color color{255, 255, 255, 255};
    std::string text;    // For TEXT
    float value = 0.0f;  // For INDICATOR (e.g., health, progress)
    bool visible = true;
};

}  // namespace astd::components