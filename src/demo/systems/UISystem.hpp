#pragma once

#include <unordered_map>

#include "engine/Color.hpp"
#include "engine/ecs/System.hpp"
#include "../components/UI.hpp"

class SDL_Renderer;
class SDL_Texture;
class TTF_Font;
class TTF_Text;
class TTF_TextEngine;

namespace astd::systems {

class UISystem : public ast::System<components::UI> {
public:
    UISystem(ast::Registry& registry);
    ~UISystem();
    void update(float dt) override;

private:
    // void renderText(const std::string& text, const ast::Color& color, float x, float y);
    void renderIcon(SDL_Texture* icon, float x, float y, float width, float height);
    void renderIndicator(float value, float x, float y, float width, float height);

    SDL_Renderer* renderer_;
    TTF_TextEngine* textEngine_;
    TTF_Font* font_;
    std::unordered_map<ast::Entity, TTF_Text*> textMap_;
    ast::Entity scoreUIEntity_;
    ast::Entity highScoreUIEntity_;
    ast::Entity healthUIEntity_;
    ast::Entity gameStartTextEntity_;
    ast::Entity gameOverTextEntity_;
    ast::Entity speedUIEntity_;
    ast::Entity instructionsUIEntity_;
};

}  // namespace astd::systems