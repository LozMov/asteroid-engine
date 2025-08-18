#include <filesystem>

#include "engine/Audio.hpp"
#include "engine/Engine.hpp"
#include "engine/Input.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/events/EventSubscriber.hpp"

#include "components/Components.hpp"
#include "systems/Systems.hpp"
#include "Prefab.hpp"

using namespace astd::components;
using namespace astd::systems;

class DemoEngine : public ast::Engine {
public:
    DemoEngine(const std::string& title, int width, int height, const std::string& assetsDir)
        : Engine(title, width, height, assetsDir), screenWidth_(width), screenHeight_(height) {}

    bool init() {
        if (!Engine::init()) {
            return false;
        }

        setup("level1.json");
        return true;
    }

protected:
    void update(float dt) override {
        // Handle game-specific input
        if (ast::Input::isKeyPressed(ast::Input::Scancode::ESCAPE)) {
            quit();
        }

        if (ast::Input::isKeyPressed(ast::Input::Scancode::F12)) {
            setup("level1.json");
            AST_INFO("Game restarted");
        }

        registry_.update(dt);
    }

    void setup(const std::string& levelFile) {
        registry_ = ast::Registry();

        registry_.context.screenWidth = static_cast<float>(screenWidth_);
        registry_.context.screenHeight = static_cast<float>(screenHeight_);
        registry_.context.renderer = renderer_;

        registry_.attach<AnimationSystem>(registry_);
        registry_.attach<InputSystem>(registry_);
        registry_.attach<PlayerControlSystem>(registry_);
        registry_.attach<EnemySystem>(registry_);
        registry_.attach<PhysicsSystem>(registry_);
        registry_.attach<CameraSystem>(registry_);
        registry_.attach<RenderSystem>(registry_);
        registry_.attach<UISystem>(registry_);
        // registry_.attach<DebugSystem>(registry_);
        // registry_.attach<EntityCleanupSystem>(registry_);

        std::vector<std::string> prefabs =
            astd::Prefab::loadPrefabsFromFile(registry_, assetsDirectory_ + "/prefabs.json");
        // for (const auto& prefab : prefabs) {
        //     Prefab::createEntityFromPrefab(registry_, prefab);
        // }
        auto entities = astd::Prefab::loadLevel(registry_, assetsDirectory_ + "/" + levelFile);
        
        // Set up camera to follow the player
        ast::Entity playerEntity{};
        for (const auto& entityData : entities) {
            if (entityData.prefab == "Player") {
                playerEntity = entityData.entity;
                registry_.context.playerEntity = playerEntity; // Store player entity reference
                break;
            }
        }
        
        // Find the camera and set its target to the player
        for (const auto& [entity, cameraPtr] : registry_.getAll<astd::components::Camera>()) {
            auto* camera = static_cast<astd::components::Camera*>(cameraPtr.get());
            camera->target = playerEntity;
            AST_INFO("Set camera target to player entity {}", playerEntity);
            break;
        }
    }

    ast::Registry registry_;
    int screenWidth_;
    int screenHeight_;
};

std::filesystem::path findAssetsDirectory(const char* argv0, const char* dirName) {
    std::filesystem::path currentPath = std::filesystem::path(argv0).parent_path();
    while (!std::filesystem::is_directory(currentPath / dirName)) {
        if (!currentPath.has_parent_path()) {
            throw std::runtime_error("Could not find assets directory");
        }
        currentPath = currentPath.parent_path();
    }
    return currentPath / dirName;
}

int main(int argc, char* argv[]) {
    auto* game =
        new DemoEngine("Demo", 1920, 1080, findAssetsDirectory(argv[0], "assets").string());

    if (!game->init()) {
        AST_ERROR("Failed to initialize game");
        return -1;
    }
    game->run();

    delete game;
    return 0;
}