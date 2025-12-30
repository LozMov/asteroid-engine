#include "Engine.hpp"

#include <SDL3_ttf/SDL_ttf.h>

#include <filesystem>

#include "Audio.hpp"
#include "Cache.hpp"
#include "Input.hpp"
#include "events/Event.hpp"
#include "events/EventBus.hpp"

namespace ast {

Engine::Engine(const std::string& title, int width, int height, const std::filesystem::path& assetsDir)
    : window_(nullptr),
      renderer_(nullptr),
      width_(width),
      height_(height),
      title_(title),
      running_(false),
      assetsDirectory_(assetsDir),
      timer_(60) {
    spdlog::set_pattern("%^[%L][%T] %s:%#: %v%$");
    spdlog::set_level(spdlog::level::trace);
    AST_INFO("Initializing SDL...");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ERROR();
        return;
    }

    AST_INFO("Initializing TTF...");
    if (!TTF_Init()) {
        SDL_ERROR();
        SDL_Quit();
        return;
    }

    AST_INFO("Creating window...");
    window_ = SDL_CreateWindow(title_.c_str(), width_, height_, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        SDL_ERROR();
        SDL_Quit();
        return;
    }

    AST_INFO("Creating renderer...");
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        SDL_ERROR();
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return;
    }
    // SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);
    SDL_SetRenderLogicalPresentation(renderer_, width_, height_,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    AST_INFO("Assets Directory: {}", assetsDirectory_.string());
    // Initialize texture cache
    ast::Cache::init(renderer_);
    ast::Cache::setAssetsDirectory(assetsDirectory_ / "images");

    Audio::getInstance().init();
    Audio::getInstance().setAssetsDirectory(assetsDirectory_ / "sounds");

    running_ = true;
    AST_INFO("Initialization complete!");
}

Engine::~Engine() {
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

// bool Engine::init() {

// }

void Engine::run() {
    while (running_) {
        timer_.startFrame();
        clear(Color::WHITE);
        handleEvents();
        update(timer_.getDeltaTime());
        present();
        Audio::getInstance().update();
        timer_.endFrame();
    }
}

void Engine::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit();
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                EventBus::publish(Input::KeyEvent{static_cast<Input::Scancode>(event.key.scancode),
                                                  event.key.repeat, event.key.down});
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                width_ = event.window.data1;
                height_ = event.window.data2;
                EventBus::publish(events::WindowResizeEvent{width_, height_});
                break;
        }
    }
}

void Engine::update(float dt) {}

void Engine::quit() { running_ = false; }

void Engine::clear(const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer_);
}

void Engine::present() {
    // Present the frame to the screen
    if (!SDL_RenderPresent(renderer_)) {
        SDL_ERROR();
    }
}

// void Engine::destroy() {}

}  // namespace ast