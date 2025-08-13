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
    enum class ScalingMode { DEFAULT, TILED, NINE_GRID };
    enum class FlipMode { NONE, HORIZONTAL, VERTICAL, BOTH };

    explicit Sprite(const ast::Color& color = ast::Color::RED)
        : texture(ast::Cache::getDummyTexture(color)) {}

    explicit Sprite(const ast::Texture& texture, const ast::Vector2& size = ast::Vector2())
        : texture(texture), size(size) {
        if (size.x <= 0.0f || size.y <= 0.0f) {
            this->size = texture.size;
        }
    }

    explicit Sprite(const std::string& fileName, const ast::Vector2& size = ast::Vector2())
        : Sprite(ast::Cache::getTexture(fileName), size) {}

    // Sprite sheet constructor
    Sprite(const ast::Texture& texture, int frameCountX, int frameCountY,
           const ast::Vector2& size = ast::Vector2())
        : texture(texture), size(size) {
        calculateFrames(frameCountX, frameCountY);
    }

    Sprite(const std::string& fileName, int frameCountX, int frameCountY,
           const ast::Vector2& size = ast::Vector2())
        : Sprite(ast::Cache::getTexture(fileName), frameCountX, frameCountY, size) {}

    // 9-Grid Constructor
    Sprite(const std::string& fileName, float leftWidth, float rightWidth, float topHeight,
           float bottomHeight, const ast::Vector2& size = ast::Vector2())
        : Sprite(ast::Cache::getTexture(fileName), size) {
        scalingMode = ScalingMode::NINE_GRID;
        frames.push_back({leftWidth, rightWidth, topHeight, bottomHeight});
    }

    void calculateFrames(int frameCountX, int frameCountY,
                         const ast::Vector2& size = ast::Vector2()) {
        frames.clear();
        frameCount = frameCountX * frameCountY;
        frames.reserve(frameCount);
        float frameWidth = texture.size.x / frameCountX;
        float frameHeight = texture.size.y / frameCountY;
        for (int y = 0; y < frameCountY; ++y) {
            for (int x = 0; x < frameCountX; ++x) {
                frames.push_back({x * frameWidth, y * frameHeight, frameWidth, frameHeight});
            }
        }
        if (size.x <= 0.0f || size.y <= 0.0f) {
            this->size = {frameWidth, frameHeight};
        } else {
            this->size = size;
        }
        isFrame = true;
        this->frameCountX = frameCountX;
        this->frameCountY = frameCountY;
    }

    std::vector<SDL_FRect> frames;  // Set by constructor
    ast::Texture texture;           // Set by constructor
    ast::Vector2 size;
    ast::Vector2 origin;  // Normalized coordinates
    // ast::Color color;
    float parallaxFactor = 1.0f;  // 0 = static, 1 = moves with camera
    int zIndex = 0;

    int frameCountX = 1;  // Set by constructor
    int frameCountY = 1;  // Set by constructor
    int frameCount = 1;   // Set by constructor
    int frameIndex = 0;

    ScalingMode scalingMode = ScalingMode::DEFAULT;
    FlipMode flipMode = FlipMode::NONE;
    bool visible = true;
    bool isBackground = false;
    bool isFrame = false;  // Set by constructor
};

}  // namespace astd::components