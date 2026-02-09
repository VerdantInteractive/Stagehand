#pragma once

namespace stagehand::names {

#define NAMESPACE_STR "stagehand"

    constexpr const char* NAMESPACE = NAMESPACE_STR;

    namespace phases {
        constexpr const char* ON_EARLY_UPDATE = NAMESPACE_STR "::OnEarlyUpdate";
        constexpr const char* ON_LATE_UPDATE = NAMESPACE_STR "::OnLateUpdate";
        constexpr const char* PRE_RENDER = NAMESPACE_STR "::PreRender";
        constexpr const char* ON_RENDER = NAMESPACE_STR "::OnRender";
        constexpr const char* POST_RENDER = NAMESPACE_STR "::PostRender";
    }

    namespace systems {
        constexpr const char* ENTITY_RENDERING_COMPUTE = NAMESPACE_STR "::entity_rendering::Entity Rendering (Compute)";
        constexpr const char* ENTITY_RENDERING_MULTIMESH = NAMESPACE_STR "::entity_rendering::Entity Rendering (MultiMesh)";
        constexpr const char* PREFAB_INSTANTIATION = NAMESPACE_STR "::Prefab Instantiation";
        constexpr const char* TRANSFORM_UPDATE_2D = NAMESPACE_STR "::Transform Update (2D)";
        constexpr const char* TRANSFORM_UPDATE_3D = NAMESPACE_STR "::Transform Update (3D)";
    }

    namespace prefabs {
        constexpr const char* ENTITY_2D = NAMESPACE_STR "::Entity2D";
        constexpr const char* ENTITY_3D = NAMESPACE_STR "::Entity3D";

        constexpr const char* INSTANCE = NAMESPACE_STR "::Instance";
        constexpr const char* INSTANCE_2D = NAMESPACE_STR "::Instance2D";
        constexpr const char* INSTANCE_3D = NAMESPACE_STR "::Instance3D";

        constexpr const char* SEGMENT = NAMESPACE_STR "::Segment";
        constexpr const char* SEGMENT_2D = NAMESPACE_STR "::Segment2D";
        constexpr const char* SEGMENT_3D = NAMESPACE_STR "::Segment3D";
    }

#undef NAMESPACE_STR

} // namespace stagehand::names
