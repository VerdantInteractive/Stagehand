/// Tests for the `REGISTER` and `REGISTER_IN_MODULE` registration macros and
/// the module-aware `Registry` constructor.

#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/registry.h"

namespace test_registry_macros {
    struct SimpleMacroComponent {};

    namespace some_module {
        struct ModuleComponent {};
    } // namespace some_module
} // namespace test_registry_macros

// Register at translation-unit scope using macros.
REGISTER([](flecs::world &w) { w.component<test_registry_macros::SimpleMacroComponent>(); });
REGISTER_IN_MODULE(test_registry_macros::some_module, [](flecs::world &w) { w.component<test_registry_macros::some_module::ModuleComponent>(); });

namespace {
    struct RegistryMacrosFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

TEST_F(RegistryMacrosFixture, RegisterMacroRegistersComponent) {
    auto c = world.component<test_registry_macros::SimpleMacroComponent>();
    ASSERT_NE(c.id(), 0u);
}

TEST_F(RegistryMacrosFixture, RegisterInModuleCreatesModuleScopedComponent) {
    auto c = world.component<test_registry_macros::some_module::ModuleComponent>();
    ASSERT_NE(c.id(), 0u);

    // Verify the component can be looked up by its fully-qualified name.
    auto e = world.lookup("test_registry_macros::some_module::ModuleComponent");
    ASSERT_TRUE(e.is_valid());
}
