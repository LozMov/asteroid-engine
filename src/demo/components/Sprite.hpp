#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL_rect.h>

#include "engine/Cache.hpp"
#include "engine/Color.hpp"
#include "engine/Texture.hpp"
#include "engine/Vector2.hpp"
#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Sprite : ast::Component {
    enum class ScalingMode { NONE, CENTER, STRETCH, FIT, FILL, TILED };
    enum class FlipMode { NONE, HORIZONTAL, VERTICAL, BOTH };

    explicit Sprite(const ast::Color& color = ast::Color::MAGENTA)
        : texture(ast::Cache::MISSING_TEXTURE), color(color), hasTexture(false) {}

    explicit Sprite(const ast::Texture& texture, const ast::Vector2& logicalSize = ast::Vector2())
        : texture(texture), logicalSize(logicalSize) {
        if (logicalSize.x <= 0.0f || logicalSize.y <= 0.0f) {
            this->logicalSize = texture.size;
        }
    }

    explicit Sprite(const std::string& fileName, const ast::Vector2& logicalSize = ast::Vector2())
        : Sprite(ast::Cache::getTexture(fileName), logicalSize) {}

    // Sprite sheet constructor
    Sprite(const ast::Texture& texture, int frameCountX, int frameCountY,
           const ast::Vector2& logicalSize = ast::Vector2())
        : texture(texture),
          logicalSize(logicalSize),
          frameCountX(frameCountX),
          frameCountY(frameCountY),
          isFrame(true) {
        frameCount = frameCountX * frameCountY;
        frames.reserve(frameCount);
        float frameWidth = texture.size.x / frameCountX;
        float frameHeight = texture.size.y / frameCountY;
        for (int y = 0; y < frameCountY; ++y) {
            for (int x = 0; x < frameCountX; ++x) {
                frames.push_back({x * frameWidth, y * frameHeight, frameWidth, frameHeight});
            }
        }
        if (logicalSize.x <= 0.0f || logicalSize.y <= 0.0f) {
            this->logicalSize = {frameWidth, frameHeight};
        }
    }

    Sprite(const std::string& fileName, int frameCountX, int frameCountY,
           const ast::Vector2& logicalSize = ast::Vector2())
        : Sprite(ast::Cache::getTexture(fileName), frameCountX, frameCountY, logicalSize) {}

    std::vector<SDL_FRect> frames;  // Set by constructor
    ast::Texture texture;           // Set by constructor
    ast::Vector2 logicalSize;
    ast::Vector2 origin;  // Normalized coordinates
    ast::Color color;
    int zIndex = 0;

    int frameCountX = 1;  // Set by constructor
    int frameCountY = 1;  // Set by constructor
    int frameCount = 1;   // Set by constructor
    int frameIndex = 0;

    ScalingMode scalingMode = ScalingMode::NONE;
    FlipMode flipMode = FlipMode::NONE;
    bool visible = true;
    bool hasTexture = true;  // Set by constructor
    bool isFrame = false;    // Set by constructor
};

}  // namespace astd::components