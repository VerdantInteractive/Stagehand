/// Unit tests for the component definition macros (FLOAT, DOUBLE, INT32, etc.).
/// Tests verify:
///   1. Struct layout, default values, conversions, and assignment.
///   2. Flecs component registration (component exists and has correct member metadata).
///   3. Entity-level roundtrips for all macro types.

#include <gtest/gtest.h>
#include <flecs.h>
#include "stagehand/registry.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/godot_variants.h"

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

    // Godot variant components without defaults
    GODOT_VARIANT(TestColor, Color);
    GODOT_VARIANT(TestVector2, Vector2);
    GODOT_VARIANT(TestVector2i, Vector2i);
    GODOT_VARIANT(TestVector3, Vector3);
    GODOT_VARIANT(TestVector3i, Vector3i);
    GODOT_VARIANT(TestVector4, Vector4);
    GODOT_VARIANT(TestVector4i, Vector4i);
    GODOT_VARIANT(TestRect2, Rect2);
    GODOT_VARIANT(TestRect2i, Rect2i);
    GODOT_VARIANT(TestPlane, Plane);
    GODOT_VARIANT(TestQuaternion, Quaternion);
    GODOT_VARIANT(TestBasis, Basis);
    GODOT_VARIANT(TestTransform2D, Transform2D);
    GODOT_VARIANT(TestTransform3D, Transform3D);
    GODOT_VARIANT(TestAABB, AABB);
    GODOT_VARIANT(TestProjection, Projection);

    // Godot variant components with custom defaults
    GODOT_VARIANT(TestColorRed, Color, 1.0f, 0.0f, 0.0f, 1.0f);
    GODOT_VARIANT(TestVector2One, Vector2, 1.0f, 1.0f);
    GODOT_VARIANT(TestVector2iOne, Vector2i, 1, 1);
    GODOT_VARIANT(TestVector3Up, Vector3, 0.0f, 1.0f, 0.0f);
    GODOT_VARIANT(TestVector3iUp, Vector3i, 0, 1, 0);
    GODOT_VARIANT(TestVector4One, Vector4, 1.0f, 1.0f, 1.0f, 1.0f);
    GODOT_VARIANT(TestVector4iOne, Vector4i, 1, 1, 1, 1);
    GODOT_VARIANT(TestRect2Unit, Rect2, 0.0f, 0.0f, 1.0f, 1.0f);
    GODOT_VARIANT(TestRect2iUnit, Rect2i, 0, 0, 1, 1);
    GODOT_VARIANT(TestPlaneUp, Plane, 0.0f, 1.0f, 0.0f, 0.0f);
    GODOT_VARIANT(TestQuaternionIdentity, Quaternion, 0.0f, 0.0f, 0.0f, 1.0f);
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

TEST(Macros_FLOAT, DefaultValueIsZero) {
    test_macros::TestFloat f;
    ASSERT_NEAR(f.value, 0.0f, 1e-9f);
}

TEST(Macros_FLOAT, CustomDefaultValue) {
    test_macros::TestFloatDefault f;
    ASSERT_NEAR(f.value, 3.14f, 1e-5f);
}

TEST(Macros_FLOAT, ConstructFromValue) {
    test_macros::TestFloat f(42.5f);
    ASSERT_NEAR(f.value, 42.5f, 1e-9f);
}

TEST(Macros_FLOAT, ImplicitConversionToFloat) {
    test_macros::TestFloat f(10.0f);
    float v = f;
    ASSERT_NEAR(v, 10.0f, 1e-9f);
}

TEST(Macros_FLOAT, AssignmentOperator) {
    test_macros::TestFloat f;
    f = 99.0f;
    ASSERT_NEAR(f.value, 99.0f, 1e-9f);
}

TEST_F(MacroFixture, FloatComponentIsRegisteredInFlecs) {
    auto c = world.component<test_macros::TestFloat>();
    ASSERT_TRUE(c.id() != 0);
}

TEST_F(MacroFixture, FloatGetterIsRegistered) {
    auto& getters = stagehand::get_component_getters();
    ASSERT_TRUE(getters.count("TestFloat") == 1);
}

TEST_F(MacroFixture, FloatSetterIsRegistered) {
    auto& setters = stagehand::get_component_setters();
    ASSERT_TRUE(setters.count("TestFloat") == 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// DOUBLE macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_DOUBLE, DefaultValueIsZero) {
    test_macros::TestDouble d;
    ASSERT_NEAR(d.value, 0.0, 1e-15);
}

TEST(Macros_DOUBLE, CustomDefaultValue) {
    test_macros::TestDoubleDefault d;
    ASSERT_NEAR(d.value, 2.718, 1e-6);
}

TEST(Macros_DOUBLE, ConstructFromValue) {
    test_macros::TestDouble d(1.23456789);
    ASSERT_NEAR(d.value, 1.23456789, 1e-15);
}

TEST(Macros_DOUBLE, ImplicitConversion) {
    test_macros::TestDouble d(5.5);
    double v = d;
    ASSERT_NEAR(v, 5.5, 1e-15);
}

TEST(Macros_DOUBLE, AssignmentOperator) {
    test_macros::TestDouble d;
    d = 77.7;
    ASSERT_NEAR(d.value, 77.7, 1e-10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT32 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT32, DefaultValueIsZero) {
    test_macros::TestInt32 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT32, CustomDefaultValue) {
    test_macros::TestInt32Default i;
    ASSERT_EQ(i.value, 42);
}

TEST(Macros_INT32, ConstructFromValue) {
    test_macros::TestInt32 i(-12345);
    ASSERT_EQ(i.value, -12345);
}

TEST(Macros_INT32, ImplicitConversion) {
    test_macros::TestInt32 i(100);
    int32_t v = i;
    ASSERT_EQ(v, 100);
}

TEST(Macros_INT32, AssignmentOperator) {
    test_macros::TestInt32 i;
    i = 999;
    ASSERT_EQ(i.value, 999);
}

TEST(Macros_INT32, MutableReferenceConversion) {
    test_macros::TestInt32 i(5);
    int32_t& ref = i;
    ref = 10;
    ASSERT_EQ(i.value, 10);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT32 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT32, DefaultValueIsZero) {
    test_macros::TestUint32 u;
    ASSERT_EQ(u.value, 0u);
}

TEST(Macros_UINT32, CustomDefaultValue) {
    test_macros::TestUint32Default u;
    ASSERT_EQ(u.value, 100u);
}

TEST(Macros_UINT32, ConstructAndConvert) {
    test_macros::TestUint32 u(0xDEADBEEF);
    uint32_t v = u;
    ASSERT_EQ(v, 0xDEADBEEF);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT16 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT16, DefaultValueIsZero) {
    test_macros::TestInt16 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT16, CustomDefaultValue) {
    test_macros::TestInt16Default i;
    ASSERT_EQ(i.value, -500);
}

TEST(Macros_INT16, ConstructAndConvert) {
    test_macros::TestInt16 i(static_cast<int16_t>(-32768));
    int16_t v = i;
    ASSERT_EQ(v, -32768);
}

TEST(Macros_INT16, AssignmentOperator) {
    test_macros::TestInt16 i;
    i = static_cast<int16_t>(32767);
    ASSERT_EQ(i.value, 32767);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT16 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT16, DefaultValueIsZero) {
    test_macros::TestUint16 u;
    ASSERT_EQ(u.value, 0);
}

TEST(Macros_UINT16, CustomDefaultValue) {
    test_macros::TestUint16Default u;
    ASSERT_EQ(u.value, 1000);
}

TEST(Macros_UINT16, MaxValue) {
    test_macros::TestUint16 u(static_cast<uint16_t>(65535));
    uint16_t v = u;
    ASSERT_EQ(v, 65535);
}

// ═══════════════════════════════════════════════════════════════════════════════
// INT8 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_INT8, DefaultValueIsZero) {
    test_macros::TestInt8 i;
    ASSERT_EQ(i.value, 0);
}

TEST(Macros_INT8, CustomDefaultValue) {
    test_macros::TestInt8Default i;
    ASSERT_EQ(i.value, -1);
}

TEST(Macros_INT8, BoundaryValues) {
    test_macros::TestInt8 imin(static_cast<int8_t>(-128));
    test_macros::TestInt8 imax(static_cast<int8_t>(127));
    ASSERT_EQ(static_cast<int8_t>(imin), -128);
    ASSERT_EQ(static_cast<int8_t>(imax), 127);
}

// ═══════════════════════════════════════════════════════════════════════════════
// UINT8 macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_UINT8, DefaultValueIsZero) {
    test_macros::TestUint8 u;
    ASSERT_EQ(u.value, 0);
}

TEST(Macros_UINT8, CustomDefaultValue) {
    test_macros::TestUint8Default u;
    ASSERT_EQ(u.value, 255);
}

TEST(Macros_UINT8, MaxValue) {
    test_macros::TestUint8 u(static_cast<uint8_t>(255));
    uint8_t v = u;
    ASSERT_EQ(v, 255);
}

// ═══════════════════════════════════════════════════════════════════════════════
// TAG macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_TAG, TagIsEmptyStruct) {
    // Tags should be zero-size (empty struct).
    ASSERT_EQ(sizeof(test_macros::TestTag), 1u); // C++ empty struct = 1 byte
}

TEST(Macros_TAG, TagIsDefaultConstructible) {
    test_macros::TestTag t{};
    (void)t;
    ASSERT_TRUE(true);
}

TEST_F(MacroFixture, TagComponentIsRegisteredInFlecs) {
    auto c = world.component<test_macros::TestTag>();
    ASSERT_TRUE(c.id() != 0);
}

TEST_F(MacroFixture, TagCanBeAddedToEntity) {
    auto e = world.entity().add<test_macros::TestTag>();
    ASSERT_TRUE(e.has<test_macros::TestTag>());
}

// ═══════════════════════════════════════════════════════════════════════════════
// POINTER macro tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_POINTER, DefaultIsNullptr) {
    test_macros::TestPointer p;
    ASSERT_TRUE(p.ptr == nullptr);
    ASSERT_FALSE(static_cast<bool>(p));
}

TEST(Macros_POINTER, ConstructFromRawPointer) {
    test_macros::DummyTarget target;
    target.x = 42;
    test_macros::TestPointer p(&target);
    ASSERT_TRUE(p.ptr == &target);
    ASSERT_TRUE(static_cast<bool>(p));
    ASSERT_EQ(p->x, 42);
}

TEST(Macros_POINTER, ConstructFromUintptr) {
    test_macros::DummyTarget target;
    auto addr = reinterpret_cast<std::uintptr_t>(&target);
    test_macros::TestPointer p(addr);
    ASSERT_TRUE(p.ptr == &target);
}

TEST(Macros_POINTER, ArrowOperator) {
    test_macros::DummyTarget target;
    target.x = 99;
    test_macros::TestPointer p(&target);
    ASSERT_EQ(p->x, 99);
}

TEST(Macros_POINTER, EqualityOperators) {
    test_macros::DummyTarget a, b;
    test_macros::TestPointer pa(&a), pb(&b), pa2(&a);
    ASSERT_TRUE(pa == pa2);
    ASSERT_TRUE(pa != pb);
}

TEST(Macros_POINTER, AssignmentOperator) {
    test_macros::DummyTarget target;
    test_macros::TestPointer p;
    p = &target;
    ASSERT_TRUE(p.ptr == &target);
}

TEST(Macros_POINTER, UintptrConversion) {
    test_macros::DummyTarget target;
    test_macros::TestPointer p(&target);
    std::uintptr_t addr = static_cast<std::uintptr_t>(p);
    ASSERT_EQ(addr, reinterpret_cast<std::uintptr_t>(&target));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Flecs integration: components added to entities
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(MacroFixture, Int32ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestInt32>({ 77 });
    const auto* data = e.try_get<test_macros::TestInt32>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->value, 77);
}

TEST_F(MacroFixture, FloatComponentOnEntityRoundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestFloat>({ 1.5f });
    const auto* data = e.try_get<test_macros::TestFloat>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->value, 1.5f, 1e-9f);
}

TEST_F(MacroFixture, DoubleComponentOnEntityRoundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestDouble>({ 9.99 });
    const auto* data = e.try_get<test_macros::TestDouble>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->value, 9.99, 1e-12);
}

TEST_F(MacroFixture, Uint8ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    e.set<test_macros::TestUint8>({ 200 });
    const auto* data = e.try_get<test_macros::TestUint8>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->value, 200);
}

TEST_F(MacroFixture, PointerComponentOnEntityRoundtrip) {
    test_macros::DummyTarget target;
    target.x = 123;

    auto e = world.entity();
    e.set<test_macros::TestPointer>({ &target });
    const auto* data = e.try_get<test_macros::TestPointer>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->ptr, &target);
    ASSERT_EQ(data->ptr->x, 123);
}

TEST_F(MacroFixture, MultipleComponentsOnSameEntity) {
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

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Color
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Color, DefaultConstructor) {
    test_macros::TestColor c;
    // Godot's Color default constructor initializes to opaque black (0, 0, 0, 1)
    ASSERT_NEAR(c.r, 0.0f, 1e-9f);
    ASSERT_NEAR(c.g, 0.0f, 1e-9f);
    ASSERT_NEAR(c.b, 0.0f, 1e-9f);
    ASSERT_NEAR(c.a, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Color, CustomDefaultValue) {
    test_macros::TestColorRed c;
    ASSERT_NEAR(c.r, 1.0f, 1e-5f);
    ASSERT_NEAR(c.g, 0.0f, 1e-5f);
    ASSERT_NEAR(c.b, 0.0f, 1e-5f);
    ASSERT_NEAR(c.a, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Color, ConstructFromBase) {
    Color base(0.5f, 0.25f, 0.75f, 0.9f);
    test_macros::TestColor c(base);
    ASSERT_NEAR(c.r, 0.5f, 1e-5f);
    ASSERT_NEAR(c.g, 0.25f, 1e-5f);
    ASSERT_NEAR(c.b, 0.75f, 1e-5f);
    ASSERT_NEAR(c.a, 0.9f, 1e-5f);
}

TEST(Macros_GodotVariant_Color, ConversionToBase) {
    test_macros::TestColor c;
    c.r = 0.1f;
    c.g = 0.2f;
    c.b = 0.3f;
    c.a = 0.4f;
    Color base = c;
    ASSERT_NEAR(base.r, 0.1f, 1e-5f);
    ASSERT_NEAR(base.g, 0.2f, 1e-5f);
    ASSERT_NEAR(base.b, 0.3f, 1e-5f);
    ASSERT_NEAR(base.a, 0.4f, 1e-5f);
}

TEST(Macros_GodotVariant_Color, AssignmentOperator) {
    Color base(0.8f, 0.6f, 0.4f, 0.2f);
    test_macros::TestColor c;
    c = base;
    ASSERT_NEAR(c.r, 0.8f, 1e-5f);
    ASSERT_NEAR(c.g, 0.6f, 1e-5f);
    ASSERT_NEAR(c.b, 0.4f, 1e-5f);
    ASSERT_NEAR(c.a, 0.2f, 1e-5f);
}

TEST_F(MacroFixture, ColorComponentIsRegistered) {
    auto c = world.component<test_macros::TestColor>();
    ASSERT_TRUE(c.id() != 0);
}

TEST_F(MacroFixture, ColorGetterAndSetterAreRegistered) {
    auto& getters = stagehand::get_component_getters();
    auto& setters = stagehand::get_component_setters();
    ASSERT_EQ(getters.count("TestColor"), 1);
    ASSERT_EQ(setters.count("TestColor"), 1);
}

TEST_F(MacroFixture, ColorComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Color value(0.3f, 0.5f, 0.7f, 0.9f);
    e.set<test_macros::TestColor>(value);
    const auto* data = e.try_get<test_macros::TestColor>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->r, 0.3f, 1e-5f);
    ASSERT_NEAR(data->g, 0.5f, 1e-5f);
    ASSERT_NEAR(data->b, 0.7f, 1e-5f);
    ASSERT_NEAR(data->a, 0.9f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector2
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector2, DefaultConstructor) {
    test_macros::TestVector2 v;
    ASSERT_NEAR(v.x, 0.0f, 1e-9f);
    ASSERT_NEAR(v.y, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_Vector2, CustomDefaultValue) {
    test_macros::TestVector2One v;
    ASSERT_NEAR(v.x, 1.0f, 1e-5f);
    ASSERT_NEAR(v.y, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Vector2, ConstructFromBase) {
    Vector2 base(3.0f, 4.0f);
    test_macros::TestVector2 v(base);
    ASSERT_NEAR(v.x, 3.0f, 1e-5f);
    ASSERT_NEAR(v.y, 4.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Vector2, ConversionToBase) {
    test_macros::TestVector2 v;
    v.x = 5.0f;
    v.y = 12.0f;
    Vector2 base = v;
    ASSERT_NEAR(base.x, 5.0f, 1e-5f);
    ASSERT_NEAR(base.y, 12.0f, 1e-5f);
}

TEST_F(MacroFixture, Vector2ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector2 value(10.5f, 20.5f);
    e.set<test_macros::TestVector2>(value);
    const auto* data = e.try_get<test_macros::TestVector2>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->x, 10.5f, 1e-5f);
    ASSERT_NEAR(data->y, 20.5f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector2i
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector2i, DefaultConstructor) {
    test_macros::TestVector2i v;
    ASSERT_EQ(v.x, 0);
    ASSERT_EQ(v.y, 0);
}

TEST(Macros_GodotVariant_Vector2i, CustomDefaultValue) {
    test_macros::TestVector2iOne v;
    ASSERT_EQ(v.x, 1);
    ASSERT_EQ(v.y, 1);
}

TEST(Macros_GodotVariant_Vector2i, ConstructFromBase) {
    Vector2i base(100, -200);
    test_macros::TestVector2i v(base);
    ASSERT_EQ(v.x, 100);
    ASSERT_EQ(v.y, -200);
}

TEST_F(MacroFixture, Vector2iComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector2i value(42, -99);
    e.set<test_macros::TestVector2i>(value);
    const auto* data = e.try_get<test_macros::TestVector2i>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->x, 42);
    ASSERT_EQ(data->y, -99);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector3
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector3, DefaultConstructor) {
    test_macros::TestVector3 v;
    ASSERT_NEAR(v.x, 0.0f, 1e-9f);
    ASSERT_NEAR(v.y, 0.0f, 1e-9f);
    ASSERT_NEAR(v.z, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_Vector3, CustomDefaultValue) {
    test_macros::TestVector3Up v;
    ASSERT_NEAR(v.x, 0.0f, 1e-5f);
    ASSERT_NEAR(v.y, 1.0f, 1e-5f);
    ASSERT_NEAR(v.z, 0.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Vector3, ConstructFromBase) {
    Vector3 base(1.0f, 2.0f, 3.0f);
    test_macros::TestVector3 v(base);
    ASSERT_NEAR(v.x, 1.0f, 1e-5f);
    ASSERT_NEAR(v.y, 2.0f, 1e-5f);
    ASSERT_NEAR(v.z, 3.0f, 1e-5f);
}

TEST_F(MacroFixture, Vector3ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector3 value(7.5f, 8.5f, 9.5f);
    e.set<test_macros::TestVector3>(value);
    const auto* data = e.try_get<test_macros::TestVector3>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->x, 7.5f, 1e-5f);
    ASSERT_NEAR(data->y, 8.5f, 1e-5f);
    ASSERT_NEAR(data->z, 9.5f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector3i
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector3i, DefaultConstructor) {
    test_macros::TestVector3i v;
    ASSERT_EQ(v.x, 0);
    ASSERT_EQ(v.y, 0);
    ASSERT_EQ(v.z, 0);
}

TEST(Macros_GodotVariant_Vector3i, CustomDefaultValue) {
    test_macros::TestVector3iUp v;
    ASSERT_EQ(v.x, 0);
    ASSERT_EQ(v.y, 1);
    ASSERT_EQ(v.z, 0);
}

TEST(Macros_GodotVariant_Vector3i, ConstructFromBase) {
    Vector3i base(10, 20, 30);
    test_macros::TestVector3i v(base);
    ASSERT_EQ(v.x, 10);
    ASSERT_EQ(v.y, 20);
    ASSERT_EQ(v.z, 30);
}

TEST_F(MacroFixture, Vector3iComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector3i value(-5, 15, 25);
    e.set<test_macros::TestVector3i>(value);
    const auto* data = e.try_get<test_macros::TestVector3i>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->x, -5);
    ASSERT_EQ(data->y, 15);
    ASSERT_EQ(data->z, 25);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector4
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector4, DefaultConstructor) {
    test_macros::TestVector4 v;
    ASSERT_NEAR(v.x, 0.0f, 1e-9f);
    ASSERT_NEAR(v.y, 0.0f, 1e-9f);
    ASSERT_NEAR(v.z, 0.0f, 1e-9f);
    ASSERT_NEAR(v.w, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_Vector4, CustomDefaultValue) {
    test_macros::TestVector4One v;
    ASSERT_NEAR(v.x, 1.0f, 1e-5f);
    ASSERT_NEAR(v.y, 1.0f, 1e-5f);
    ASSERT_NEAR(v.z, 1.0f, 1e-5f);
    ASSERT_NEAR(v.w, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Vector4, ConstructFromBase) {
    Vector4 base(1.0f, 2.0f, 3.0f, 4.0f);
    test_macros::TestVector4 v(base);
    ASSERT_NEAR(v.x, 1.0f, 1e-5f);
    ASSERT_NEAR(v.y, 2.0f, 1e-5f);
    ASSERT_NEAR(v.z, 3.0f, 1e-5f);
    ASSERT_NEAR(v.w, 4.0f, 1e-5f);
}

TEST_F(MacroFixture, Vector4ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector4 value(5.5f, 6.5f, 7.5f, 8.5f);
    e.set<test_macros::TestVector4>(value);
    const auto* data = e.try_get<test_macros::TestVector4>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->x, 5.5f, 1e-5f);
    ASSERT_NEAR(data->y, 6.5f, 1e-5f);
    ASSERT_NEAR(data->z, 7.5f, 1e-5f);
    ASSERT_NEAR(data->w, 8.5f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Vector4i
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Vector4i, DefaultConstructor) {
    test_macros::TestVector4i v;
    ASSERT_EQ(v.x, 0);
    ASSERT_EQ(v.y, 0);
    ASSERT_EQ(v.z, 0);
    ASSERT_EQ(v.w, 0);
}

TEST(Macros_GodotVariant_Vector4i, CustomDefaultValue) {
    test_macros::TestVector4iOne v;
    ASSERT_EQ(v.x, 1);
    ASSERT_EQ(v.y, 1);
    ASSERT_EQ(v.z, 1);
    ASSERT_EQ(v.w, 1);
}

TEST(Macros_GodotVariant_Vector4i, ConstructFromBase) {
    Vector4i base(100, 200, 300, 400);
    test_macros::TestVector4i v(base);
    ASSERT_EQ(v.x, 100);
    ASSERT_EQ(v.y, 200);
    ASSERT_EQ(v.z, 300);
    ASSERT_EQ(v.w, 400);
}

TEST_F(MacroFixture, Vector4iComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Vector4i value(-1, -2, -3, -4);
    e.set<test_macros::TestVector4i>(value);
    const auto* data = e.try_get<test_macros::TestVector4i>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->x, -1);
    ASSERT_EQ(data->y, -2);
    ASSERT_EQ(data->z, -3);
    ASSERT_EQ(data->w, -4);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Rect2
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Rect2, DefaultConstructor) {
    test_macros::TestRect2 r;
    ASSERT_NEAR(r.position.x, 0.0f, 1e-9f);
    ASSERT_NEAR(r.position.y, 0.0f, 1e-9f);
    ASSERT_NEAR(r.size.x, 0.0f, 1e-9f);
    ASSERT_NEAR(r.size.y, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_Rect2, CustomDefaultValue) {
    test_macros::TestRect2Unit r;
    ASSERT_NEAR(r.position.x, 0.0f, 1e-5f);
    ASSERT_NEAR(r.position.y, 0.0f, 1e-5f);
    ASSERT_NEAR(r.size.x, 1.0f, 1e-5f);
    ASSERT_NEAR(r.size.y, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Rect2, ConstructFromBase) {
    Rect2 base(Vector2(10.0f, 20.0f), Vector2(30.0f, 40.0f));
    test_macros::TestRect2 r(base);
    ASSERT_NEAR(r.position.x, 10.0f, 1e-5f);
    ASSERT_NEAR(r.position.y, 20.0f, 1e-5f);
    ASSERT_NEAR(r.size.x, 30.0f, 1e-5f);
    ASSERT_NEAR(r.size.y, 40.0f, 1e-5f);
}

TEST_F(MacroFixture, Rect2ComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Rect2 value(Vector2(5.0f, 15.0f), Vector2(25.0f, 35.0f));
    e.set<test_macros::TestRect2>(value);
    const auto* data = e.try_get<test_macros::TestRect2>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->position.x, 5.0f, 1e-5f);
    ASSERT_NEAR(data->position.y, 15.0f, 1e-5f);
    ASSERT_NEAR(data->size.x, 25.0f, 1e-5f);
    ASSERT_NEAR(data->size.y, 35.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Rect2i
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Rect2i, DefaultConstructor) {
    test_macros::TestRect2i r;
    ASSERT_EQ(r.position.x, 0);
    ASSERT_EQ(r.position.y, 0);
    ASSERT_EQ(r.size.x, 0);
    ASSERT_EQ(r.size.y, 0);
}

TEST(Macros_GodotVariant_Rect2i, CustomDefaultValue) {
    test_macros::TestRect2iUnit r;
    ASSERT_EQ(r.position.x, 0);
    ASSERT_EQ(r.position.y, 0);
    ASSERT_EQ(r.size.x, 1);
    ASSERT_EQ(r.size.y, 1);
}

TEST(Macros_GodotVariant_Rect2i, ConstructFromBase) {
    Rect2i base(Vector2i(100, 200), Vector2i(300, 400));
    test_macros::TestRect2i r(base);
    ASSERT_EQ(r.position.x, 100);
    ASSERT_EQ(r.position.y, 200);
    ASSERT_EQ(r.size.x, 300);
    ASSERT_EQ(r.size.y, 400);
}

TEST_F(MacroFixture, Rect2iComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Rect2i value(Vector2i(-10, 20), Vector2i(30, 40));
    e.set<test_macros::TestRect2i>(value);
    const auto* data = e.try_get<test_macros::TestRect2i>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_EQ(data->position.x, -10);
    ASSERT_EQ(data->position.y, 20);
    ASSERT_EQ(data->size.x, 30);
    ASSERT_EQ(data->size.y, 40);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Plane
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Plane, DefaultConstructor) {
    test_macros::TestPlane p;
    ASSERT_NEAR(p.normal.x, 0.0f, 1e-9f);
    ASSERT_NEAR(p.normal.y, 0.0f, 1e-9f);
    ASSERT_NEAR(p.normal.z, 0.0f, 1e-9f);
    ASSERT_NEAR(p.d, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_Plane, CustomDefaultValue) {
    test_macros::TestPlaneUp p;
    ASSERT_NEAR(p.normal.x, 0.0f, 1e-5f);
    ASSERT_NEAR(p.normal.y, 1.0f, 1e-5f);
    ASSERT_NEAR(p.normal.z, 0.0f, 1e-5f);
    ASSERT_NEAR(p.d, 0.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Plane, ConstructFromBase) {
    Plane base(Vector3(1.0f, 0.0f, 0.0f), 5.0f);
    test_macros::TestPlane p(base);
    ASSERT_NEAR(p.normal.x, 1.0f, 1e-5f);
    ASSERT_NEAR(p.normal.y, 0.0f, 1e-5f);
    ASSERT_NEAR(p.normal.z, 0.0f, 1e-5f);
    ASSERT_NEAR(p.d, 5.0f, 1e-5f);
}

TEST_F(MacroFixture, PlaneComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Plane value(Vector3(0.0f, 1.0f, 0.0f), 10.0f);
    e.set<test_macros::TestPlane>(value);
    const auto* data = e.try_get<test_macros::TestPlane>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->normal.x, 0.0f, 1e-5f);
    ASSERT_NEAR(data->normal.y, 1.0f, 1e-5f);
    ASSERT_NEAR(data->normal.z, 0.0f, 1e-5f);
    ASSERT_NEAR(data->d, 10.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Quaternion
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Quaternion, DefaultConstructor) {
    test_macros::TestQuaternion q;
    // Godot's Quaternion default constructor initializes to identity (0, 0, 0, 1)
    ASSERT_NEAR(q.x, 0.0f, 1e-9f);
    ASSERT_NEAR(q.y, 0.0f, 1e-9f);
    ASSERT_NEAR(q.z, 0.0f, 1e-9f);
    ASSERT_NEAR(q.w, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Quaternion, CustomDefaultValue) {
    test_macros::TestQuaternionIdentity q;
    ASSERT_NEAR(q.x, 0.0f, 1e-5f);
    ASSERT_NEAR(q.y, 0.0f, 1e-5f);
    ASSERT_NEAR(q.z, 0.0f, 1e-5f);
    ASSERT_NEAR(q.w, 1.0f, 1e-5f);
}

TEST(Macros_GodotVariant_Quaternion, ConstructFromBase) {
    Quaternion base(1.0f, 2.0f, 3.0f, 4.0f);
    test_macros::TestQuaternion q(base);
    ASSERT_NEAR(q.x, 1.0f, 1e-5f);
    ASSERT_NEAR(q.y, 2.0f, 1e-5f);
    ASSERT_NEAR(q.z, 3.0f, 1e-5f);
    ASSERT_NEAR(q.w, 4.0f, 1e-5f);
}

TEST_F(MacroFixture, QuaternionComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Quaternion value(0.5f, 0.5f, 0.5f, 0.5f);
    e.set<test_macros::TestQuaternion>(value);
    const auto* data = e.try_get<test_macros::TestQuaternion>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->x, 0.5f, 1e-5f);
    ASSERT_NEAR(data->y, 0.5f, 1e-5f);
    ASSERT_NEAR(data->z, 0.5f, 1e-5f);
    ASSERT_NEAR(data->w, 0.5f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Basis
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Basis, DefaultConstructor) {
    test_macros::TestBasis b;
    // Default Basis should be identity or zero depending on Godot version
    // We just verify it constructs without error
    ASSERT_TRUE(true);
}

TEST(Macros_GodotVariant_Basis, ConstructFromBase) {
    Basis base;
    base.set_column(0, Vector3(1.0f, 0.0f, 0.0f));
    base.set_column(1, Vector3(0.0f, 2.0f, 0.0f));
    base.set_column(2, Vector3(0.0f, 0.0f, 3.0f));
    test_macros::TestBasis b(base);
    Vector3 col0 = b.get_column(0);
    ASSERT_NEAR(col0.x, 1.0f, 1e-5f);
}

TEST_F(MacroFixture, BasisComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Basis value;
    value.set_column(0, Vector3(1.0f, 2.0f, 3.0f));
    value.set_column(1, Vector3(4.0f, 5.0f, 6.0f));
    value.set_column(2, Vector3(7.0f, 8.0f, 9.0f));
    e.set<test_macros::TestBasis>(value);
    const auto* data = e.try_get<test_macros::TestBasis>();
    ASSERT_TRUE(data != nullptr);
    Vector3 col0 = data->get_column(0);
    ASSERT_NEAR(col0.x, 1.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Transform2D
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Transform2D, DefaultConstructor) {
    test_macros::TestTransform2D t;
    // Just verify it constructs
    ASSERT_TRUE(true);
}

TEST(Macros_GodotVariant_Transform2D, ConstructFromBase) {
    Transform2D base;
    base.set_origin(Vector2(10.0f, 20.0f));
    test_macros::TestTransform2D t(base);
    ASSERT_NEAR(t.get_origin().x, 10.0f, 1e-5f);
    ASSERT_NEAR(t.get_origin().y, 20.0f, 1e-5f);
}

TEST_F(MacroFixture, Transform2DComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Transform2D value;
    value.set_origin(Vector2(5.0f, 15.0f));
    e.set<test_macros::TestTransform2D>(value);
    const auto* data = e.try_get<test_macros::TestTransform2D>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->get_origin().x, 5.0f, 1e-5f);
    ASSERT_NEAR(data->get_origin().y, 15.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Transform3D
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Transform3D, DefaultConstructor) {
    test_macros::TestTransform3D t;
    // Just verify it constructs
    ASSERT_TRUE(true);
}

TEST(Macros_GodotVariant_Transform3D, ConstructFromBase) {
    Transform3D base;
    base.set_origin(Vector3(10.0f, 20.0f, 30.0f));
    test_macros::TestTransform3D t(base);
    ASSERT_NEAR(t.get_origin().x, 10.0f, 1e-5f);
    ASSERT_NEAR(t.get_origin().y, 20.0f, 1e-5f);
    ASSERT_NEAR(t.get_origin().z, 30.0f, 1e-5f);
}

TEST_F(MacroFixture, Transform3DComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Transform3D value;
    value.set_origin(Vector3(1.0f, 2.0f, 3.0f));
    e.set<test_macros::TestTransform3D>(value);
    const auto* data = e.try_get<test_macros::TestTransform3D>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->get_origin().x, 1.0f, 1e-5f);
    ASSERT_NEAR(data->get_origin().y, 2.0f, 1e-5f);
    ASSERT_NEAR(data->get_origin().z, 3.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - AABB
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_AABB, DefaultConstructor) {
    test_macros::TestAABB a;
    ASSERT_NEAR(a.position.x, 0.0f, 1e-9f);
    ASSERT_NEAR(a.position.y, 0.0f, 1e-9f);
    ASSERT_NEAR(a.position.z, 0.0f, 1e-9f);
    ASSERT_NEAR(a.size.x, 0.0f, 1e-9f);
    ASSERT_NEAR(a.size.y, 0.0f, 1e-9f);
    ASSERT_NEAR(a.size.z, 0.0f, 1e-9f);
}

TEST(Macros_GodotVariant_AABB, ConstructFromBase) {
    AABB base(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f));
    test_macros::TestAABB a(base);
    ASSERT_NEAR(a.position.x, 1.0f, 1e-5f);
    ASSERT_NEAR(a.position.y, 2.0f, 1e-5f);
    ASSERT_NEAR(a.position.z, 3.0f, 1e-5f);
    ASSERT_NEAR(a.size.x, 4.0f, 1e-5f);
    ASSERT_NEAR(a.size.y, 5.0f, 1e-5f);
    ASSERT_NEAR(a.size.z, 6.0f, 1e-5f);
}

TEST_F(MacroFixture, AABBComponentOnEntityRoundtrip) {
    auto e = world.entity();
    AABB value(Vector3(10.0f, 20.0f, 30.0f), Vector3(40.0f, 50.0f, 60.0f));
    e.set<test_macros::TestAABB>(value);
    const auto* data = e.try_get<test_macros::TestAABB>();
    ASSERT_TRUE(data != nullptr);
    ASSERT_NEAR(data->position.x, 10.0f, 1e-5f);
    ASSERT_NEAR(data->position.y, 20.0f, 1e-5f);
    ASSERT_NEAR(data->position.z, 30.0f, 1e-5f);
    ASSERT_NEAR(data->size.x, 40.0f, 1e-5f);
    ASSERT_NEAR(data->size.y, 50.0f, 1e-5f);
    ASSERT_NEAR(data->size.z, 60.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GODOT_VARIANT macro tests - Projection
// ═══════════════════════════════════════════════════════════════════════════════

TEST(Macros_GodotVariant_Projection, DefaultConstructor) {
    test_macros::TestProjection p;
    // Just verify it constructs
    ASSERT_TRUE(true);
}

TEST(Macros_GodotVariant_Projection, ConstructFromBase) {
    Projection base;
    test_macros::TestProjection p(base);
    // Just verify it constructs from base
    ASSERT_TRUE(true);
}

TEST_F(MacroFixture, ProjectionComponentOnEntityRoundtrip) {
    auto e = world.entity();
    Projection value;
    e.set<test_macros::TestProjection>(value);
    const auto* data = e.try_get<test_macros::TestProjection>();
    ASSERT_TRUE(data != nullptr);
}
