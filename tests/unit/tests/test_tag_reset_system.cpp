/// Unit tests for the tag reset (change detection) system.
/// Tests verify:
///   1. Change detection tags are disabled after PostRender phase.
///   2. Tags are re-enabled when components are set again.
///   3. Multiple entities' change tags are all reset.

#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/traits.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/entity.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace test_tag_reset {
    FLOAT(TrackedA, 0.0f);
    FLOAT(TrackedB, 0.0f);
    INT32(TrackedC, 0);
    GODOT_VARIANT(TrackedVec, godot::Vector2);
} // namespace test_tag_reset

namespace {
    struct TagResetFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };

    bool entity_has_enabled_change_tag(flecs::entity entity) {
        bool found_enabled = false;
        entity.each([&found_enabled, &entity](flecs::id id) {
            if (id.is_pair()) {
                return;
            }
            flecs::entity comp = id.entity();
            if (comp.is_valid() && comp.has<stagehand::IsChangeDetectionTag>()) {
                // CanToggle means the tag can be disabled. Check if it's still enabled.
                if (entity.enabled(id)) {
                    found_enabled = true;
                }
            }
        });
        return found_enabled;
    }
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Tag reset system is registered
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TagResetFixture, TagResetSystemIsRegistered) {
    flecs::entity system = world.lookup(stagehand::names::systems::TAG_RESET_CHANGE_DETECTION);
    ASSERT_TRUE(system.is_valid());
    ASSERT_TRUE(system.has(flecs::System));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Change tags are disabled after progress
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TagResetFixture, ChangeTagDisabledAfterProgress) {
    stagehand::entity entity = world.entity();
    entity.set<test_tag_reset::TrackedA>({5.0f});

    // Change tag should be enabled after set
    ASSERT_TRUE(entity.has<test_tag_reset::TrackedA::ChangeTag>());

    // After a full tick, the PostRender tag reset system should have disabled it
    world.progress(0.016f);

    // The change tag should now be disabled (not in the entity's active components)
    ASSERT_FALSE(entity_has_enabled_change_tag(entity));
}

TEST_F(TagResetFixture, MultipleComponentChangesAllResetAfterProgress) {
    stagehand::entity entity = world.entity();
    entity.set<test_tag_reset::TrackedA>({1.0f});
    entity.set<test_tag_reset::TrackedB>({2.0f});
    entity.set<test_tag_reset::TrackedC>({3});

    // All change tags should be set initially
    ASSERT_TRUE(entity.has<test_tag_reset::TrackedA::ChangeTag>());
    ASSERT_TRUE(entity.has<test_tag_reset::TrackedB::ChangeTag>());
    ASSERT_TRUE(entity.has<test_tag_reset::TrackedC::ChangeTag>());

    world.progress(0.016f);

    // All should be reset after progress
    ASSERT_FALSE(entity_has_enabled_change_tag(entity));
}

TEST_F(TagResetFixture, ChangeTagReEnabledOnSubsequentSet) {
    stagehand::entity entity = world.entity();
    entity.set<test_tag_reset::TrackedA>({5.0f});

    // First tick — tags reset
    world.progress(0.016f);
    ASSERT_FALSE(entity_has_enabled_change_tag(entity));

    // Set again — tag should re-enable
    entity.set<test_tag_reset::TrackedA>({10.0f});
    ASSERT_TRUE(entity.has<test_tag_reset::TrackedA::ChangeTag>());

    // Second tick — tags reset again
    world.progress(0.016f);
    ASSERT_FALSE(entity_has_enabled_change_tag(entity));
}

TEST_F(TagResetFixture, MultipleEntitiesAllHaveTagsReset) {
    stagehand::entity e1 = world.entity();
    stagehand::entity e2 = world.entity();
    stagehand::entity e3 = world.entity();

    e1.set<test_tag_reset::TrackedA>({1.0f});
    e2.set<test_tag_reset::TrackedA>({2.0f});
    e3.set<test_tag_reset::TrackedA>({3.0f});

    world.progress(0.016f);

    ASSERT_FALSE(entity_has_enabled_change_tag(e1));
    ASSERT_FALSE(entity_has_enabled_change_tag(e2));
    ASSERT_FALSE(entity_has_enabled_change_tag(e3));
}

TEST_F(TagResetFixture, OnlyChangedEntitiesHaveTagsToReset) {
    stagehand::entity e1 = world.entity();
    stagehand::entity e2 = world.entity();

    e1.set<test_tag_reset::TrackedA>({1.0f});
    e2.set<test_tag_reset::TrackedA>({2.0f});

    // First tick: both get change tags reset
    world.progress(0.016f);

    // Only modify e1, not e2
    e1.set<test_tag_reset::TrackedA>({10.0f});
    ASSERT_TRUE(e1.has<test_tag_reset::TrackedA::ChangeTag>());

    // e2 should still have no active change tags
    ASSERT_FALSE(entity_has_enabled_change_tag(e2));

    // After progress, e1's tag should also be reset
    world.progress(0.016f);
    ASSERT_FALSE(entity_has_enabled_change_tag(e1));
}

TEST_F(TagResetFixture, GodotVariantChangeTagAlsoResets) {
    stagehand::entity entity = world.entity();
    entity.set<test_tag_reset::TrackedVec>(test_tag_reset::TrackedVec(godot::Vector2(1, 2)));

    ASSERT_TRUE(entity.has<test_tag_reset::TrackedVec::ChangeTag>());

    world.progress(0.016f);

    ASSERT_FALSE(entity_has_enabled_change_tag(entity));
}
