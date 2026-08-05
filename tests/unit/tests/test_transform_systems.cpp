/// Unit tests for transform system traits — compose and decompose.
/// These test the pure math operations used by the transform systems
/// without requiring a running Godot engine.

#include <flecs.h>
#include <gtest/gtest.h>
#include <numbers>

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
    float angle = std::numbers::pi_v<float> / 4.0f; // 45 degrees
    Traits::compose_transform(result, stagehand::transform::Position2D(godot::Vector2(0, 0)), stagehand::transform::Rotation2D(angle),
                              stagehand::transform::Scale2D(godot::Vector2(1, 1)));

    ASSERT_NEAR(result.get_rotation(), angle, EPSILON);
}

TEST_F(TransformSystemFixture, Compose2DCombinedPRS) {
    using Traits = stagehand::transform::TransformSystemTraits<stagehand::transform::Transform2D>;
    stagehand::transform::Transform2D result;
    float angle = std::numbers::pi_v<float> / 2.0f;
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
// 3D Decompose system test
// ═══════════════════════════════════════════════════════════════════════════════

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
