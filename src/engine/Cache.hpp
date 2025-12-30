#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "Color.hpp"
#include "Texture.hpp"

class SDL_Renderer;

namespace ast {
/**
 * Cache class for managing textures and dummy textures.
 * This class is a singleton that provides access to textures by file name
 * and allows for the creation of dummy textures based on color.
 */
class Cache {
public:
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    static Cache& getInstance();
    static void init(SDL_Renderer* renderer);
    static const Texture& getTexture(const std::string& fileName);
    static Texture getDummyTexture(const ast::Color& color);
    static void setAssetsDirectory(const std::filesystem::path& directory);
    static void clear();

    inline static Texture MISSING_TEXTURE{};

private:
    Cache() = default;

    const Texture& getTextureImpl(const std::string& fileName);
    Texture getDummyTextureImpl(const ast::Color& color);

    std::unordered_map<std::string, Texture> textures_;
    std::filesystem::path assetsDirectory_;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace ast