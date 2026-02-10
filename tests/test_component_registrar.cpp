/// Unit tests for stagehand::ComponentRegistrar — the chaining API on component macros.
///
/// Tests verify:
///   1. then() provides general-purpose chaining via flecs::component<T>.
///   2. Traits, tags, pairs, and hooks are applied through then().
///   3. Multiple then() calls compose correctly.
///   4. Getter/setter registration survives chaining.
///   5. Integration with macros (FLOAT, INT16, TAG, etc.).

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <flecs.h>
#include "stagehand/registry.h"
#include "stagehand/ecs/components/macros.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Component definitions using chained modifiers
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {

    // A component with CanToggle trait added via then()
    INT16(Toggleable).then([](auto c) { c.add(flecs::CanToggle); });

    // A tag with CanToggle
    TAG(ToggleableTag).then([](auto c) { c.add(flecs::CanToggle); });

    // A component with multiple chained operations
    FLOAT(MultiChain).then([](auto c) { c.add(flecs::CanToggle); });

    // For pair testing
    TAG(MarkerA);
    TAG(MarkerB);

} // namespace test_registrar

// ═══════════════════════════════════════════════════════════════════════════════
// Fixture
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct RegistrarFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override {
            stagehand::register_components_and_systems_with_world(world);
        }
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: then() for adding traits
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(RegistrarFixture, AddCanToggleViaChain) {
    auto comp = world.component<test_registrar::Toggleable>();
    ASSERT_TRUE(comp.has(flecs::CanToggle));
}

TEST_F(RegistrarFixture, TagWithCanToggleViaChain) {
    auto comp = world.component<test_registrar::ToggleableTag>();
    ASSERT_TRUE(comp.has(flecs::CanToggle));
}

TEST_F(RegistrarFixture, ToggleableComponentCanBeDisabled) {
    auto e = world.entity();
    e.set<test_registrar::Toggleable>({ 42 });
    ASSERT_TRUE(e.has<test_registrar::Toggleable>());

    e.disable<test_registrar::Toggleable>();
    ASSERT_TRUE(e.has<test_registrar::Toggleable>());

    e.enable<test_registrar::Toggleable>();
    ASSERT_TRUE(e.has<test_registrar::Toggleable>());
}

TEST_F(RegistrarFixture, ToggleableTagCanBeDisabled) {
    auto e = world.entity();
    e.add<test_registrar::ToggleableTag>();
    ASSERT_TRUE(e.has<test_registrar::ToggleableTag>());

    e.disable<test_registrar::ToggleableTag>();
    ASSERT_TRUE(e.has<test_registrar::ToggleableTag>());

    e.enable<test_registrar::ToggleableTag>();
    ASSERT_TRUE(e.has<test_registrar::ToggleableTag>());
}

TEST_F(RegistrarFixture, MultiChainHasCanToggle) {
    auto comp = world.component<test_registrar::MultiChain>();
    ASSERT_TRUE(comp.has(flecs::CanToggle));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Programmatic ComponentRegistrar with multiple then() calls
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {
    struct ManualComponent {
        int data = 0;
    };

    inline auto register_ManualComponent = stagehand::ComponentRegistrar<ManualComponent>(
        [](flecs::world& world) {
        world.component<ManualComponent>().member<int>("data");
    }
    ).then([](auto c) {
        c.add(flecs::CanToggle);
    }).then([](auto c) {
        c.set_doc_name("ManualComponent");
    });
}

TEST_F(RegistrarFixture, ProgrammaticRegistrarWithThen) {
    auto comp = world.component<test_registrar::ManualComponent>();
    ASSERT_NE(comp.id(), 0u);
    ASSERT_TRUE(comp.has(flecs::CanToggle));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: then() — type-based tag
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {
    struct TypeTagA {};
    struct TypeTagged {
        float x = 0;
    };

    inline auto register_TypeTagA = stagehand::ComponentRegistrar<TypeTagA>(
        [](flecs::world& world) { world.component<TypeTagA>(); }
    );

    inline auto register_TypeTagged = stagehand::ComponentRegistrar<TypeTagged>(
        [](flecs::world& world) {
        world.component<TypeTagged>().member<float>("x");
    }
    ).then([](auto c) { c.template add<TypeTagA>(); });
}

TEST_F(RegistrarFixture, AddTypeAddsTagToComponent) {
    auto comp = world.component<test_registrar::TypeTagged>();
    ASSERT_TRUE(comp.has<test_registrar::TypeTagA>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: then() — pair by entity ids
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {
    struct PairRelation {};
    struct PairComponent {
        int val = 0;
    };

    inline auto register_PairRelation = stagehand::ComponentRegistrar<PairRelation>(
        [](flecs::world& world) { world.component<PairRelation>(); }
    );

    inline auto register_PairComponent = stagehand::ComponentRegistrar<PairComponent>(
        [](flecs::world& world) {
        world.component<PairComponent>().member<int>("val");
    }
    ).then([](auto c) { c.add(flecs::OnDelete, flecs::Panic); });
}

TEST_F(RegistrarFixture, AddPairByEntityIds) {
    auto comp = world.component<test_registrar::PairComponent>();
    ASSERT_TRUE(comp.has(flecs::OnDelete, flecs::Panic));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: then() — typed pair
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {
    struct Rel {};
    struct Target {};
    struct TypedPairComponent {
        int val = 0;
    };

    inline auto register_Rel = stagehand::ComponentRegistrar<Rel>(
        [](flecs::world& w) { w.component<Rel>(); }
    );
    inline auto register_Target = stagehand::ComponentRegistrar<Target>(
        [](flecs::world& w) { w.component<Target>(); }
    );
    inline auto register_TypedPairComponent = stagehand::ComponentRegistrar<TypedPairComponent>(
        [](flecs::world& w) {
        w.component<TypedPairComponent>().member<int>("val");
    }
    ).then([](auto c) { c.template add<Rel, Target>(); });
}

TEST_F(RegistrarFixture, AddTypedPair) {
    auto comp = world.component<test_registrar::TypedPairComponent>();
    bool has_pair = comp.template has<test_registrar::Rel, test_registrar::Target>();
    ASSERT_TRUE(has_pair);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: then() composing multiple operations in one call
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_registrar {
    struct ComposedComponent {
        int data = 0;
    };

    inline auto register_ComposedComponent = stagehand::ComponentRegistrar<ComposedComponent>(
        [](flecs::world& world) {
        world.component<ComposedComponent>().member<int>("data");
    }
    ).then([](auto c) {
        // Multiple operations in a single then() — flecs::component<T>
        // supports method chaining, so this composes naturally.
        c.add(flecs::CanToggle);
    });
}

TEST_F(RegistrarFixture, ThenComposesMultipleOperations) {
    auto comp = world.component<test_registrar::ComposedComponent>();
    ASSERT_TRUE(comp.has(flecs::CanToggle));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Entity-level usage of chained components
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(RegistrarFixture, ChainedComponentStillHasGetterAndSetter) {
    // INT16(Toggleable).add(flecs::CanToggle) should still register getters/setters.
    auto& getters = stagehand::get_component_getters();
    auto& setters = stagehand::get_component_setters();
    ASSERT_EQ(getters.count("Toggleable"), 1u);
    ASSERT_EQ(setters.count("Toggleable"), 1u);
}

TEST_F(RegistrarFixture, ChainedComponentUsedOnEntity) {
    auto e = world.entity();
    e.set<test_registrar::Toggleable>({ 100 });

    const auto* data = e.try_get<test_registrar::Toggleable>();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->value, 100);
}

TEST_F(RegistrarFixture, ChainedComponentQuery) {
    world.entity().set<test_registrar::Toggleable>({ 1 });
    world.entity().set<test_registrar::Toggleable>({ 2 });
    world.entity().set<test_registrar::Toggleable>({ 3 });

    int count = 0;
    int sum = 0;
    world.each([&](test_registrar::Toggleable& t) {
        ++count;
        sum += t.value;
    });

    ASSERT_EQ(count, 3);
    ASSERT_EQ(sum, 6);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Hook components (on_add, on_set, on_remove) via then()
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_hooks {

    // ── on_add: sets a marker value when the component is first added ────────

    // Shared flag: set to true by the on_add hook at registration time.
    // We use a shared_ptr so the captured state survives beyond the
    // static-initialization callback and stays valid for tests.
    inline auto on_add_fired = std::make_shared<bool>(false);

    INT32(OnAddTracked).then([](auto c) {
        c.on_add([](OnAddTracked& v) {
            v.value = 999;
        });
    });

    // ── on_set: records every set value ──────────────────────────────────────

    inline auto set_log = std::make_shared<std::vector<int>>();

    INT32(OnSetTracked).then([](auto c) {
        c.on_set([](OnSetTracked& v) {
            // Note: on_set fires after the value has been written.
            test_hooks::set_log->push_back(v.value);
        });
    });

    // ── on_remove: records that removal happened ─────────────────────────────

    inline auto on_remove_fired = std::make_shared<bool>(false);

    INT32(OnRemoveTracked).then([](auto c) {
        c.on_remove([](OnRemoveTracked&) {
            *test_hooks::on_remove_fired = true;
        });
    });

    // ── Combined hooks on a single component ─────────────────────────────────

    inline auto combined_log = std::make_shared<std::vector<std::string>>();

    INT32(CombinedHooks).then([](auto c) {
        c.on_add([](CombinedHooks&) {
            test_hooks::combined_log->push_back("add");
        });
    }).then([](auto c) {
        c.on_set([](CombinedHooks&) {
            test_hooks::combined_log->push_back("set");
        });
    }).then([](auto c) {
        c.on_remove([](CombinedHooks&) {
            test_hooks::combined_log->push_back("remove");
        });
    });

    // ── A plain component with no hooks (control) ────────────────────────────

    INT32(NoHooks);

} // namespace test_hooks

// ═══════════════════════════════════════════════════════════════════════════════
// Hook test fixture — resets shared state before each test
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct HookFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override {
            // Reset shared hook state
            *test_hooks::on_add_fired = false;
            *test_hooks::on_remove_fired = false;
            test_hooks::set_log->clear();
            test_hooks::combined_log->clear();

            stagehand::register_components_and_systems_with_world(world);
        }
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: on_add hook
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, on_add_fires_when_component_added) {
    auto e = world.entity();
    e.add<test_hooks::OnAddTracked>();

    const auto* data = e.try_get<test_hooks::OnAddTracked>();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->value, 999) << "on_add hook should have set value to 999";
}

TEST_F(HookFixture, on_add_fires_on_first_set) {
    auto e = world.entity();
    e.set<test_hooks::OnAddTracked>({ 5 });

    // on_add fires first (sets to 999), then set overwrites with 5.
    const auto* data = e.try_get<test_hooks::OnAddTracked>();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->value, 5);
}

TEST_F(HookFixture, on_add_does_not_fire_on_subsequent_set) {
    auto e = world.entity();
    e.set<test_hooks::OnAddTracked>({ 5 });
    e.set<test_hooks::OnAddTracked>({ 10 });

    const auto* data = e.try_get<test_hooks::OnAddTracked>();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->value, 10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: on_set hook
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, on_set_fires_when_value_is_set) {
    auto e = world.entity();
    e.set<test_hooks::OnSetTracked>({ 42 });

    ASSERT_FALSE(test_hooks::set_log->empty());
    ASSERT_EQ(test_hooks::set_log->back(), 42);
}

TEST_F(HookFixture, on_set_fires_on_every_set) {
    auto e = world.entity();
    e.set<test_hooks::OnSetTracked>({ 1 });
    e.set<test_hooks::OnSetTracked>({ 2 });
    e.set<test_hooks::OnSetTracked>({ 3 });

    ASSERT_GE(test_hooks::set_log->size(), 3u);
    // The last three entries must be 1, 2, 3 (in order).
    auto sz = test_hooks::set_log->size();
    ASSERT_EQ((*test_hooks::set_log)[sz - 3], 1);
    ASSERT_EQ((*test_hooks::set_log)[sz - 2], 2);
    ASSERT_EQ((*test_hooks::set_log)[sz - 1], 3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: on_remove hook
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, on_remove_fires_when_component_removed) {
    auto e = world.entity();
    e.set<test_hooks::OnRemoveTracked>({ 1 });
    ASSERT_FALSE(*test_hooks::on_remove_fired);

    e.remove<test_hooks::OnRemoveTracked>();
    ASSERT_TRUE(*test_hooks::on_remove_fired);
}

TEST_F(HookFixture, on_remove_does_not_fire_without_removal) {
    auto e = world.entity();
    e.set<test_hooks::OnRemoveTracked>({ 1 });
    e.set<test_hooks::OnRemoveTracked>({ 2 });
    ASSERT_FALSE(*test_hooks::on_remove_fired);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Combined hooks
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, combined_hooks_fire_in_lifecycle_order) {
    auto e = world.entity();
    e.set<test_hooks::CombinedHooks>({ 1 });   // triggers on_add + on_set
    e.set<test_hooks::CombinedHooks>({ 2 });   // triggers on_set only
    e.remove<test_hooks::CombinedHooks>();      // triggers on_remove

    // Expected sequence: add, set, set, remove
    ASSERT_GE(test_hooks::combined_log->size(), 4u);
    auto& log = *test_hooks::combined_log;
    auto sz = log.size();
    ASSERT_EQ(log[sz - 4], "add");
    ASSERT_EQ(log[sz - 3], "set");
    ASSERT_EQ(log[sz - 2], "set");
    ASSERT_EQ(log[sz - 1], "remove");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Hooks do NOT fire for unrelated components
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, hook_does_not_fire_for_other_components) {
    auto e = world.entity();
    e.set<test_hooks::NoHooks>({ 77 });

    // None of the hook flags should have been triggered.
    ASSERT_TRUE(test_hooks::set_log->empty());
    ASSERT_FALSE(*test_hooks::on_remove_fired);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Multiple entities with hooks
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(HookFixture, hooks_fire_independently_per_entity) {
    auto e1 = world.entity();
    auto e2 = world.entity();

    e1.set<test_hooks::OnSetTracked>({ 10 });
    e2.set<test_hooks::OnSetTracked>({ 20 });

    ASSERT_GE(test_hooks::set_log->size(), 2u);
    auto sz = test_hooks::set_log->size();
    ASSERT_EQ((*test_hooks::set_log)[sz - 2], 10);
    ASSERT_EQ((*test_hooks::set_log)[sz - 1], 20);
}
