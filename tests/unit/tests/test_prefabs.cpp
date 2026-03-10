/// Unit tests for Entity2D and Entity3D prefabs.
/// Tests verify:
///   1. Prefabs are registered and can be looked up by name.
///   2. Prefabs have the expected components.
///   3. Entities instantiated from prefabs inherit the correct components.
///   4. Prefab instances can have their components overridden.

#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/prefabs/entity.h"
#include "stagehand/entity.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace {
    struct PrefabFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };

    constexpr float EPSILON = 1e-4f;
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Prefab registration
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, Entity2DPrefabExists) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    ASSERT_TRUE(prefab.is_valid());
    ASSERT_TRUE(prefab.has(flecs::Prefab));
}

TEST_F(PrefabFixture, Entity3DPrefabExists) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    ASSERT_TRUE(prefab.is_valid());
    ASSERT_TRUE(prefab.has(flecs::Prefab));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Entity2D prefab component composition
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, Entity2DPrefabHasPosition2D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Position2D>());
}

TEST_F(PrefabFixture, Entity2DPrefabHasRotation2D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Rotation2D>());
}

TEST_F(PrefabFixture, Entity2DPrefabHasScale2D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Scale2D>());
}

TEST_F(PrefabFixture, Entity2DPrefabHasTransform2D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Transform2D>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Entity3D prefab component composition
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, Entity3DPrefabHasPosition3D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Position3D>());
}

TEST_F(PrefabFixture, Entity3DPrefabHasRotation3D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Rotation3D>());
}

TEST_F(PrefabFixture, Entity3DPrefabHasScale3D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Scale3D>());
}

TEST_F(PrefabFixture, Entity3DPrefabHasTransform3D) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    ASSERT_TRUE(prefab.has<stagehand::transform::Transform3D>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Instantiation from prefab
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, InstantiateEntity2DInheritsComponents) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    flecs::entity instance = world.entity().is_a(prefab);

    ASSERT_TRUE(instance.has<stagehand::transform::Position2D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Rotation2D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Scale2D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Transform2D>());
}

TEST_F(PrefabFixture, InstantiateEntity3DInheritsComponents) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    flecs::entity instance = world.entity().is_a(prefab);

    ASSERT_TRUE(instance.has<stagehand::transform::Position3D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Rotation3D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Scale3D>());
    ASSERT_TRUE(instance.has<stagehand::transform::Transform3D>());
}

TEST_F(PrefabFixture, PrefabInstanceCanOverrideComponents) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    stagehand::entity instance = world.entity().is_a(prefab);

    instance.set<stagehand::transform::Position2D>(stagehand::transform::Position2D(godot::Vector2(50, 75)));

    const stagehand::transform::Position2D *pos = instance.try_get<stagehand::transform::Position2D>();
    ASSERT_NE(pos, nullptr);
    ASSERT_NEAR(pos->x, 50.0f, EPSILON);
    ASSERT_NEAR(pos->y, 75.0f, EPSILON);
}

TEST_F(PrefabFixture, MultiplePrefabInstancesAreIndependent) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);

    stagehand::entity inst1 = world.entity().is_a(prefab);
    stagehand::entity inst2 = world.entity().is_a(prefab);

    inst1.set<stagehand::transform::Position3D>(stagehand::transform::Position3D(godot::Vector3(1, 2, 3)));
    inst2.set<stagehand::transform::Position3D>(stagehand::transform::Position3D(godot::Vector3(4, 5, 6)));

    const stagehand::transform::Position3D *pos1 = inst1.try_get<stagehand::transform::Position3D>();
    const stagehand::transform::Position3D *pos2 = inst2.try_get<stagehand::transform::Position3D>();

    ASSERT_NE(pos1, nullptr);
    ASSERT_NE(pos2, nullptr);
    ASSERT_NEAR(pos1->x, 1.0f, EPSILON);
    ASSERT_NEAR(pos2->x, 4.0f, EPSILON);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Query-based tests: find entities by prefab parent
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, QueryByPrefabInheritance) {
    flecs::entity prefab2d = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    flecs::entity prefab3d = world.lookup(stagehand::names::prefabs::ENTITY_3D);

    world.entity().is_a(prefab2d);
    world.entity().is_a(prefab2d);
    world.entity().is_a(prefab3d);

    auto query2d = world.query_builder<>().with(flecs::IsA, prefab2d).build();
    auto query3d = world.query_builder<>().with(flecs::IsA, prefab3d).build();

    ASSERT_EQ(query2d.count(), 2);
    ASSERT_EQ(query3d.count(), 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Default values
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PrefabFixture, Entity2DScaleDefaultsToOne) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_2D);
    stagehand::entity instance = world.entity().is_a(prefab);

    const stagehand::transform::Scale2D *scale = instance.try_get<stagehand::transform::Scale2D>();
    ASSERT_NE(scale, nullptr);
    ASSERT_NEAR(scale->x, 1.0f, EPSILON);
    ASSERT_NEAR(scale->y, 1.0f, EPSILON);
}

TEST_F(PrefabFixture, Entity3DScaleDefaultsToOne) {
    flecs::entity prefab = world.lookup(stagehand::names::prefabs::ENTITY_3D);
    stagehand::entity instance = world.entity().is_a(prefab);

    const stagehand::transform::Scale3D *scale = instance.try_get<stagehand::transform::Scale3D>();
    ASSERT_NE(scale, nullptr);
    ASSERT_NEAR(scale->x, 1.0f, EPSILON);
    ASSERT_NEAR(scale->y, 1.0f, EPSILON);
    ASSERT_NEAR(scale->z, 1.0f, EPSILON);
}
