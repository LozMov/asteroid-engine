#pragma once

#include "Vector2.hpp"

namespace ast {

class Input {
public:
    enum class Scancode {
        Unknown = 0,
        A = 4,
        B = 5,
        C = 6,
        D = 7,
        E = 8,
        F = 9,
        G = 10,
        H = 11,
        I = 12,
        J = 13,
        K = 14,
        L = 15,
        M = 16,
        N = 17,
        O = 18,
        P = 19,
        Q = 20,
        R = 21,
        S = 22,
        T = 23,
        U = 24,
        V = 25,
        W = 26,
        X = 27,
        Y = 28,
        Z = 29,

        RETURN = 40,
        ESCAPE = 41,
        BACKSPACE = 42,
        TAB = 43,
        SPACE = 44,

        F1 = 58,
        F2 = 59,
        F3 = 60,
        F4 = 61,
        F5 = 62,
        F6 = 63,
        F7 = 64,
        F8 = 65,
        F9 = 66,
        F10 = 67,
        F11 = 68,
        F12 = 69,

        RIGHT = 79,
        LEFT = 80,
        DOWN = 81,
        UP = 82,
        LCTRL = 224,
        LSHIFT = 225,
        LALT = 226,
        LGUI = 227,
        RCTRL = 228,
        RSHIFT = 229,
        RALT = 230,
        RGUI = 231
    };

    enum class Keycode {};

    enum class MouseButton {
        LEFT = 1,
        MIDDLE = 2,
        RIGHT = 4,
        X1 = 8,
        X2 = 16
    };

    struct KeyEvent {
        Scancode scancode;
        bool repeat;
        bool isKeyDown;
    };

    struct MouseButtonEvent {
        MouseButton button;
        Vector2 position;
    };

    static bool hasKeyboard();
    static bool isKeyPressed(Scancode c);
    static bool isKeyPressed(Keycode c);

    static bool hasMouse();
    static bool isMouseButtonPressed(MouseButton b);
    static Vector2 getMousePosition();

private:
    Input();
};

}  // namespace ast