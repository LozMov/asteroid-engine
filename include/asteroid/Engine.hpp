#pragma once

#include <filesystem>
#include <string>

#include "Color.hpp"
#include "Scene.hpp"
#include "Timer.hpp"

class SDL_Window;
class SDL_Renderer;

namespace ast {

class Engine final {
public:
    Engine(const std::string& title, int width, int height, const std::filesystem::path& assetsDir);
    ~Engine();

    // bool init();
    void run();
    void quit();

    void handleEvents();
    void update(float dt);
    void clear(const Color& color);
    void present();
    // void destroy();

    std::filesystem::path assetsDirectory_;
    std::string title_;
    Timer timer_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    bool running_;
};

}  // namespace ast