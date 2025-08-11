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
        auto* transform = registry_.get<Transform>(entity);
        ast::Vector2 position = transform->position;
        float logicalSizeX = sprite->logicalSize.x * transform->scale.x;
        float logicalSizeY = sprite->logicalSize.y * transform->scale.y;
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

        if (sprite->hasTexture) {
            switch (sprite->scalingMode) {
                case Sprite::ScalingMode::TILED:
                    SDL_RenderTextureTiled(renderer_, sprite->texture.handle, srcRectPtr,
                                           transform->scale.x, &dstRect);
                    break;
                default:
                    if (transform->rotation == 0.0f && sprite->flipMode == Sprite::FlipMode::NONE) {
                        SDL_RenderTexture(renderer_, sprite->texture.handle, srcRectPtr, &dstRect);
                    } else {
                        SDL_FPoint center = {origin.x * logicalSizeX, origin.y * logicalSizeY};
                        SDL_RenderTextureRotated(renderer_, sprite->texture.handle, srcRectPtr,
                                                 &dstRect, transform->rotation, &center,
                                                 static_cast<SDL_FlipMode>(sprite->flipMode));
                    }
                    break;
            }
        } else {
            auto color = sprite->color;
            SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer_, &dstRect);
        }
    }
}

}  // namespace astd::systems