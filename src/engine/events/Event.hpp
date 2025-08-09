#pragma once

namespace ast::events {

struct Event {};

struct WindowResizeEvent : Event {
    WindowResizeEvent(int width, int height) : width(width), height(height) {}

    int width;
    int height;
};

struct KeyEvent : Event {
    KeyEvent(int scancode, bool repeat) : scancode(scancode), repeat(repeat) {}

    int scancode;
    bool repeat;
};

struct MouseButtonEvent : Event {
    MouseButtonEvent(int button, int x, int y) : button(button), x(x), y(y) {}

    int button;
    int x;
    int y;
};

}  // namespace ast::events