/// Unit tests for transform system traits — compose and decompose.
/// These test the pure math operations used by the transform systems
/// without requiring a running Godot engine.

#include <cmath>
#include <flecs.h>
#include <gtest/gtest.h>

#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/systems/transform.h"
#include "stagehand/entity.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace {
    struct TransformSystemFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };

    constexpr float EPSILON = 1e-4f;
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// 2D Transform compose tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TransformSystemFixture, Compose2DIdentity) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(0, 0)), stagehand::transform::Rotation2D(0.0f),
                              stagehand::transform::Scale2D(godot::Vector2(1, 1)));

    ASSERT_NEAR(result.get_origin().x, 0.0f, EPSILON);
    ASSERT_NEAR(result.get_origin().y, 0.0f, EPSILON);
    ASSERT_NEAR(result.get_rotation(), 0.0f, EPSILON);
    ASSERT_NEAR(result.get_scale().x, 1.0f, EPSILON);
    ASSERT_NEAR(result.get_scale().y, 1.0f, EPSILON);
}

TEST_F(TransformSystemFixture, Compose2DTranslation) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(100, 200)), stagehand::transform::Rotation2D(0.0f),
                              stagehand::transform::Scale2D(godot::Vector2(1, 1)));

    ASSERT_NEAR(result.get_origin().x, 100.0f, EPSILON);
    ASSERT_NEAR(result.get_origin().y, 200.0f, EPSILON);
}

TEST_F(TransformSystemFixture, Compose2DScale) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(0, 0)), stagehand::transform::Rotation2D(0.0f),
                              stagehand::transform::Scale2D(godot::Vector2(2, 3)));

    ASSERT_NEAR(result.get_scale().x, 2.0f, EPSILON);
    ASSERT_NEAR(result.get_scale().y, 3.0f, EPSILON);
}

TEST_F(TransformSystemFixture, Compose2DRotation) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    float angle = static_cast<float>(M_PI / 4.0); // 45 degrees
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(0, 0)), stagehand::transform::Rotation2D(angle),
                              stagehand::transform::Scale2D(godot::Vector2(1, 1)));

    ASSERT_NEAR(result.get_rotation(), angle, EPSILON);
}

TEST_F(TransformSystemFixture, Compose2DCombinedPRS) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    float angle = static_cast<float>(M_PI / 2.0);
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(10, 20)), stagehand::transform::Rotation2D(angle),
                              stagehand::transform::Scale2D(godot::Vector2(2, 3)));

    ASSERT_NEAR(result.get_origin().x, 10.0f, EPSILON);
    ASSERT_NEAR(result.get_origin().y, 20.0f, EPSILON);
    ASSERT_NEAR(result.get_rotation(), angle, EPSILON);
    ASSERT_NEAR(result.get_scale().x, 2.0f, EPSILON);
    ASSERT_NEAR(result.get_scale().y, 3.0f, EPSILON);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 3D Transform compose tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TransformSystemFixture, Compose3DIdentity) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform3D>;
    stagehand::transform::Transform3D result;
    Traits::compose_transform(result, stagehand::transform::Position3D(godot::Vector3(0, 0, 0)), stagehand::transform::Rotation3D(godot::Quaternion()),
                              stagehand::transform::Scale3D(godot::Vector3(1, 1, 1)));

    ASSERT_NEAR(result.origin.x, 0.0f, EPSILON);
    ASSERT_NEAR(result.origin.y, 0.0f, EPSILON);
    ASSERT_NEAR(result.origin.z, 0.0f, EPSILON);
}

TEST_F(TransformSystemFixture, Compose3DTranslation) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform3D>;
    stagehand::transform::Transform3D result;
    Traits::compose_transform(result, stagehand::transform::Position3D(godot::Vector3(5, 10, 15)), stagehand::transform::Rotation3D(godot::Quaternion()),
                              stagehand::transform::Scale3D(godot::Vector3(1, 1, 1)));

    ASSERT_NEAR(result.origin.x, 5.0f, EPSILON);
    ASSERT_NEAR(result.origin.y, 10.0f, EPSILON);
    ASSERT_NEAR(result.origin.z, 15.0f, EPSILON);
}

TEST_F(TransformSystemFixture, Compose3DScale) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform3D>;
    stagehand::transform::Transform3D result;
    Traits::compose_transform(result, stagehand::transform::Position3D(godot::Vector3(0, 0, 0)), stagehand::transform::Rotation3D(godot::Quaternion()),
                              stagehand::transform::Scale3D(godot::Vector3(2, 3, 4)));

    godot::Vector3 scale = result.basis.get_scale();
    ASSERT_NEAR(scale.x, 2.0f, EPSILON);
    ASSERT_NEAR(scale.y, 3.0f, EPSILON);
    ASSERT_NEAR(scale.z, 4.0f, EPSILON);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 2D Decompose→Compose roundtrip via ECS system (runs actual systems)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TransformSystemFixture, ComposeSystemRunsAndUpdatesTransform2D) {
    stagehand::entity entity = world.entity();
    entity.set<stagehand::transform::Position2D>(stagehand::transform::Position2D(godot::Vector2(100, 200)));
    entity.set<stagehand::transform::Rotation2D>(stagehand::transform::Rotation2D(0.5f));
    entity.set<stagehand::transform::Scale2D>(stagehand::transform::Scale2D(godot::Vector2(2, 3)));
    // Add Transform2D without triggering change detection — bypass the entity wrapper
    // to avoid firing the decompose system (which would overwrite position/rotation/scale)
    static_cast<flecs::entity>(entity).set<stagehand::transform::Transform2D>(stagehand::transform::Transform2D());

    // Verify all components are present
    ASSERT_TRUE(entity.has<stagehand::transform::Position2D>());
    ASSERT_TRUE(entity.has<stagehand::transform::Rotation2D>());
    ASSERT_TRUE(entity.has<stagehand::transform::Scale2D>());
    ASSERT_TRUE(entity.has<stagehand::transform::Transform2D>());

    // Verify change tags are present and enabled
    ASSERT_TRUE(entity.has<stagehand::transform::HasChangedPosition2D>()) << "HasChangedPosition2D should be present";
    ASSERT_TRUE(entity.has<stagehand::transform::HasChangedRotation2D>()) << "HasChangedRotation2D should be present";
    ASSERT_TRUE(entity.has<stagehand::transform::HasChangedScale2D>()) << "HasChangedScale2D should be present";
    ASSERT_TRUE(entity.has<stagehand::transform::HasChangedTransform2D>()) << "HasChangedTransform2D should be present";

    ASSERT_TRUE(static_cast<flecs::entity>(entity).enabled<stagehand::transform::HasChangedPosition2D>()) << "HasChangedPosition2D should be enabled";
    ASSERT_TRUE(static_cast<flecs::entity>(entity).enabled<stagehand::transform::HasChangedTransform2D>()) << "HasChangedTransform2D should be enabled";

    // Verify the compose system exists
    flecs::entity compose_sys = world.lookup(stagehand::names::systems::TRANSFORM_COMPOSE_2D);
    ASSERT_TRUE(compose_sys.is_valid()) << "Compose 2D system should exist";

    // Try running the compose system directly via C API
    ecs_run(world.c_ptr(), compose_sys.id(), 0.0f, nullptr);

    // Check if compose system ran
    const stagehand::transform::Transform2D *t1 = entity.try_get<stagehand::transform::Transform2D>();
    ASSERT_NE(t1, nullptr);
    ASSERT_NEAR(t1->get_origin().x, 100.0f, EPSILON) << "After direct run, transform origin.x should be 100";
    ASSERT_NEAR(t1->get_origin().y, 200.0f, EPSILON);
    ASSERT_NEAR(t1->get_rotation(), 0.5f, EPSILON);
}

TEST_F(TransformSystemFixture, DecomposeSystemRunsAndExtractsComponents3D) {
    stagehand::entity entity = world.entity();
    entity.set<stagehand::transform::Position3D>(stagehand::transform::Position3D());
    entity.set<stagehand::transform::Rotation3D>(stagehand::transform::Rotation3D());
    entity.set<stagehand::transform::Scale3D>(stagehand::transform::Scale3D());

    godot::Transform3D input_transform = godot::Transform3D(godot::Basis().scaled(godot::Vector3(2, 3, 4)), godot::Vector3(10, 20, 30));
    entity.set<stagehand::transform::Transform3D>(stagehand::transform::Transform3D(input_transform));

    world.progress(0.016f);

    const stagehand::transform::Position3D *pos = entity.try_get<stagehand::transform::Position3D>();
    ASSERT_NE(pos, nullptr);
    ASSERT_NEAR(pos->x, 10.0f, EPSILON);
    ASSERT_NEAR(pos->y, 20.0f, EPSILON);
    ASSERT_NEAR(pos->z, 30.0f, EPSILON);

    const stagehand::transform::Scale3D *scale = entity.try_get<stagehand::transform::Scale3D>();
    ASSERT_NE(scale, nullptr);
    ASSERT_NEAR(scale->x, 2.0f, EPSILON);
    ASSERT_NEAR(scale->y, 3.0f, EPSILON);
    ASSERT_NEAR(scale->z, 4.0f, EPSILON);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Transform systems are registered by name
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TransformSystemFixture, TransformSystemsAreRegisteredByName) {
    ASSERT_TRUE(world.lookup(stagehand::names::systems::TRANSFORM_COMPOSE_2D).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::systems::TRANSFORM_DECOMPOSE_2D).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::systems::TRANSFORM_COMPOSE_3D).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::systems::TRANSFORM_DECOMPOSE_3D).is_valid());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Transform compose only runs when change detection fires
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TransformSystemFixture, ComposeDoesNotRunWithoutChangeTag) {
    stagehand::entity entity = world.entity();
    entity.set<stagehand::transform::Position2D>(stagehand::transform::Position2D(godot::Vector2(50, 60)));
    entity.set<stagehand::transform::Rotation2D>(stagehand::transform::Rotation2D(0.0f));
    entity.set<stagehand::transform::Scale2D>(stagehand::transform::Scale2D(godot::Vector2(1, 1)));
    // Add Transform2D without triggering decompose change detection
    static_cast<flecs::entity>(entity).set<stagehand::transform::Transform2D>(stagehand::transform::Transform2D());

    flecs::entity compose_sys = world.lookup(stagehand::names::systems::TRANSFORM_COMPOSE_2D);
    ASSERT_TRUE(compose_sys.is_valid());

    // First direct run: compose runs because change tags are set
    ecs_run(world.c_ptr(), compose_sys.id(), 0.0f, nullptr);
    ASSERT_NEAR(entity.try_get<stagehand::transform::Transform2D>()->get_origin().x, 50.0f, EPSILON);

    // Manually disable change tags (simulate what the tag reset system does after PostRender)
    static_cast<flecs::entity>(entity).disable<stagehand::transform::HasChangedPosition2D>();
    static_cast<flecs::entity>(entity).disable<stagehand::transform::HasChangedRotation2D>();
    static_cast<flecs::entity>(entity).disable<stagehand::transform::HasChangedScale2D>();

    // Modify position directly without using the entity wrapper (bypass change detection)
    entity.ensure<stagehand::transform::Position2D>() = stagehand::transform::Position2D(godot::Vector2(999, 999));
    entity.modified<stagehand::transform::Position2D>();

    // Run compose again — no change tags are enabled, so compose should NOT update
    ecs_run(world.c_ptr(), compose_sys.id(), 0.0f, nullptr);

    // Transform should still show the old composed value (50, 60)
    const stagehand::transform::Transform2D *t = entity.try_get<stagehand::transform::Transform2D>();
    ASSERT_NE(t, nullptr);
    ASSERT_NEAR(t->get_origin().x, 50.0f, EPSILON) << "Compose should not run without change tags";
}
