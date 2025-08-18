#include "UISystem.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <engine/ecs/Registry.hpp>

#include "../components/Sprite.hpp"

namespace astd::systems {

using namespace components;

UISystem::UISystem(ast::Registry& registry)
    : System(registry), renderer_(registry.context.renderer) {
    textEngine_ = TTF_CreateRendererTextEngine(renderer_);
    // Try multiple font paths
    const char* fontPaths[] = {"../assets/font.ttf", "C:/Windows/Fonts/arial.ttf",
                               "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                               "/System/Library/Fonts/Helvetica.ttc"};

    font_ = nullptr;
    for (const char* path : fontPaths) {
        font_ = TTF_OpenFont(path, 48);
        if (font_) break;
    }

    if (!font_) {
        AST_ERROR("Failed to load font!");
    }

    // Create score UI
    scoreUIEntity_ = registry_.createEntity();
    components::UI scoreUI;
    scoreUI.type = components::UI::Type::TEXT;
    scoreUI.text = "Score: 0";
    scoreUI.position = ast::Vector2(30, 30);
    scoreUI.color = ast::Color(128, 255, 128, 255);
    registry_.insert(scoreUIEntity_, scoreUI);

    highScoreUIEntity_ = registry_.createEntity();
    components::UI highScoreUI;
    highScoreUI.type = components::UI::Type::TEXT;
    highScoreUI.text = "High Score: 0";
    highScoreUI.position = ast::Vector2(30, 100);
    highScoreUI.color = ast::Color(255, 128, 128, 255);
    registry_.insert(highScoreUIEntity_, highScoreUI);

    // Create speed multiplier UI
    speedUIEntity_ = registry_.createEntity();
    components::UI speedUI;
    speedUI.type = components::UI::Type::TEXT;
    speedUI.text = "Speed: 1.0x";
    speedUI.position = ast::Vector2(30, 170);
    speedUI.color = ast::Color(128, 128, 255, 255);
    registry_.insert(speedUIEntity_, speedUI);

    // Create game start text
    gameStartTextEntity_ = registry_.createEntity();
    components::UI gameStartText;
    gameStartText.type = components::UI::Type::TEXT;
    gameStartText.text = "Press R to start the game!";
    gameStartText.position = ast::Vector2(150, registry_.context.screenHeight / 2);
    gameStartText.color = ast::Color(192, 100, 40, 255);
    registry_.insert(gameStartTextEntity_, gameStartText);

    // Create game over text
    gameOverTextEntity_ = registry_.createEntity();
    components::UI gameOverText;
    gameOverText.type = components::UI::Type::TEXT;
    gameOverText.text = "Game Over! Press F12 to restart";
    gameOverText.position = ast::Vector2(150, registry_.context.screenHeight / 2);
    gameOverText.color = ast::Color(255, 100, 100, 255);
    gameOverText.visible = false;
    registry_.insert(gameOverTextEntity_, gameOverText);

    // Create instructions text
    instructionsUIEntity_ = registry_.createEntity();
    components::UI instructionsUI;
    instructionsUI.type = components::UI::Type::TEXT;
    instructionsUI.text = "Use SPACE to jump!";
    instructionsUI.position = ast::Vector2(150, registry_.context.screenHeight / 2 + 80);
    instructionsUI.color = ast::Color(255, 100, 100, 255);
    registry_.insert(instructionsUIEntity_, instructionsUI);
}

UISystem::~UISystem() {
    for (const auto& [entity, text] : textMap_) {
        TTF_DestroyText(text);
    }
    TTF_DestroyRendererTextEngine(textEngine_);
}

void UISystem::renderIcon(SDL_Texture* icon, float x, float y, float width, float height) {
    SDL_FRect rect = {x, y, width, height};
    SDL_RenderTexture(renderer_, icon, nullptr, &rect);
}

void UISystem::renderIndicator(float value, float x, float y, float width, float height) {
    // Draw a simple bar (e.g., health/progress)
    SDL_FRect rect = {x, y, width, height};
    // bgColor
    SDL_SetRenderDrawColor(renderer_, 80, 80, 80, 200);
    SDL_RenderFillRect(renderer_, &rect);
    rect.w *= value;
    // fgColor
    SDL_SetRenderDrawColor(renderer_, 0, 200, 0, 255);
    SDL_RenderFillRect(renderer_, &rect);
}

void UISystem::update(float dt) {
    for (auto entity : entities_) {
        auto* ui = registry_.get<UI>(entity);
        if (!ui->visible) {
            continue;
        }
        float x = ui->position.x;
        float y = ui->position.y;
        Sprite* sprite;
        TTF_Text* text;
        switch (ui->type) {
            case UI::Type::TEXT:
                if (textMap_.find(entity) == textMap_.end()) {
                    text = TTF_CreateText(textEngine_, font_, ui->text.c_str(), 0);
                    TTF_SetTextColor(text, ui->color.r, ui->color.g, ui->color.b, ui->color.a);
                    textMap_[entity] = text;
                } else {
                    text = textMap_[entity];
                }
                TTF_DrawRendererText(text, x, y);
                break;
            case UI::Type::ICON:
                sprite = registry_.get<Sprite>(entity);
                if (!sprite) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                                 "Sprite component missing for entity %llu", entity);
                    break;
                }
                renderIcon(sprite->texture.handle, x, y, sprite->size.x, sprite->size.y);
                break;
            case UI::Type::INDICATOR:
                renderIndicator(ui->value, x, y, ui->size.x, ui->size.y);
                break;
        }
    }
}

}  // namespace astd::systems