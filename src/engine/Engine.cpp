#include "Engine.hpp"

#include <SDL3_ttf/SDL_ttf.h>

#include "Audio.hpp"
#include "Cache.hpp"
#include "events/EventBus.hpp"

namespace ast {

Engine::Engine(const std::string& title, int width, int height, const std::string& assetsDir)
    : window_(nullptr),
      renderer_(nullptr),
      width_(width),
      height_(height),
      title_(title),
      running_(false),
      assetsDirectory_(assetsDir),
      timer_(60) {}

ast::Engine::~Engine() { destroy(); }

bool ast::Engine::init() {
    spdlog::set_pattern("%^[%L][%T] %s:%#: %v%$");
    spdlog::set_level(spdlog::level::trace);
    AST_INFO("Initializing SDL...");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ERROR();
        return false;
    }

    AST_INFO("Initializing TTF...");
    if (!TTF_Init()) {
        SDL_ERROR();
        SDL_Quit();
        return false;
    }

    Audio::getInstance().init();
    Audio::getInstance().setAssetsDirectory(assetsDirectory_ + "/sounds");

    AST_INFO("Creating window...");
    window_ = SDL_CreateWindow(title_.c_str(), width_, height_, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        SDL_ERROR();
        SDL_Quit();
        return false;
    }

    AST_INFO("Creating renderer...");
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        SDL_ERROR();
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }
    // SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);
    SDL_SetRenderLogicalPresentation(renderer_, width_, height_,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    running_ = true;
    AST_INFO("Initialization complete!");

    return true;
}

void ast::Engine::run() {
    while (running_) {
        timer_.startFrame();
        clear(ast::Color::BLACK);
        handleEvents();
        update(timer_.getDeltaTime());
        present();
        Audio::getInstance().update();
        timer_.endFrame();
    }
}

void ast::Engine::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit();
                break;
            case SDL_EVENT_KEY_DOWN:
                EventBus::publish<events::KeyEvent>(event.key.key, event.key.repeat);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                width_ = event.window.data1;
                height_ = event.window.data2;
                EventBus::publish<events::WindowResizeEvent>(width_, height_);
                break;
        }
    }
}


void ast::Engine::quit() { running_ = false; }

void ast::Engine::clear(const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer_);
}

void ast::Engine::present() {
    // Present the frame to the screen
    if (!SDL_RenderPresent(renderer_)) {
        SDL_ERROR();
    }
}

void ast::Engine::destroy() {
    Cache::clear();

    TTF_Quit();

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    Audio::getInstance().shutdown();
    SDL_Quit();
}

}  // namespace ast