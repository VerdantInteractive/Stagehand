/// Unit tests for the STRUCT and STRUCT_ component macros.
/// Tests verify:
///   1. Struct definition, default values, and aggregate initialization.
///   2. Flecs component and member registration via PFR reflection.
///   3. Getter/setter and data_type registration for GDScript integration.
///   4. Change detection (STRUCT) and no change detection (STRUCT_).
///   5. ComponentRegistrar::then() chaining.

#include <flecs.h>
#include <gtest/gtest.h>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/traits.h"
#include "stagehand/registry.h"

namespace test_struct {
    STRUCT_(SimpleStruct, {
        float x = 1.0f;
        float y = 2.0f;
    });

    STRUCT_(DefaultValues, {
        float damage_cooldown = 0.3f;
        int player_hit_radius = 5;
        double precision = 0.001;
    });

    STRUCT_(SingleField, { int32_t count = 42; });

    STRUCT(TrackedStruct, {
        float speed = 10.0f;
        int32_t health = 100;
    });

    STRUCT_(AsSingleton, {
        float global_speed = 1.0f;
        int32_t max_entities = 1000;
    }).then([](auto component) { component.add(flecs::Singleton); });
} // namespace test_struct

// ═══════════════════════════════════════════════════════════════════════════════
// Fixture
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct StructMacroFixture : ::testing::Test {
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
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Default values
// ═══════════════════════════════════════════════════════════════════════════════

TEST(StructMacroValues, DefaultValuesAreApplied) {
    test_struct::SimpleStruct s;
    ASSERT_NEAR(s.x, 1.0f, 1e-9f);
    ASSERT_NEAR(s.y, 2.0f, 1e-9f);
}

TEST(StructMacroValues, MultiFieldDefaultValues) {
    test_struct::DefaultValues d;
    ASSERT_NEAR(d.damage_cooldown, 0.3f, 1e-6f);
    ASSERT_EQ(d.player_hit_radius, 5);
    ASSERT_NEAR(d.precision, 0.001, 1e-9);
}

TEST(StructMacroValues, AggregateInitialization) {
    test_struct::SimpleStruct s{10.0f, 20.0f};
    ASSERT_NEAR(s.x, 10.0f, 1e-9f);
    ASSERT_NEAR(s.y, 20.0f, 1e-9f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Flecs component registration
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(StructMacroFixture, ComponentIsRegisteredInFlecs) {
    flecs::component<test_struct::SimpleStruct> comp = world.component<test_struct::SimpleStruct>();
    ASSERT_NE(comp.id(), 0u);
}

TEST_F(StructMacroFixture, ComponentHasCorrectMemberCount) {
    flecs::entity comp = world.component<test_struct::DefaultValues>();

    // Verify member registration using the Flecs meta API
    const ecs_member_t *m0 = ecs_struct_get_member(world.c_ptr(), comp.id(), "damage_cooldown");
    const ecs_member_t *m1 = ecs_struct_get_member(world.c_ptr(), comp.id(), "player_hit_radius");
    const ecs_member_t *m2 = ecs_struct_get_member(world.c_ptr(), comp.id(), "precision");

    ASSERT_NE(m0, nullptr) << "member 'damage_cooldown' should be registered";
    ASSERT_NE(m1, nullptr) << "member 'player_hit_radius' should be registered";
    ASSERT_NE(m2, nullptr) << "member 'precision' should be registered";
}

TEST_F(StructMacroFixture, MemberNamesAreCorrect) {
    flecs::entity comp = world.component<test_struct::SimpleStruct>();
    const ecs_member_t *mx = ecs_struct_get_member(world.c_ptr(), comp.id(), "x");
    const ecs_member_t *my = ecs_struct_get_member(world.c_ptr(), comp.id(), "y");
    ASSERT_NE(mx, nullptr) << "member 'x' should be registered";
    ASSERT_NE(my, nullptr) << "member 'y' should be registered";
    ASSERT_EQ(mx->type, flecs::F32) << "member 'x' should be float";
    ASSERT_EQ(my->type, flecs::F32) << "member 'y' should be float";
}

TEST_F(StructMacroFixture, ThreeFieldMemberNamesAreCorrect) {
    flecs::entity comp = world.component<test_struct::DefaultValues>();
    const ecs_member_t *m0 = ecs_struct_get_member(world.c_ptr(), comp.id(), "damage_cooldown");
    const ecs_member_t *m1 = ecs_struct_get_member(world.c_ptr(), comp.id(), "player_hit_radius");
    const ecs_member_t *m2 = ecs_struct_get_member(world.c_ptr(), comp.id(), "precision");
    ASSERT_NE(m0, nullptr);
    ASSERT_NE(m1, nullptr);
    ASSERT_NE(m2, nullptr);
    ASSERT_EQ(m0->type, flecs::F32);
    ASSERT_EQ(m1->type, flecs::I32);
    ASSERT_EQ(m2->type, flecs::F64);
}

TEST_F(StructMacroFixture, SingleFieldMemberNameIsCorrect) {
    flecs::entity comp = world.component<test_struct::SingleField>();
    const ecs_member_t *m = ecs_struct_get_member(world.c_ptr(), comp.id(), "count");
    ASSERT_NE(m, nullptr) << "member 'count' should be registered";
    ASSERT_EQ(m->type, flecs::I32);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Component registry (getter/setter)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(StructMacroFixture, GetterIsRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("SimpleStruct");
    ASSERT_NE(it, registry.end());
    ASSERT_TRUE(static_cast<bool>(it->second.getter));
}

TEST_F(StructMacroFixture, SetterIsRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("SimpleStruct");
    ASSERT_NE(it, registry.end());
    ASSERT_TRUE(static_cast<bool>(it->second.setter));
}

TEST_F(StructMacroFixture, DataTypeIsStruct) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("SimpleStruct");
    ASSERT_NE(it, registry.end());
    ASSERT_EQ(it->second.data_type, "struct");
}

TEST_F(StructMacroFixture, EntityIdIsRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("DefaultValues");
    ASSERT_NE(it, registry.end());
    ASSERT_NE(it->second.entity_id, 0u);
    ASSERT_EQ(it->second.entity_id, world.component<test_struct::DefaultValues>().id());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Entity roundtrip
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(StructMacroFixture, EntityRoundtripSetAndGet) {
    flecs::entity e = world.entity().set<test_struct::SimpleStruct>({5.0f, 10.0f});
    const test_struct::SimpleStruct *data = e.try_get<test_struct::SimpleStruct>();
    ASSERT_NE(data, nullptr);
    ASSERT_NEAR(data->x, 5.0f, 1e-9f);
    ASSERT_NEAR(data->y, 10.0f, 1e-9f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Change detection
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(StructMacroFixture, StructWithoutChangeDetectionDoesNotRegisterRelation) {
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_struct::SimpleStruct>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_struct::DefaultValues>()));
    ASSERT_FALSE(component_has_change_detection_relation(world.component<test_struct::SingleField>()));
}

TEST_F(StructMacroFixture, StructWithChangeDetectionRegistersRelation) {
    ASSERT_TRUE(component_has_change_detection_relation(world.component<test_struct::TrackedStruct>()));
}

TEST_F(StructMacroFixture, HasChangedTagExistsForTrackedStruct) {
    flecs::component<test_struct::HasChangedTrackedStruct> tag = world.component<test_struct::HasChangedTrackedStruct>();
    ASSERT_NE(tag.id(), 0u);
    ASSERT_TRUE(tag.has<stagehand::IsChangeDetectionTag>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// .then() chaining
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(StructMacroFixture, ThenChainingAppliesSingleton) {
    // AsSingleton was registered with .then([](auto c) { c.add(flecs::Singleton); })
    // After setting the singleton, we should be able to query it at entity_id 0
    world.set<test_struct::AsSingleton>({5.0f, 100});
    const test_struct::AsSingleton *data = world.try_get<test_struct::AsSingleton>();
    ASSERT_NE(data, nullptr);
    ASSERT_NEAR(data->global_speed, 5.0f, 1e-6f);
    ASSERT_EQ(data->max_entities, 100);
}
