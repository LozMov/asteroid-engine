#pragma once

#include "Vector2.hpp"

class SDL_Texture;

namespace ast {

struct Texture {
    SDL_Texture* handle = nullptr;
    Vector2 size;
};

}  // namespace ast