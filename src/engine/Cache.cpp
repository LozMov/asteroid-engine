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

Texture Cache::getDummyTexture(const ast::Color& color) {
    return getInstance().getDummyTextureImpl(color);
}

void Cache::setAssetsDirectory(const std::filesystem::path& directory) {
    getInstance().assetsDirectory_ = directory;
}

void Cache::init(SDL_Renderer* renderer) {
    clear();
    getInstance().renderer_ = renderer;
    MISSING_TEXTURE = getInstance().getDummyTextureImpl(ast::Color::MAGENTA);
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
    std::filesystem::path imagePath = assetsDirectory_ / fileName;
    SDL_Texture* handle = IMG_LoadTexture(renderer_, imagePath.string().c_str());
    if (!handle) {
        SDL_ERROR();
        return MISSING_TEXTURE;
    }
    Texture texture{.handle = handle};
    SDL_GetTextureSize(handle, &texture.size.x, &texture.size.y);
    return textures_[fileName] = texture;
}

Texture Cache::getDummyTextureImpl(const ast::Color& color) {
    Texture dummyTexture {
        .handle = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1, 1),
        .size = Vector2{1.0f, 1.0f}
    };
    if (!dummyTexture.handle) {
        SDL_ERROR();
    } else {
        SDL_SetRenderTarget(renderer_, dummyTexture.handle);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer_);
        SDL_SetRenderTarget(renderer_, nullptr);
    }
    return dummyTexture;
}

}  // namespace ast