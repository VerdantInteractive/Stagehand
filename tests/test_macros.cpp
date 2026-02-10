/// Unit tests for the component definition macros (FLOAT, DOUBLE, INT32, etc.).
/// Tests verify:
///   1. Struct layout, default values, conversions, and assignment.
///   2. Flecs component registration (component exists and has correct member metadata).
///   3. Entity-level roundtrips for all macro types.

#include <gtest/gtest.h>
#include <flecs.h>
#include "stagehand/registry.h"
#include "stagehand/ecs/components/macros.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Define test components using the macros.
// These are at file scope (as the macros require) and will auto-register via
// the ComponentRegistrar global inline variables.
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_macros {
    FLOAT(TestFloat);
    FLOAT(TestFloatDefault, 3.14f);
    DOUBLE(TestDouble);
    DOUBLE(TestDoubleDefault, 2.718);
    INT32(TestInt32);
    INT32(TestInt32Default, 42);
    UINT32(TestUint32);
    UINT32(TestUint32Default, 100u);
    INT16(TestInt16);
    INT16(TestInt16Default, -500);
    UINT16(TestUint16);
    UINT16(TestUint16Default, 1000);
    INT8(TestInt8);
    INT8(TestInt8Default, -1);
    UINT8(TestUint8);
    UINT8(TestUint8Default, 255);
    TAG(TestTag);

    struct DummyTarget { int x = 0; };
    POINTER(TestPointer, DummyTarget);
} // namespace test_macros

// ═══════════════════════════════════════════════════════════════════════════════
// Fixture: creates a world and runs all registrations
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct MacroFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override {
            stagehand::register_components_and_systems_with_world(world);
        }
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// FLOAT macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_FLOAT, default_value_is_zero) {
    test_macros::TestFloat f;
    ASSERT_NEAR(f.value, 0.0f, 1e-9f);
}

TEST(Macros_FLOAT, custom_default_value) {
    test_macros::TestFloatDefault f;
    ASSERT_NEAR(f.value, 3.14f, 1e-5f);
}

TEST(Macros_FLOAT, construct_from_value) {
    test_macros::TestFloat f(42.5f);
    ASSERT_NEAR(f.value, 42.5f, 1e-9f);
}

TEST(Macros_FLOAT, implicit_conversion_to_float) {
    test_macros::TestFloat f(10.0f);
    float v = f;
    ASSERT_NEAR(v, 10.0f, 1e-9f);
}

TEST(Macros_FLOAT, assignment_operator) {
    test_macros::TestFloat f;
    f = 99.0f;
    ASSERT_NEAR(f.value, 99.0f, 1e-9f);
}

TEST_F(MacroFixture, float_component_is_registered_in_flecs) {
    auto c = world.component<test_macros::TestFloat>();
    ASSERT_TRUE(c.id() != 0);
}

TEST_F(MacroFixture, float_getter_is_registered) {
    auto& getters = stagehand::get_component_getters();
    ASSERT_TRUE(getters.count("TestFloat") == 1);
}

TEST_F(MacroFixture, float_setter_is_registered) {
    auto& setters = stagehand::get_component_setters();
    ASSERT_TRUE(setters.count("TestFloat") == 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// DOUBLE macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_DOUBLE, default_value_is_zero) {
    test_macros::TestDouble d;
    ASSERT_NEAR(d.value, 0.0, 1e-15);
}

TEST(Macros_DOUBLE, custom_default_value) {
    test_macros::TestDoubleDefault d;
    ASSERT_NEAR(d.value, 2.718, 1e-6);
}

TEST(Macros_DOUBLE, construct_from_value) {
    test_macros::TestDouble d(1.23456789);
    ASSERT_NEAR(d.value, 1.23456789, 1e-15);
}

TEST(Macros_DOUBLE, implicit_conversion) {
    test_macros::TestDouble d(5.5);
    double v = d;
    ASSERT_NEAR(v, 5.5, 1e-15);
}

TEST(Macros_DOUBLE, assignment_operator) {
    test_macros::TestDouble d;
    d = 77.7;
    ASSERT_NEAR(d.value, 77.7, 1e-10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT32 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT32, default_value_is_zero) {
    test_macros::TestInt32 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT32, custom_default_value) {
    test_macros::TestInt32Default i;
    ASSERT_EQ(i.value, 42);
}

TEST(Macros_INT32, construct_from_value) {
    test_macros::TestInt32 i(-12345);
    ASSERT_EQ(i.value, -12345);
}

TEST(Macros_INT32, implicit_conversion) {
    test_macros::TestInt32 i(100);
    int32_t v = i;
    ASSERT_EQ(v, 100);
}

TEST(Macros_INT32, assignment_operator) {
    test_macros::TestInt32 i;
    i = 999;
    ASSERT_EQ(i.value, 999);
}

TEST(Macros_INT32, mutable_reference_conversion) {
    test_macros::TestInt32 i(5);
    int32_t& ref = i;
    ref = 10;
    ASSERT_EQ(i.value, 10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT32 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT32, default_value_is_zero) {
    test_macros::TestUint32 u;
    ASSERT_EQ(u.value, 0u);
}

TEST(Macros_UINT32, custom_default_value) {
    test_macros::TestUint32Default u;
    ASSERT_EQ(u.value, 100u);
}

TEST(Macros_UINT32, construct_and_convert) {
    test_macros::TestUint32 u(0xDEADBEEF);
    uint32_t v = u;
    ASSERT_EQ(v, 0xDEADBEEF);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT16 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT16, default_value_is_zero) {
    test_macros::TestInt16 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT16, custom_default_value) {
    test_macros::TestInt16Default i;
    ASSERT_EQ(i.value, -500);
}

TEST(Macros_INT16, construct_and_convert) {
    test_macros::TestInt16 i(static_cast<int16_t>(-32768));
    int16_t v = i;
    ASSERT_EQ(v, -32768);
}

TEST(Macros_INT16, assignment_operator) {
    test_macros::TestInt16 i;
    i = static_cast<int16_t>(32767);
    ASSERT_EQ(i.value, 32767);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT16 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT16, default_value_is_zero) {
    test_macros::TestUint16 u;
    ASSERT_EQ(u.value, 0);
}

TEST(Macros_UINT16, custom_default_value) {
    test_macros::TestUint16Default u;
    ASSERT_EQ(u.value, 1000);
}

TEST(Macros_UINT16, max_value) {
    test_macros::TestUint16 u(static_cast<uint16_t>(65535));
    uint16_t v = u;
    ASSERT_EQ(v, 65535);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT8 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT8, default_value_is_zero) {
    test_macros::TestInt8 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT8, custom_default_value) {
    test_macros::TestInt8Default i;
    ASSERT_EQ(i.value, -1);
}

TEST(Macros_INT8, boundary_values) {
    test_macros::TestInt8 imin(static_cast<int8_t>(-128));
    test_macros::TestInt8 imax(static_cast<int8_t>(127));
    ASSERT_EQ(static_cast<int8_t>(imin), -128);
    ASSERT_EQ(static_cast<int8_t>(imax), 127);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT8 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT8, default_value_is_zero) {
    test_macros::TestUint8 u;
    ASSERT_EQ(u.value, 0);
}

TEST(Macros_UINT8, custom_default_value) {
    test_macros::TestUint8Default u;
    ASSERT_EQ(u.value, 255);
}

TEST(Macros_UINT8, max_value) {
    test_macros::TestUint8 u(static_cast<uint8_t>(255));
    uint8_t v = u;
    ASSERT_EQ(v, 255);
}

// ═══════════════════════════════════════════════════════════════════════════════
// TAG macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_TAG, tag_is_empty_struct) {
    // Tags should be zero-size (empty struct).
    ASSERT_EQ(sizeof(test_macros::TestTag), 1u); // C++ empty struct = 1 byte
}

TEST(Macros_TAG, tag_is_default_constructible) {
    test_macros::TestTag t{};
    (void)t;
    ASSERT_TRUE(true);
}

TEST_F(MacroFixture, tag_component_is_registered_in_flecs) {
    auto c = world.component<test_macros::TestTag>();
    ASSERT_TRUE(c.id() != 0);
}

TEST_F(MacroFixture, tag_can_be_added_to_entity) {
    auto e = world.entity().add<test_macros::TestTag>();
    ASSERT_TRUE(e.has<test_macros::TestTag>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// POINTER macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_POINTER, default_is_nullptr) {
    test_macros::TestPointer p;
    ASSERT_TRUE(p.ptr == nullptr);
    ASSERT_FALSE(static_cast<bool>(p));
}

TEST(Macros_POINTER, construct_from_raw_pointer) {
    test_macros::DummyTarget target;
    target.x = 42;
    test_macros::TestPointer p(&target);
    ASSERT_TRUE(p.ptr == &target);
    ASSERT_TRUE(static_cast<bool>(p));
    ASSERT_EQ(p->x, 42);
}

TEST(Macros_POINTER, construct_from_uintptr) {
    test_macros::DummyTarget target;
    auto addr = reinterpret_cast<std::uintptr_t>(&target);
    test_macros::TestPointer p(addr);
    ASSERT_TRUE(p.ptr == &target);
}

TEST(Macros_POINTER, arrow_operator) {
    test_macros::DummyTarget target;
    target.x = 99;
    test_macros::TestPointer p(&target);
    ASSERT_EQ(p->x, 99);
}

TEST(Macros_POINTER, equality_operators) {
    test_macros::DummyTarget a, b;
    test_macros::TestPointer pa(&a), pb(&b), pa2(&a);
    ASSERT_TRUE(pa == pa2);
    ASSERT_TRUE(pa != pb);
}

TEST(Macros_POINTER, assignment_operator) {
    test_macros::DummyTarget target;
    test_macros::TestPointer p;
    p = &target;
    ASSERT_TRUE(p.ptr == &target);
}

TEST(Macros_POINTER, uintptr_conversion) {
    test_macros::DummyTarget target;
    test_macros::TestPointer p(&target);
    std::uintptr_t addr = static_cast<std::uintptr_t>(p);
    ASSERT_EQ(addr, reinterpret_cast<std::uintptr_t>(&target));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Flecs integration: components added to entities
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(MacroFixture, int32_component_on_entity_roundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestInt32>({ 77 });
    const auto* data = e.try_get<test_macros::TestInt32>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->value, 77);
}

TEST_F(MacroFixture, float_component_on_entity_roundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestFloat>({ 1.5f });
    const auto* data = e.try_get<test_macros::TestFloat>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->value, 1.5f, 1e-9f);
}

TEST_F(MacroFixture, double_component_on_entity_roundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestDouble>({ 9.99 });
    const auto* data = e.try_get<test_macros::TestDouble>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->value, 9.99, 1e-12);
}

TEST_F(MacroFixture, uint8_component_on_entity_roundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestUint8>({ 200 });
    const auto* data = e.try_get<test_macros::TestUint8>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->value, 200);
}

TEST_F(MacroFixture, pointer_component_on_entity_roundtrip) {
    test_macros::DummyTarget target;
    target.x = 123;

    auto e = world.entity();
    e.set<test_macros::TestPointer>({ &target });
    const auto* data = e.try_get<test_macros::TestPointer>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->ptr, &target);
    ASSERT_EQ(data->ptr->x, 123);
}

TEST_F(MacroFixture, multiple_components_on_same_entity) {
    auto e = world.entity();
    e.set<test_macros::TestInt32>({ 10 });
    e.set<test_macros::TestFloat>({ 20.0f });
    e.add<test_macros::TestTag>();

    ASSERT_TRUE(e.has<test_macros::TestInt32>());
    ASSERT_TRUE(e.has<test_macros::TestFloat>());
    ASSERT_TRUE(e.has<test_macros::TestTag>());

    ASSERT_EQ(e.try_get<test_macros::TestInt32>()->value, 10);
    ASSERT_NEAR(e.try_get<test_macros::TestFloat>()->value, 20.0f, 1e-9f);
}
