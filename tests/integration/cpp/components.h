#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"

namespace stagehand_tests {

    // ─── Scalar component macros ─────────────────────────────────────────────────

    FLOAT(TestFloat, 0.0f);
    DOUBLE(TestDouble, 0.0);
    INT32(TestInt32, 0);
    UINT32(TestUint32, 0);
    INT16(TestInt16, 0);
    UINT16(TestUint16, 0);
    INT8(TestInt8, 0);
    UINT8(TestUint8, 0);

    // ─── Tag components ──────────────────────────────────────────────────────────

    TAG(TestTag);
    TAG(MarkerA);
    TAG(MarkerB);

    // ─── Collection component macros ─────────────────────────────────────────────

    VECTOR(TestVectorFloat, float);
    VECTOR(TestVectorInt, int32_t);
    ARRAY(TestArray4, float, 4);

    // ─── Godot Variant component macros ──────────────────────────────────────────

    GODOT_VARIANT(TestVector2, godot::Vector2);
    GODOT_VARIANT(TestVector3, godot::Vector3);
    GODOT_VARIANT(TestVector2i, godot::Vector2i);
    GODOT_VARIANT(TestVector3i, godot::Vector3i);
    GODOT_VARIANT(TestColor, godot::Color);
    GODOT_VARIANT(TestRect2, godot::Rect2);
    GODOT_VARIANT(TestTransform2D, godot::Transform2D);
    GODOT_VARIANT(TestTransform3D, godot::Transform3D);
    GODOT_VARIANT(TestQuaternion, godot::Quaternion);
    GODOT_VARIANT(TestDictionary, godot::Dictionary);
    GODOT_VARIANT(TestString, godot::String);

    // ─── Counter / accumulator components for system tests ───────────────────────

    INT32(TickCount, 0);
    INT32(AccumulatorValue, 0);
    FLOAT(EntityValue, 0.0f);

    // ─── Singleton for passing signal test results back to GDScript ──────────────

    GODOT_VARIANT(SignalTestResult, godot::Dictionary);

    // ─── Singleton for passing scene children test results back to GDScript ──────

    GODOT_VARIANT(SceneChildrenResult, godot::Dictionary);

    // ─── ComponentRegistrar::then() test: default value via on_add ────────────────

    FLOAT(DefaultedFloat, 0.0f).then([](flecs::component<DefaultedFloat> c) { c.on_add([](DefaultedFloat &f) { f.value = 42.0f; }); });

} // namespace stagehand_tests
