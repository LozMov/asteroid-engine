#pragma once

#include <cstdint>

namespace ast {

struct Color {
    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    constexpr Color(uint32_t color)
        : r((color >> 24) & 0xFF),
          g((color >> 16) & 0xFF),
          b((color >> 8) & 0xFF),
          a(color & 0xFF) {}

    constexpr operator uint32_t() const { return r << 24 | g << 16 | b << 8 | a; }

    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color YELLOW;
    static const Color MAGENTA;
    static const Color CYAN;
    static const Color GRAY;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

inline const Color Color::BLACK{0, 0, 0};
inline const Color Color::WHITE{255, 255, 255};
inline const Color Color::RED{255, 0, 0};
inline const Color Color::GREEN{0, 255, 0};
inline const Color Color::BLUE{0, 0, 255};
inline const Color Color::YELLOW{255, 255, 0};
inline const Color Color::MAGENTA{255, 0, 255};
inline const Color Color::CYAN{0, 255, 255};
inline const Color Color::GRAY{128, 128, 128};

}  // namespace ast