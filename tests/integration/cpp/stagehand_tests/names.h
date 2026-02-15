#pragma once

namespace stagehand_tests::names {

#define NAMESPACE_STR "stagehand_tests"

    constexpr const char *NAMESPACE = NAMESPACE_STR;

    namespace prefabs {
        constexpr const char *TEST_ENTITY_2D = NAMESPACE_STR "::TestEntity2D";
        constexpr const char *TEST_ENTITY_3D = NAMESPACE_STR "::TestEntity3D";
        constexpr const char *RENDERED_ENTITY_2D = NAMESPACE_STR "::RenderedEntity2D";
        constexpr const char *RENDERED_ENTITY_3D = NAMESPACE_STR "::RenderedEntity3D";
        constexpr const char *COLORED_ENTITY_2D = NAMESPACE_STR "::ColoredEntity2D";
        constexpr const char *COLORED_ENTITY_3D = NAMESPACE_STR "::ColoredEntity3D";
        constexpr const char *CUSTOM_DATA_ENTITY_2D = NAMESPACE_STR "::CustomDataEntity2D";
        constexpr const char *CUSTOM_DATA_ENTITY_3D = NAMESPACE_STR "::CustomDataEntity3D";
        constexpr const char *FULL_ENTITY_2D = NAMESPACE_STR "::FullEntity2D";
        constexpr const char *FULL_ENTITY_3D = NAMESPACE_STR "::FullEntity3D";
    } // namespace prefabs

    namespace systems {
        constexpr const char *TICK_COUNTER = NAMESPACE_STR "::Tick Counter";
        constexpr const char *EMIT_TEST_SIGNAL = NAMESPACE_STR "::Emit Test Signal";
        constexpr const char *READ_SCENE_CHILDREN = NAMESPACE_STR "::Read Scene Children";
        constexpr const char *ACCUMULATOR = NAMESPACE_STR "::Accumulator";
        constexpr const char *SUM_QUERY = NAMESPACE_STR "::Sum Query";
        constexpr const char *TOGGLE_TAG = NAMESPACE_STR "::Toggle Tag";
        constexpr const char *COUNT_RENDERED_ENTITIES = NAMESPACE_STR "::Count Rendered Entities";
        constexpr const char *QUERY_ENTITY_TRANSFORMS = NAMESPACE_STR "::Query Entity Transforms";
        constexpr const char *LOOKUP_ENTITIES = NAMESPACE_STR "::Lookup Entities";
    } // namespace systems

#undef NAMESPACE_STR

} // namespace stagehand_tests::names
