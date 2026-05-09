#pragma once

#include "ecs/Registry.hpp"

namespace ast {

class Engine;

class Scene {
public:
    explicit Scene(Engine* engine);
    virtual ~Scene() = default;

    virtual void update(float dt) = 0;
    virtual void render() = 0;

    Engine* engine_;
    Registry registry_;
};

}