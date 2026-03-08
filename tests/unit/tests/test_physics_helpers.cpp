/// Unit tests for physics helper functions and compile-time lookup tables.
/// These tests cover the pure logic that doesn't require a running Godot engine:
///   1. PhysicsBodyType enum values and constexpr lookup tables.
///   2. is_2d_body_type / is_dynamic_body_type classification.
///   3. Physics component struct layout and defaults.

#include <cstdint>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/physics.h"

using namespace stagehand::physics;

// ═══════════════════════════════════════════════════════════════════════════════
// Body type classification tables
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsHelpers, Static2DIs2D) { ASSERT_TRUE(is_2d_body_type(PhysicsBodyType::Static2D)); }

TEST(PhysicsHelpers, Kinematic2DIs2D) { ASSERT_TRUE(is_2d_body_type(PhysicsBodyType::Kinematic2D)); }

TEST(PhysicsHelpers, Rigid2DIs2D) { ASSERT_TRUE(is_2d_body_type(PhysicsBodyType::Rigid2D)); }

TEST(PhysicsHelpers, RigidLinear2DIs2D) { ASSERT_TRUE(is_2d_body_type(PhysicsBodyType::RigidLinear2D)); }

TEST(PhysicsHelpers, Static3DIsNot2D) { ASSERT_FALSE(is_2d_body_type(PhysicsBodyType::Static3D)); }

TEST(PhysicsHelpers, Kinematic3DIsNot2D) { ASSERT_FALSE(is_2d_body_type(PhysicsBodyType::Kinematic3D)); }

TEST(PhysicsHelpers, Rigid3DIsNot2D) { ASSERT_FALSE(is_2d_body_type(PhysicsBodyType::Rigid3D)); }

TEST(PhysicsHelpers, RigidLinear3DIsNot2D) { ASSERT_FALSE(is_2d_body_type(PhysicsBodyType::RigidLinear3D)); }

// ═══════════════════════════════════════════════════════════════════════════════
// Dynamic body type classification
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsHelpers, Static2DIsNotDynamic) { ASSERT_FALSE(is_dynamic_body_type(PhysicsBodyType::Static2D)); }

TEST(PhysicsHelpers, Kinematic2DIsNotDynamic) { ASSERT_FALSE(is_dynamic_body_type(PhysicsBodyType::Kinematic2D)); }

TEST(PhysicsHelpers, Rigid2DIsDynamic) { ASSERT_TRUE(is_dynamic_body_type(PhysicsBodyType::Rigid2D)); }

TEST(PhysicsHelpers, RigidLinear2DIsDynamic) { ASSERT_TRUE(is_dynamic_body_type(PhysicsBodyType::RigidLinear2D)); }

TEST(PhysicsHelpers, Static3DIsNotDynamic) { ASSERT_FALSE(is_dynamic_body_type(PhysicsBodyType::Static3D)); }

TEST(PhysicsHelpers, Kinematic3DIsNotDynamic) { ASSERT_FALSE(is_dynamic_body_type(PhysicsBodyType::Kinematic3D)); }

TEST(PhysicsHelpers, Rigid3DIsDynamic) { ASSERT_TRUE(is_dynamic_body_type(PhysicsBodyType::Rigid3D)); }

TEST(PhysicsHelpers, RigidLinear3DIsDynamic) { ASSERT_TRUE(is_dynamic_body_type(PhysicsBodyType::RigidLinear3D)); }

// ═══════════════════════════════════════════════════════════════════════════════
// Enum numeric values
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsHelpers, EnumValuesAreContiguous) {
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Static2D), 0);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Kinematic2D), 1);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Rigid2D), 2);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::RigidLinear2D), 3);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Static3D), 4);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Kinematic3D), 5);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::Rigid3D), 6);
    ASSERT_EQ(static_cast<uint8_t>(PhysicsBodyType::RigidLinear3D), 7);
}

TEST(PhysicsHelpers, BodyTypeCountIsCorrect) { ASSERT_EQ(PHYSICS_BODY_TYPE_COUNT, 8); }

// ═══════════════════════════════════════════════════════════════════════════════
// Lookup table consistency
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsHelpers, LookupTablesHaveCorrectSize) {
    ASSERT_EQ(body_type_is_2d.size(), PHYSICS_BODY_TYPE_COUNT);
    ASSERT_EQ(body_type_is_dynamic.size(), PHYSICS_BODY_TYPE_COUNT);
}

TEST(PhysicsHelpers, LookupTableAndFunctionConsistency) {
    for (uint8_t i = 0; i < PHYSICS_BODY_TYPE_COUNT; ++i) {
        PhysicsBodyType type = static_cast<PhysicsBodyType>(i);
        ASSERT_EQ(is_2d_body_type(type), body_type_is_2d[i]) << "Mismatch at index " << static_cast<int>(i);
        ASSERT_EQ(is_dynamic_body_type(type), body_type_is_dynamic[i]) << "Mismatch at index " << static_cast<int>(i);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 2D and 3D classification is complementary
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsHelpers, AllTypesAreEither2DOr3D) {
    // First 4 are 2D, last 4 are 3D
    for (uint8_t i = 0; i < 4; ++i) {
        ASSERT_TRUE(is_2d_body_type(static_cast<PhysicsBodyType>(i)));
    }
    for (uint8_t i = 4; i < 8; ++i) {
        ASSERT_FALSE(is_2d_body_type(static_cast<PhysicsBodyType>(i)));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Physics component defaults
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PhysicsComponentDefaults, CollisionLayerDefaultsToOne) {
    CollisionLayer layer;
    ASSERT_EQ(layer.value, 1u);
}

TEST(PhysicsComponentDefaults, CollisionMaskDefaultsToOne) {
    CollisionMask mask;
    ASSERT_EQ(mask.value, 1u);
}

TEST(PhysicsComponentDefaults, Velocity2DDefaultsToZero) {
    Velocity2D vel;
    ASSERT_FLOAT_EQ(vel.x, 0.0f);
    ASSERT_FLOAT_EQ(vel.y, 0.0f);
}

TEST(PhysicsComponentDefaults, AngularVelocity2DDefaultsToZero) {
    AngularVelocity2D vel;
    ASSERT_FLOAT_EQ(vel.value, 0.0f);
}

TEST(PhysicsComponentDefaults, Velocity3DDefaultsToZero) {
    Velocity3D vel;
    ASSERT_FLOAT_EQ(vel.x, 0.0f);
    ASSERT_FLOAT_EQ(vel.y, 0.0f);
    ASSERT_FLOAT_EQ(vel.z, 0.0f);
}

TEST(PhysicsComponentDefaults, AngularVelocity3DDefaultsToZero) {
    AngularVelocity3D vel;
    ASSERT_FLOAT_EQ(vel.x, 0.0f);
    ASSERT_FLOAT_EQ(vel.y, 0.0f);
    ASSERT_FLOAT_EQ(vel.z, 0.0f);
}
