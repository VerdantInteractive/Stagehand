#include <array>
#include <cstdint>
#include <flecs.h>
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/traits.h"
#include "stagehand/entity.h"
#include "stagehand/registry.h"

namespace test_no_change_detection {
    FLOAT_(NoTrackFloat, 1.5f);
    UINT16_(NoTrackUint16, 123);
    INT8_(NoTrackInt8, -2);

    struct PointerTarget {
        int value = 0;
    };
    POINTER_(NoTrackPointer, PointerTarget);

    VECTOR_(NoTrackVectorInt, int, {1, 2, 3});
    ARRAY_(NoTrackArrayFloat, float, 3, {1.0f, 2.0f, 3.0f});

    enum class NoTrackEnum : uint16_t {
        A = 1,
        B = 2,
    };
    ENUM_(NoTrackEnum, uint16_t);

    GODOT_VARIANT_(NoTrackVector3, godot::Vector3, 1.0f, 2.0f, 3.0f);

    struct NoTrackStruct {
        int hp = 12;
        float speed = 1.25f;
    };

    STRUCT_(NoTrackStruct);

    struct TrackedStruct {
        int hp = 20;
        float speed = 2.5f;
    };

    FLOAT(TrackedFloat, 0.0f);
    GODOT_VARIANT(TrackedVector3, godot::Vector3);
    STRUCT(TrackedStruct);

    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackFloat>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackUint16>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackInt8>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackPointer>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackVectorInt>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackArrayFloat>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackVector3>);
    static_assert(!stagehand::internal::component_has_change_tag_v<NoTrackStruct>);

    static_assert(stagehand::internal::component_has_change_tag_v<TrackedFloat>);
    static_assert(stagehand::internal::component_has_change_tag_v<TrackedVector3>);
    static_assert(stagehand::internal::component_has_change_tag_v<TrackedStruct>);
} // namespace test_no_change_detection

namespace {
    struct NoChangeDetectionFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };

    bool component_has_change_detection_relation(flecs::entity component_entity) {
        bool has_change_detection = false;
        component_entity.each(flecs::With, [&has_change_detection](flecs::entity target) {
            if (target.has<stagehand::IsChangeDetectionTag>()) {
                has_change_detection = true;
            }
        });
        return has_change_detection;
    }

    bool entity_has_any_change_detection_tag(flecs::entity entity) {
        bool has_change_detection = false;
        entity.each([&has_change_detection](flecs::id id) {
            if (id.is_pair()) {
                return;
            }

            flecs::entity component_entity = id.entity();
            if (component_entity.is_valid() && component_entity.has<stagehand::IsChangeDetectionTag>()) {
                has_change_detection = true;
            }
        });
        return has_change_detection;
    }
} // namespace

TEST_F(NoChangeDetectionFixture, OptOutComponentsDoNotRegisterChangeDetectionRelations) {
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackFloat>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackUint16>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackInt8>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackPointer>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackVectorInt>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackArrayFloat>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackVector3>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackEnum>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_no_change_detection::NoTrackStruct>()));
}

TEST_F(NoChangeDetectionFixture, TrackedComponentsStillRegisterChangeDetectionRelations) {
    ASSERT_TRUE(component_has_change_detection_relation(world.component<test_no_change_detection::TrackedFloat>()));
    ASSERT_TRUE(component_has_change_detection_relation(world.component<test_no_change_detection::TrackedVector3>()));
    ASSERT_TRUE(component_has_change_detection_relation(world.component<test_no_change_detection::TrackedStruct>()));
}

TEST_F(NoChangeDetectionFixture, StagehandEntitySetDoesNotEnableChangeTagForOptOutComponents) {
    stagehand::entity no_track_entity = world.entity();
    no_track_entity.set<test_no_change_detection::NoTrackFloat>({42.0f});
    ASSERT_FALSE(entity_has_any_change_detection_tag(no_track_entity));

    stagehand::entity tracked_entity = world.entity();
    tracked_entity.set<test_no_change_detection::TrackedFloat>({42.0f});
    ASSERT_TRUE(entity_has_any_change_detection_tag(tracked_entity));
}

TEST_F(NoChangeDetectionFixture, ShiftOperatorRespectsOptOutBehavior) {
    stagehand::entity no_track_entity = world.entity();
    no_track_entity << test_no_change_detection::NoTrackVector3(godot::Vector3(3.0f, 4.0f, 5.0f));
    ASSERT_FALSE(entity_has_any_change_detection_tag(no_track_entity));

    stagehand::entity tracked_entity = world.entity();
    tracked_entity << test_no_change_detection::TrackedVector3(godot::Vector3(3.0f, 4.0f, 5.0f));
    ASSERT_TRUE(entity_has_any_change_detection_tag(tracked_entity));
}

TEST_F(NoChangeDetectionFixture, StructShiftOperatorRespectsOptOutBehavior) {
    stagehand::entity no_track_entity = world.entity();
    no_track_entity << test_no_change_detection::NoTrackStruct{33, 3.0f};
    ASSERT_FALSE(entity_has_any_change_detection_tag(no_track_entity));

    stagehand::entity tracked_entity = world.entity();
    tracked_entity << test_no_change_detection::TrackedStruct{44, 4.0f};
    ASSERT_TRUE(entity_has_any_change_detection_tag(tracked_entity));
}

TEST_F(NoChangeDetectionFixture, OptOutComponentsStillRegisterGetterSetterFunctions) {
    std::unordered_map<std::string, stagehand::ComponentFunctions> &registry = stagehand::get_component_registry();

    for (const std::string &component_name : {"NoTrackFloat", "NoTrackUint16", "NoTrackInt8", "NoTrackPointer", "NoTrackVectorInt", "NoTrackArrayFloat",
                                              "NoTrackVector3", "NoTrackEnum", "NoTrackStruct"}) {
        auto it = registry.find(component_name);
        ASSERT_NE(it, registry.end()) << component_name;
        ASSERT_TRUE(static_cast<bool>(it->second.getter)) << component_name;
        ASSERT_TRUE(static_cast<bool>(it->second.setter)) << component_name;
    }
}

TEST_F(NoChangeDetectionFixture, OptOutContainersAndPointersRoundtripCorrectly) {
    stagehand::entity entity = world.entity();

    entity.set<test_no_change_detection::NoTrackVectorInt>({std::vector<int>{8, 9, 10}});
    entity.set<test_no_change_detection::NoTrackArrayFloat>({std::array<float, 3>{4.0f, 5.0f, 6.0f}});

    test_no_change_detection::PointerTarget pointer_target;
    pointer_target.value = 77;
    entity.set<test_no_change_detection::NoTrackPointer>({&pointer_target});

    const test_no_change_detection::NoTrackVectorInt *vector_component = entity.try_get<test_no_change_detection::NoTrackVectorInt>();
    const test_no_change_detection::NoTrackArrayFloat *array_component = entity.try_get<test_no_change_detection::NoTrackArrayFloat>();
    const test_no_change_detection::NoTrackPointer *pointer_component = entity.try_get<test_no_change_detection::NoTrackPointer>();

    ASSERT_NE(vector_component, nullptr);
    ASSERT_NE(array_component, nullptr);
    ASSERT_NE(pointer_component, nullptr);

    ASSERT_EQ(vector_component->value.size(), 3u);
    ASSERT_EQ(vector_component->value[0], 8);
    ASSERT_EQ(vector_component->value[2], 10);

    ASSERT_EQ((*array_component)[0], 4.0f);
    ASSERT_EQ((*array_component)[1], 5.0f);
    ASSERT_EQ((*array_component)[2], 6.0f);

    ASSERT_EQ(pointer_component->ptr, &pointer_target);
    ASSERT_EQ(pointer_component->ptr->value, 77);
    ASSERT_FALSE(entity_has_any_change_detection_tag(entity));
}
