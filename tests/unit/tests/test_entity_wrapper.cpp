#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/entity.h"
#include "stagehand/registry.h"

namespace test_entity_wrapper {
    FLOAT(TestFloat, 0.0f);
} // namespace test_entity_wrapper

namespace {
    struct EntityWrapperFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

TEST_F(EntityWrapperFixture, SetAddsComponent) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TestFloat>({3.5f});

    ASSERT_TRUE(entity.has<test_entity_wrapper::TestFloat>());

    const test_entity_wrapper::TestFloat *value = entity.try_get<test_entity_wrapper::TestFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 3.5f);
}

TEST_F(EntityWrapperFixture, SetUpdatesValue) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TestFloat>({1.0f});
    entity.set<test_entity_wrapper::TestFloat>({9.0f});

    const test_entity_wrapper::TestFloat *value = entity.try_get<test_entity_wrapper::TestFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 9.0f);
}

TEST_F(EntityWrapperFixture, ModifyInPlaceUpdatesComponent) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_wrapper::TestFloat>({2.0f});

    entity.modify<test_entity_wrapper::TestFloat>([](test_entity_wrapper::TestFloat &value) { value.value += 6.0f; });

    const test_entity_wrapper::TestFloat *value = entity.try_get<test_entity_wrapper::TestFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 8.0f);
}

TEST_F(EntityWrapperFixture, ModifyMissingComponentDoesNothing) {
    stagehand::entity entity = world.entity();
    entity.modify<test_entity_wrapper::TestFloat>([](test_entity_wrapper::TestFloat &value) { value.value = 100.0f; });

    ASSERT_FALSE(entity.has<test_entity_wrapper::TestFloat>());
}

TEST_F(EntityWrapperFixture, RvalueSetWorks) {
    stagehand::entity entity = world.entity();
    entity.set(test_entity_wrapper::TestFloat(7.0f));

    const test_entity_wrapper::TestFloat *value = entity.try_get<test_entity_wrapper::TestFloat>();
    ASSERT_NE(value, nullptr);
    ASSERT_FLOAT_EQ(value->value, 7.0f);
}
