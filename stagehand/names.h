#pragma once

namespace stagehand::names {

#define NAMESPACE_STR "stagehand"

    constexpr const char *NAMESPACE = NAMESPACE_STR;

    namespace phases {
        constexpr const char *ON_EARLY_UPDATE = NAMESPACE_STR "::OnEarlyUpdate";
        constexpr const char *ON_LATE_UPDATE = NAMESPACE_STR "::OnLateUpdate";
        constexpr const char *PRE_RENDER = NAMESPACE_STR "::PreRender";
        constexpr const char *ON_RENDER = NAMESPACE_STR "::OnRender";
        constexpr const char *POST_RENDER = NAMESPACE_STR "::PostRender";
    } // namespace phases

    namespace systems {
        constexpr const char *ENTITY_RENDERING_COMPUTE = NAMESPACE_STR "::rendering::Entity Rendering (Compute)";
        constexpr const char *ENTITY_RENDERING_INSTANCED = NAMESPACE_STR "::rendering::Entity Rendering (Instanced)";
        constexpr const char *ENTITY_RENDERING_MULTIMESH = NAMESPACE_STR "::rendering::Entity Rendering (MultiMesh)";
        constexpr const char *PHYSICS_BODY_SPACE_ASSIGNMENT_2D = NAMESPACE_STR "::physics::Body Space Assignment (2D)";
        constexpr const char *PHYSICS_BODY_SPACE_ASSIGNMENT_3D = NAMESPACE_STR "::physics::Body Space Assignment (3D)";
        constexpr const char *PHYSICS_FEEDBACK_ANGULAR_VELOCITY_2D = NAMESPACE_STR "::physics::Feedback Angular Velocity (2D)";
        constexpr const char *PHYSICS_FEEDBACK_ANGULAR_VELOCITY_3D = NAMESPACE_STR "::physics::Feedback Angular Velocity (3D)";
        constexpr const char *PHYSICS_FEEDBACK_TRANSFORM_2D = NAMESPACE_STR "::physics::Feedback Transform (2D)";
        constexpr const char *PHYSICS_FEEDBACK_TRANSFORM_3D = NAMESPACE_STR "::physics::Feedback Transform (3D)";
        constexpr const char *PHYSICS_FEEDBACK_VELOCITY_2D = NAMESPACE_STR "::physics::Feedback Velocity (2D)";
        constexpr const char *PHYSICS_FEEDBACK_VELOCITY_3D = NAMESPACE_STR "::physics::Feedback Velocity (3D)";
        constexpr const char *PHYSICS_SYNC_ANGULAR_VELOCITY_2D = NAMESPACE_STR "::physics::Sync Angular Velocity (2D)";
        constexpr const char *PHYSICS_SYNC_ANGULAR_VELOCITY_3D = NAMESPACE_STR "::physics::Sync Angular Velocity (3D)";
        constexpr const char *PHYSICS_SYNC_COLLISION_2D = NAMESPACE_STR "::physics::Sync Collision (2D)";
        constexpr const char *PHYSICS_SYNC_COLLISION_3D = NAMESPACE_STR "::physics::Sync Collision (3D)";
        constexpr const char *PHYSICS_SYNC_TRANSFORM_2D = NAMESPACE_STR "::physics::Sync Transform (2D)";
        constexpr const char *PHYSICS_SYNC_TRANSFORM_3D = NAMESPACE_STR "::physics::Sync Transform (3D)";
        constexpr const char *PHYSICS_SYNC_VELOCITY_2D = NAMESPACE_STR "::physics::Sync Velocity (2D)";
        constexpr const char *PHYSICS_SYNC_VELOCITY_3D = NAMESPACE_STR "::physics::Sync Velocity (3D)";
        constexpr const char *PREFAB_INSTANTIATION = NAMESPACE_STR "::Prefab Instantiation";
        constexpr const char *TAG_RESET_CHANGE_DETECTION = NAMESPACE_STR "::Tag Reset (Change Detection)";
        constexpr const char *TRANSFORM_COMPOSE_2D = NAMESPACE_STR "::transform::Transform Compose (2D)";
        constexpr const char *TRANSFORM_COMPOSE_3D = NAMESPACE_STR "::transform::Transform Compose (3D)";
        constexpr const char *TRANSFORM_DECOMPOSE_2D = NAMESPACE_STR "::transform::Transform Decompose (2D)";
        constexpr const char *TRANSFORM_DECOMPOSE_3D = NAMESPACE_STR "::transform::Transform Decompose (3D)";
    } // namespace systems

    namespace prefabs {
        constexpr const char *ENTITY_2D = NAMESPACE_STR "::Entity2D";
        constexpr const char *ENTITY_3D = NAMESPACE_STR "::Entity3D";

        constexpr const char *INSTANCE = NAMESPACE_STR "::Instance";
        constexpr const char *INSTANCE_2D = NAMESPACE_STR "::Instance2D";
        constexpr const char *INSTANCE_3D = NAMESPACE_STR "::Instance3D";

        constexpr const char *SEGMENT = NAMESPACE_STR "::Segment";
        constexpr const char *SEGMENT_2D = NAMESPACE_STR "::Segment2D";
        constexpr const char *SEGMENT_3D = NAMESPACE_STR "::Segment3D";
    } // namespace prefabs

#undef NAMESPACE_STR

} // namespace stagehand::names
