#pragma once

#include <string>

#include "Color.hpp"
#include "Timer.hpp"

class SDL_Window;
class SDL_Renderer;

namespace ast {

class Engine {
public:
    Engine(const std::string& title, int width, int height, const std::string& assetsDir);
    ~Engine();

    bool init();
    void run();
    void quit();

protected:
    void handleEvents();
    virtual void update(float dt) = 0;
    void clear(const Color& color);
    void present();
    void destroy();

    std::string assetsDirectory_;
    std::string title_;
    Timer timer_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    bool running_;
};

}  // namespace ast