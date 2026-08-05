/// Unit tests for stagehand::entity wrapper — basic functionality.
/// Tests verify:
///   1. set() and get() operations.
///   2. modify() with existing reference overload.
///   3. Multiple component operations on a single entity.
///   4. Entity destruction and component cleanup.
///   5. Tag operations.

#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/entity.h"
#include "stagehand/registry.h"

namespace test_entity_ext {
    FLOAT(Health, 100.0f);
    FLOAT(Mana, 50.0f);
    INT32(Score, 0);
    TAG(Alive);
    TAG(Poisoned);
    GODOT_VARIANT(Pos2D, Vector2);
    GODOT_VARIANT(Vel2D, Vector2);
    VECTOR(Inventory, int);
} // namespace test_entity_ext

namespace {
    struct EntityExtFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// set() and get() tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EntityExtFixture, SetAndGetComponent) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({75.0f});

    ASSERT_TRUE(entity.has<test_entity_ext::Health>());
    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Health>()->value, 75.0f);
}

TEST_F(EntityExtFixture, SetMultipleComponentsOnSameEntity) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({100.0f});
    entity.set<test_entity_ext::Mana>({50.0f});
    entity.set<test_entity_ext::Score>({0});
    entity.set<test_entity_ext::Pos2D>(test_entity_ext::Pos2D(godot::Vector2(1.0f, 2.0f)));
    entity.set<test_entity_ext::Vel2D>(test_entity_ext::Vel2D(godot::Vector2(0.5f, -0.5f)));
    entity.set<test_entity_ext::Inventory>(test_entity_ext::Inventory({1, 2, 3}));

    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Health>()->value, 100.0f);
    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Mana>()->value, 50.0f);
    ASSERT_EQ(entity.try_get<test_entity_ext::Score>()->value, 0);
    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Pos2D>()->x, 1.0f);
    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Vel2D>()->y, -0.5f);
    ASSERT_EQ(entity.try_get<test_entity_ext::Inventory>()->value.size(), 3u);
}

// ═══════════════════════════════════════════════════════════════════════════════
// modify() tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EntityExtFixture, ModifyWithExistingRefUpdatesValue) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({100.0f});

    test_entity_ext::Health *health = entity.try_get_mut<test_entity_ext::Health>();
    ASSERT_NE(health, nullptr);

    entity.modify(*health, [](test_entity_ext::Health &h) { h.value -= 25.0f; });

    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Health>()->value, 75.0f);
}

TEST_F(EntityExtFixture, ModifyWithLookupUpdatesValue) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({100.0f});

    entity.modify<test_entity_ext::Health>([](test_entity_ext::Health &h) { h.value -= 25.0f; });

    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Health>()->value, 75.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Multiple operations
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EntityExtFixture, SetThenSetAgainOverwritesValue) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Score>({10});
    entity.set<test_entity_ext::Score>({20});

    ASSERT_EQ(entity.try_get<test_entity_ext::Score>()->value, 20);
}

TEST_F(EntityExtFixture, SetThenModifyComposesCorrectly) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({50.0f});
    entity.modify<test_entity_ext::Health>([](test_entity_ext::Health &h) { h.value += 10.0f; });

    ASSERT_FLOAT_EQ(entity.try_get<test_entity_ext::Health>()->value, 60.0f);
}

TEST_F(EntityExtFixture, MultipleModifiesAccumulate) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Score>({0});

    for (int i = 0; i < 10; ++i) {
        entity.modify<test_entity_ext::Score>([](test_entity_ext::Score &s) { s.value += 1; });
    }

    ASSERT_EQ(entity.try_get<test_entity_ext::Score>()->value, 10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tag operations on stagehand::entity
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EntityExtFixture, AddAndRemoveTags) {
    stagehand::entity entity = world.entity();
    entity.add<test_entity_ext::Alive>();
    ASSERT_TRUE(entity.has<test_entity_ext::Alive>());

    entity.add<test_entity_ext::Poisoned>();
    ASSERT_TRUE(entity.has<test_entity_ext::Alive>());
    ASSERT_TRUE(entity.has<test_entity_ext::Poisoned>());

    entity.remove<test_entity_ext::Poisoned>();
    ASSERT_TRUE(entity.has<test_entity_ext::Alive>());
    ASSERT_FALSE(entity.has<test_entity_ext::Poisoned>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Entity destruction
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EntityExtFixture, DestroyedEntityIsNotAlive) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Health>({50.0f});
    ASSERT_TRUE(entity.is_alive());

    entity.destruct();
    ASSERT_FALSE(entity.is_alive());
}

TEST_F(EntityExtFixture, DestroyedEntityDoesNotAppearInQueries) {
    stagehand::entity e1 = world.entity();
    e1.set<test_entity_ext::Score>({10});

    stagehand::entity e2 = world.entity();
    e2.set<test_entity_ext::Score>({20});

    e1.destruct();

    int sum = 0;
    int count = 0;
    world.each([&](const test_entity_ext::Score &s) {
        sum += s.value;
        ++count;
    });

    ASSERT_EQ(count, 1);
    ASSERT_EQ(sum, 20);
}

TEST_F(EntityExtFixture, IncrementOperatorsWorkOnWrappedType) {
    stagehand::entity entity = world.entity();
    entity.set<test_entity_ext::Score>({5});

    // Use modify to do pre-increment
    entity.modify<test_entity_ext::Score>([](test_entity_ext::Score &s) { ++s; });
    ASSERT_EQ(entity.try_get<test_entity_ext::Score>()->value, 6);

    // Use modify to do post-decrement
    entity.modify<test_entity_ext::Score>([](test_entity_ext::Score &s) { s--; });
    ASSERT_EQ(entity.try_get<test_entity_ext::Score>()->value, 5);
}
