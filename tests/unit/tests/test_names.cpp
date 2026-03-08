/// Unit tests for stagehand::names constants.
/// Verifies that all name constants are:
///   1. Non-null and non-empty.
///   2. Properly prefixed with the stagehand namespace.

#include <cstring>
#include <gtest/gtest.h>
#include <string>

#include "stagehand/names.h"

namespace {
    void assert_has_prefix(const char *name, const char *prefix, const char *label) {
        ASSERT_NE(name, nullptr) << label << " is null";
        ASSERT_GT(std::strlen(name), 0u) << label << " is empty";
        ASSERT_EQ(std::string(name).rfind(prefix, 0), 0u) << label << " does not start with " << prefix;
    }
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Namespace constant
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Names, NamespaceIsStagehand) { ASSERT_STREQ(stagehand::names::NAMESPACE, "stagehand"); }

// ═══════════════════════════════════════════════════════════════════════════════
// Phase names
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Names, PhaseNamesArePrefixed) {
    assert_has_prefix(stagehand::names::phases::ON_EARLY_UPDATE, "stagehand::", "ON_EARLY_UPDATE");
    assert_has_prefix(stagehand::names::phases::ON_LATE_UPDATE, "stagehand::", "ON_LATE_UPDATE");
    assert_has_prefix(stagehand::names::phases::PRE_RENDER, "stagehand::", "PRE_RENDER");
    assert_has_prefix(stagehand::names::phases::ON_RENDER, "stagehand::", "ON_RENDER");
    assert_has_prefix(stagehand::names::phases::POST_RENDER, "stagehand::", "POST_RENDER");
}

// ═══════════════════════════════════════════════════════════════════════════════
// System names
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Names, SystemNamesArePrefixed) {
    assert_has_prefix(stagehand::names::systems::ENTITY_RENDERING_COMPUTE, "stagehand::", "ENTITY_RENDERING_COMPUTE");
    assert_has_prefix(stagehand::names::systems::ENTITY_RENDERING_INSTANCED, "stagehand::", "ENTITY_RENDERING_INSTANCED");
    assert_has_prefix(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH, "stagehand::", "ENTITY_RENDERING_MULTIMESH");
    assert_has_prefix(stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_2D, "stagehand::", "PHYSICS_BODY_SPACE_ASSIGNMENT_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_3D, "stagehand::", "PHYSICS_BODY_SPACE_ASSIGNMENT_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_2D, "stagehand::", "PHYSICS_FEEDBACK_ANG_VEL_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_3D, "stagehand::", "PHYSICS_FEEDBACK_ANG_VEL_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_2D, "stagehand::", "PHYSICS_FEEDBACK_TRANSFORM_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_3D, "stagehand::", "PHYSICS_FEEDBACK_TRANSFORM_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_2D, "stagehand::", "PHYSICS_FEEDBACK_VELOCITY_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_3D, "stagehand::", "PHYSICS_FEEDBACK_VELOCITY_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_2D, "stagehand::", "PHYSICS_SYNC_ANG_VEL_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_3D, "stagehand::", "PHYSICS_SYNC_ANG_VEL_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_COLLISION_2D, "stagehand::", "PHYSICS_SYNC_COLLISION_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_COLLISION_3D, "stagehand::", "PHYSICS_SYNC_COLLISION_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_2D, "stagehand::", "PHYSICS_SYNC_TRANSFORM_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_3D, "stagehand::", "PHYSICS_SYNC_TRANSFORM_3D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_VELOCITY_2D, "stagehand::", "PHYSICS_SYNC_VELOCITY_2D");
    assert_has_prefix(stagehand::names::systems::PHYSICS_SYNC_VELOCITY_3D, "stagehand::", "PHYSICS_SYNC_VELOCITY_3D");
    assert_has_prefix(stagehand::names::systems::TAG_RESET_CHANGE_DETECTION, "stagehand::", "TAG_RESET_CHANGE_DETECTION");
    assert_has_prefix(stagehand::names::systems::TRANSFORM_COMPOSE_2D, "stagehand::", "TRANSFORM_COMPOSE_2D");
    assert_has_prefix(stagehand::names::systems::TRANSFORM_COMPOSE_3D, "stagehand::", "TRANSFORM_COMPOSE_3D");
    assert_has_prefix(stagehand::names::systems::TRANSFORM_DECOMPOSE_2D, "stagehand::", "TRANSFORM_DECOMPOSE_2D");
    assert_has_prefix(stagehand::names::systems::TRANSFORM_DECOMPOSE_3D, "stagehand::", "TRANSFORM_DECOMPOSE_3D");
}

TEST(Names, AllSystemNamesAreUnique) {
    const std::vector<const char *> all_systems = {
        stagehand::names::systems::ENTITY_RENDERING_COMPUTE,
        stagehand::names::systems::ENTITY_RENDERING_INSTANCED,
        stagehand::names::systems::ENTITY_RENDERING_MULTIMESH,
        stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_2D,
        stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_3D,
        stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_2D,
        stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_3D,
        stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_2D,
        stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_3D,
        stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_2D,
        stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_3D,
        stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_2D,
        stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_3D,
        stagehand::names::systems::PHYSICS_SYNC_COLLISION_2D,
        stagehand::names::systems::PHYSICS_SYNC_COLLISION_3D,
        stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_2D,
        stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_3D,
        stagehand::names::systems::PHYSICS_SYNC_VELOCITY_2D,
        stagehand::names::systems::PHYSICS_SYNC_VELOCITY_3D,
        stagehand::names::systems::TAG_RESET_CHANGE_DETECTION,
        stagehand::names::systems::TRANSFORM_COMPOSE_2D,
        stagehand::names::systems::TRANSFORM_COMPOSE_3D,
        stagehand::names::systems::TRANSFORM_DECOMPOSE_2D,
        stagehand::names::systems::TRANSFORM_DECOMPOSE_3D,
    };

    for (size_t i = 0; i < all_systems.size(); ++i) {
        for (size_t j = i + 1; j < all_systems.size(); ++j) {
            ASSERT_STRNE(all_systems[i], all_systems[j]) << "Duplicate system names at indices " << i << " and " << j;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Prefab names
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Names, PrefabNamesArePrefixed) {
    assert_has_prefix(stagehand::names::prefabs::ENTITY_2D, "stagehand::", "ENTITY_2D");
    assert_has_prefix(stagehand::names::prefabs::ENTITY_3D, "stagehand::", "ENTITY_3D");
    assert_has_prefix(stagehand::names::prefabs::INSTANCE, "stagehand::", "INSTANCE");
    assert_has_prefix(stagehand::names::prefabs::INSTANCE_2D, "stagehand::", "INSTANCE_2D");
    assert_has_prefix(stagehand::names::prefabs::INSTANCE_3D, "stagehand::", "INSTANCE_3D");
    assert_has_prefix(stagehand::names::prefabs::SEGMENT, "stagehand::", "SEGMENT");
    assert_has_prefix(stagehand::names::prefabs::SEGMENT_2D, "stagehand::", "SEGMENT_2D");
    assert_has_prefix(stagehand::names::prefabs::SEGMENT_3D, "stagehand::", "SEGMENT_3D");
}

TEST(Names, AllPrefabNamesAreUnique) {
    const std::vector<const char *> all_prefabs = {
        stagehand::names::prefabs::ENTITY_2D,   stagehand::names::prefabs::ENTITY_3D,   stagehand::names::prefabs::INSTANCE,
        stagehand::names::prefabs::INSTANCE_2D, stagehand::names::prefabs::INSTANCE_3D, stagehand::names::prefabs::SEGMENT,
        stagehand::names::prefabs::SEGMENT_2D,  stagehand::names::prefabs::SEGMENT_3D,
    };

    for (size_t i = 0; i < all_prefabs.size(); ++i) {
        for (size_t j = i + 1; j < all_prefabs.size(); ++j) {
            ASSERT_STRNE(all_prefabs[i], all_prefabs[j]) << "Duplicate prefab names at indices " << i << " and " << j;
        }
    }
}
