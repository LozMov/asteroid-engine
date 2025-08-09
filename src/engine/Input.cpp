#include "Input.hpp"

namespace ast {

bool Input::hasKeyboard() { return SDL_HasKeyboard(); }

bool Input::isKeyPressed(Scancode c) {
    static const bool* state = SDL_GetKeyboardState(nullptr);
    return state[static_cast<int>(c)];
}

bool Input::isKeyPressed(Keycode c) {
    static const bool* state = SDL_GetKeyboardState(nullptr);
    return state[static_cast<int>(c)];
}

bool Input::hasMouse() { return SDL_HasMouse(); }

bool Input::isMouseButtonPressed(MouseButton b) {
    Uint32 state = SDL_GetMouseState(nullptr, nullptr);
    return state & static_cast<Uint32>(b);
}

Vector2 Input::getMousePosition() {
    float x, y;
    SDL_GetMouseState(&x, &y);
    return Vector2(x, y);
}

}  // namespace ast