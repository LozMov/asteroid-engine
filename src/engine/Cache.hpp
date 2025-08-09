#pragma once

#include <string>
#include <unordered_map>

#include "Texture.hpp"

class SDL_Renderer;

namespace ast {

class Cache {
public:
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    static Cache& getInstance();
    static const Texture& getTexture(const std::string& fileName);
    static void setAssetsDirectory(const std::string& directory);
    static void setRenderer(SDL_Renderer* renderer);
    static void clear();

    inline static Texture MISSING_TEXTURE{};

private:
    Cache() = default;

    const Texture& getTextureImpl(const std::string& fileName);

    std::unordered_map<std::string, Texture> textures_;
    std::string assetsDirectory_;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace ast