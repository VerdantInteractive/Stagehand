/// Unit tests for stagehand::Registry and related registration infrastructure.
///
/// Tests verify that:
///   1. Registration callbacks are invoked during world initialization.
///   2. Callbacks run in the correct world.
///   3. Null callbacks are safely rejected.
///   4. The Registry struct constructor registers callbacks.
///   5. Callbacks can register components and create entities.

#include <flecs.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Test components defined via macros (registered at static init time)
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registry {
    INT32(RegistryProbe);
    TAG(RegistryTag);
} // namespace test_registry

// ═══════════════════════════════════════════════════════════════════════════════
// Fixture: creates a fresh world and runs all registrations
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct RegistryFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Registration pipeline
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(RegistryFixture, CallbacksRegisterMacroDefinedComponents) {
    // The macro-defined components should exist in the world after registration.
    auto c = world.component<test_registry::RegistryProbe>();
    ASSERT_NE(c.id(), 0u);
}

TEST_F(RegistryFixture, CallbacksRegisterTags) {
    auto c = world.component<test_registry::RegistryTag>();
    ASSERT_NE(c.id(), 0u);
}

TEST_F(RegistryFixture, CallbackIsInvokedOnWorldInit) {
    // Register a new callback and verify it runs on a fresh world.
    auto was_called = std::make_shared<bool>(false);
    stagehand::register_callback([was_called](flecs::world &) { *was_called = true; });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);
    ASSERT_TRUE(*was_called);
}

TEST_F(RegistryFixture, MultipleCallbacksPreserveRelativeOrder) {
    auto order = std::make_shared<std::vector<int>>();
    stagehand::register_callback([order](flecs::world &) { order->push_back(1); });
    stagehand::register_callback([order](flecs::world &) { order->push_back(2); });
    stagehand::register_callback([order](flecs::world &) { order->push_back(3); });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);

    // The three values must appear in order, though other callbacks may interleave.
    ASSERT_GE(order->size(), 3u);
    int prev_idx = -1;
    for (int expected : {1, 2, 3}) {
        auto it = std::find(order->begin() + prev_idx + 1, order->end(), expected);
        ASSERT_NE(it, order->end()) << "Expected " << expected << " in order";
        prev_idx = static_cast<int>(std::distance(order->begin(), it));
    }
}

TEST_F(RegistryFixture, StructConstructorRegistersCallback) {
    auto was_called = std::make_shared<bool>(false);
    stagehand::Registry reg([was_called](flecs::world &) { *was_called = true; });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);
    ASSERT_TRUE(*was_called);
}

TEST_F(RegistryFixture, NullCallbacksAreSafelyIgnored) {
    stagehand::register_callback(nullptr);
    // If nullptr were not guarded, the next registration call would crash.
    flecs::world w2;
    ASSERT_NO_THROW(stagehand::register_components_and_systems_with_world(w2));
}

TEST_F(RegistryFixture, CallbacksRunWithCorrectWorld) {
    auto captured = std::make_shared<flecs::world *>(nullptr);
    stagehand::register_callback([captured](flecs::world &w) { *captured = &w; });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);
    ASSERT_EQ(*captured, &w2);
}

TEST_F(RegistryFixture, RegistrationIsIdempotentAcrossWorlds) {
    // Calling registration on multiple worlds should work without conflict.
    flecs::world w2;
    ASSERT_NO_THROW(stagehand::register_components_and_systems_with_world(w2));

    auto c1 = world.component<test_registry::RegistryProbe>();
    auto c2 = w2.component<test_registry::RegistryProbe>();
    ASSERT_NE(c1.id(), 0u);
    ASSERT_NE(c2.id(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Component getter/setter map infrastructure
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(RegistryFixture, GetterMapReturnsSameInstance) {
    auto &a = stagehand::get_component_registry();
    auto &b = stagehand::get_component_registry();
    ASSERT_EQ(&a, &b);
}

TEST_F(RegistryFixture, SetterMapReturnsSameInstance) {
    auto &a = stagehand::get_component_registry();
    auto &b = stagehand::get_component_registry();
    ASSERT_EQ(&a, &b);
}

TEST_F(RegistryFixture, GetterMapIsPopulatedAfterRegistration) {
    auto &registry = stagehand::get_component_registry();
    // At least the macro-defined RegistryProbe should be registered.
    ASSERT_GE(registry.size(), 1u);
    ASSERT_EQ(registry.count("RegistryProbe"), 1u);
}

TEST_F(RegistryFixture, SetterMapIsPopulatedAfterRegistration) {
    auto &registry = stagehand::get_component_registry();
    ASSERT_GE(registry.size(), 1u);
    ASSERT_EQ(registry.count("RegistryProbe"), 1u);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Callback can register components and create entities
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct AdHocComponent {
        int value = 0;
    };
} // namespace

TEST_F(RegistryFixture, CallbackCanRegisterComponent) {
    stagehand::register_callback([](flecs::world &w) { w.component<AdHocComponent>().member<int>("value"); });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);

    auto c = w2.component<AdHocComponent>();
    ASSERT_NE(c.id(), 0u);
}

TEST_F(RegistryFixture, CallbackCanCreateEntityWithComponent) {
    stagehand::register_callback([](flecs::world &w) {
        w.component<AdHocComponent>().member<int>("value");
        w.entity("adhoc_entity").set<AdHocComponent>({42});
    });

    flecs::world w2;
    stagehand::register_components_and_systems_with_world(w2);

    auto e = w2.lookup("adhoc_entity");
    ASSERT_TRUE(e.is_valid());
    ASSERT_TRUE(e.has<AdHocComponent>());
    const auto *data = e.try_get<AdHocComponent>();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->value, 42);
}
