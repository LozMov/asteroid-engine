#include "Cache.hpp"

#include <SDL3_image/SDL_image.h>

#include <filesystem>

namespace ast {

Cache& Cache::getInstance() {
    static Cache cache;
    return cache;
}

const Texture& Cache::getTexture(const std::string& fileName) {
    return getInstance().getTextureImpl(fileName);
}

void Cache::setAssetsDirectory(const std::string& directory) {
    getInstance().assetsDirectory_ = directory;
}

void Cache::setRenderer(SDL_Renderer* renderer) {
    clear();
    getInstance().renderer_ = renderer;
    MISSING_TEXTURE.handle =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
    if (!MISSING_TEXTURE.handle) {
        SDL_ERROR();
    } else {
        MISSING_TEXTURE.size = Vector2{1.0f, 1.0f};
        SDL_SetRenderTarget(renderer, MISSING_TEXTURE.handle);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, nullptr);
    }
}

void Cache::clear() {
    for (auto& pair : getInstance().textures_) {
        if (pair.second.handle) {
            SDL_DestroyTexture(pair.second.handle);
        }
    }
    getInstance().textures_.clear();
}

const Texture& Cache::getTextureImpl(const std::string& fileName) {
    // Check if already loaded
    auto it = textures_.find(fileName);
    if (it != textures_.end()) {
        return it->second;
    }
    std::filesystem::path imagePath = std::filesystem::path(assetsDirectory_) / fileName;
    SDL_Texture* handle = IMG_LoadTexture(renderer_, imagePath.string().c_str());
    if (!handle) {
        SDL_ERROR();
        return MISSING_TEXTURE;
    }
    Texture texture{.handle = handle};
    SDL_GetTextureSize(handle, &texture.size.x, &texture.size.y);
    return textures_[fileName] = texture;
}

}  // namespace ast