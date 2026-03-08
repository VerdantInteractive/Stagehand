#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/entity.h"
#include "stagehand/registry.h"

namespace test_entity_wrapper {
    FLOAT(TrackedFloat, 0.0f);
    FLOAT_(UntrackedFloat, 0.0f);
} // namespace test_entity_wrapper

namespace {
    struct EntityWrapperFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

TEST_F(EntityWrapperFixture, SetAddsComponentAndMarksTrackedComponentChanged) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TrackedFloat>({3.5f});

    ASSERT_TRUE(entity.has<test_entity_wrapper::TrackedFloat>());
    ASSERT_TRUE(entity.has<test_entity_wrapper::TrackedFloat::ChangeTag>());

    const test_entity_wrapper::TrackedFloat *value = entity.try_get<test_entity_wrapper::TrackedFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 3.5f);
}

TEST_F(EntityWrapperFixture, SetUpdatesValueAndMarksTrackedComponentChanged) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TrackedFloat>({1.0f});
    entity.disable<test_entity_wrapper::TrackedFloat::ChangeTag>();

    entity.set<test_entity_wrapper::TrackedFloat>({9.0f});

    const test_entity_wrapper::TrackedFloat *value = entity.try_get<test_entity_wrapper::TrackedFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 9.0f);
    ASSERT_TRUE(entity.has<test_entity_wrapper::TrackedFloat::ChangeTag>());
}

TEST_F(EntityWrapperFixture, ModifyInPlaceUpdatesComponentAndMarksChanged) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TrackedFloat>({2.0f});
    entity.disable<test_entity_wrapper::TrackedFloat::ChangeTag>();

    entity.modify<test_entity_wrapper::TrackedFloat>([](test_entity_wrapper::TrackedFloat &value) { value.value += 6.0f; });

    const test_entity_wrapper::TrackedFloat *value = entity.try_get<test_entity_wrapper::TrackedFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 8.0f);
    ASSERT_TRUE(entity.has<test_entity_wrapper::TrackedFloat::ChangeTag>());
}

TEST_F(EntityWrapperFixture, ModifyMissingComponentDoesNothing) {
    stagehand::entity entity = world.entity();
    entity.modify<test_entity_wrapper::TrackedFloat>([](test_entity_wrapper::TrackedFloat &value) { value.value = 100.0f; });

    ASSERT_FALSE(entity.has<test_entity_wrapper::TrackedFloat>());
    ASSERT_FALSE(entity.has<test_entity_wrapper::TrackedFloat::ChangeTag>());
}

TEST_F(EntityWrapperFixture, RvalueSetWorksForTrackedAndUntrackedComponents) {
    stagehand::entity tracked_entity = world.entity();
    tracked_entity.set(test_entity_wrapper::TrackedFloat(7.0f));

    const test_entity_wrapper::TrackedFloat *tracked_value = tracked_entity.try_get<test_entity_wrapper::TrackedFloat>();
    ASSERT_NE(tracked_value, nullptr);
    ASSERT_FLOAT_EQ(tracked_value->value, 7.0f);
    ASSERT_TRUE(tracked_entity.has<test_entity_wrapper::TrackedFloat::ChangeTag>());

    stagehand::entity untracked_entity = world.entity();
    untracked_entity.set(test_entity_wrapper::UntrackedFloat(4.0f));

    const test_entity_wrapper::UntrackedFloat *untracked_value = untracked_entity.try_get<test_entity_wrapper::UntrackedFloat>();
    ASSERT_NE(untracked_value, nullptr);
    ASSERT_FLOAT_EQ(untracked_value->value, 4.0f);
}
