#include "RenderSystem.hpp"

#include "engine/Color.hpp"
#include "engine/ecs/Registry.hpp"

namespace astd::systems {

using namespace components;

RenderSystem::RenderSystem(ast::Registry& registry)
    : System(registry),
      renderer_(registry.context.renderer),
      screenWidth_(registry.context.screenWidth),
      screenHeight_(registry.context.screenHeight) {}

void RenderSystem::update(float dt) {
    for (auto entity : entities_) {
        auto* sprite = registry_.get<Sprite>(entity);
        if (!sprite->visible) {
            continue;
        }
        auto* transform = registry_.get<Transform>(entity);
        ast::Vector2 position = transform->position;
        float logicalSizeX = sprite->size.x * transform->scale.x;
        float logicalSizeY = sprite->size.y * transform->scale.y;
        ast::Vector2 origin = sprite->origin;
        SDL_FRect* srcRectPtr = sprite->isFrame ? &sprite->frames[sprite->frameIndex] : nullptr;

        // Adjust position by origin
        float drawX = position.x - origin.x * logicalSizeX;
        float drawY = position.y - origin.y * logicalSizeY;

        // Basic culling - skip rendering if completely off-screen
        if (drawX + logicalSizeX < 0 || drawX > screenWidth_ || drawY + logicalSizeY < 0 ||
            drawY > screenHeight_) {
            continue;
        }

        SDL_FRect dstRect = {drawX, drawY, logicalSizeX, logicalSizeY};

        switch (sprite->scalingMode) {
            case Sprite::ScalingMode::DEFAULT:
                if (transform->rotation == 0.0f && sprite->flipMode == Sprite::FlipMode::NONE) {
                    SDL_RenderTexture(renderer_, sprite->texture.handle, srcRectPtr, &dstRect);
                } else {
                    SDL_FPoint center = {0.5f * logicalSizeX, 0.5f * logicalSizeY};
                    SDL_RenderTextureRotated(renderer_, sprite->texture.handle, srcRectPtr,
                                             &dstRect, transform->rotation, &center,
                                             static_cast<SDL_FlipMode>(sprite->flipMode));
                }
                break;
            case Sprite::ScalingMode::TILED:
                SDL_RenderTextureTiled(renderer_, sprite->texture.handle, srcRectPtr,
                                       transform->scale.x, &dstRect);
                break;
            case Sprite::ScalingMode::NINE_GRID:
                auto [leftWidth, rightWidth, topHeight, bottomHeight] = sprite->frames[0];
                SDL_RenderTexture9Grid(renderer_, sprite->texture.handle, srcRectPtr, leftWidth,
                                       rightWidth, topHeight, bottomHeight, transform->scale.x,
                                       &dstRect);
                break;
        }

        SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
        SDL_RenderRect(renderer_, &dstRect);
    }
}

void RenderSystem::onEntityAdded(ast::Entity entity) {
    std::sort(entities_.begin(), entities_.end(), [this](ast::Entity a, ast::Entity b) {
        auto* spriteA = registry_.get<Sprite>(a);
        auto* spriteB = registry_.get<Sprite>(b);
        return spriteA->zIndex < spriteB->zIndex;
    });
}

}  // namespace astd::systems