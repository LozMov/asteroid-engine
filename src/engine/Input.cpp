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
    SDL_MouseButtonFlags state = SDL_GetMouseState(nullptr, nullptr);
    return state & static_cast<SDL_MouseButtonFlags>(b);
}

Vector2 Input::getMousePosition() {
    float x, y;
    SDL_GetMouseState(&x, &y);
    return Vector2(x, y);
}

}  // namespace ast