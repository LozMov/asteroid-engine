# Asteroid

## Overview

The Asteroid engine is designed to facilitate the development of 2D games. It includes modules for rendering, input handling, physics, and audio management. The engine is built with C++17 and uses SDL3 for graphics, audio, and input handling.

### ECS Architecture

The engine uses an Entity-Component-System (ECS) architecture to manage game objects and their components.

The core of the architecture is the `Registry` class. It provides a way to create, destroy, and query entities, components, and systems.

Entities are identified by a unique ID, and components are added to entities. Systems are registered with the registry and are called automatically when the registry is updated.

To add a new component, create a new class that inherits from `Component`. A component is a class that contains data about an entity.

To add a new system, create a new class that inherits from `System`. A system is a class that contains logic for updating an entity. Each system has a signature, which is a list of component types that the system requires.

The `System` class is a template class that helps with the registration of systems. It is a derived class of `SystemBase`, which is the base class for all systems.

```cpp
class MySystem : public System<Comp1, Comp2, Comp3> {
public:
    MySystem(Registry& registry) : System(registry) {}
    void update(float dt) override {
        // Your system logic here
    }
};
```

### Engine Features

- `Audio` for managing audio
- `Cache` for caching and managing textures
- `EventBus` for managing events
- `Input` for handling input
- `Log` for logging
- `Timer` for limiting the frame rate

To use the engine, create a new class that inherits from `Engine` and override the  `update` method, which is called in the main loop.

```cpp
class MyGame : public Engine {
public:
    void update(float dt) override {
        // Your game logic here
    }
};
```

## Building

### Requirements

   - C++17 compatible compiler (tested with GCC 15.1.0, MinGW-w64 on Windows 11)
   - CMake (tested with version 3.28)
   - SDL3
   - SDL3_image
   - SDL3_ttf
   - Box2D
   - spdlog (in the `external` directory)
   - nlohmann/json (fetched by CMake)

### Configure and Build the Project

   ```sh
   cmake -S . -B build
   cmake --build build
   ```

The executable will be located in the `bin/` directory.

## Credits

### Libraries
  - [SDL3](https://www.libsdl.org/) for graphics and input handling
  - [SDL3_image](https://github.com/libsdl-org/SDL_image) for image loading
  - [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) for font rendering
  - [Box2D](https://box2d.org/) for physics
  - [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing
  - [spdlog](https://github.com/gabime/spdlog) for logging
