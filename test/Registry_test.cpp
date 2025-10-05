#include "gtest/gtest.h"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Component.hpp"

class TestComponent : public ast::Component {
public:
    TestComponent() {}
};

TEST(Registry, Test) {
    ast::Registry registry;
    auto entity = registry.createEntity();
    // registry.emplace<TestComponent>(entity);
    EXPECT_EQ(registry.get<TestComponent>(entity), nullptr);
}

// int main(int argc, char** argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }