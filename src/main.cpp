#include <memory>

#include "engine/Engine.hpp"

int main() {
    auto engine = std::make_unique<ast::Engine>("New Game", 1000, 800, "assets");
    engine->run();
    return 0;
}