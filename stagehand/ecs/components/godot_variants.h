#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/signal.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include "stagehand/registry.h"

using godot::AABB;
using godot::Array;
using godot::Basis;
using godot::Callable;
using godot::Color;
using godot::Dictionary;
using godot::NodePath;
using godot::PackedByteArray;
using godot::PackedColorArray;
using godot::PackedFloat32Array;
using godot::PackedFloat64Array;
using godot::PackedInt32Array;
using godot::PackedInt64Array;
using godot::PackedStringArray;
using godot::PackedVector2Array;
using godot::PackedVector3Array;
using godot::PackedVector4Array;
using godot::Plane;
using godot::Projection;
using godot::Quaternion;
using godot::RID;
using godot::real_t;
using godot::Rect2;
using godot::Rect2i;
using godot::Signal;
using godot::String;
using godot::StringName;
using godot::Transform2D;
using godot::Transform3D;
using godot::Vector2;
using godot::Vector2i;
using godot::Vector3;
using godot::Vector3i;
using godot::Vector4;
using godot::Vector4i;

/// Helper to register Color layout for any type T that matches Color layout
template <typename T>
void register_color_members(flecs::component<T> c) {
    c.template member<float>("r");
    c.template member<float>("g");
    c.template member<float>("b");
    c.template member<float>("a");
}

/// Helper to register Vector2 layout for any type T that matches Vector2 layout
template <typename T>
void register_vector2_members(flecs::component<T> c) {
    c.template member<real_t>("x");
    c.template member<real_t>("y");
}

/// Helper to register Vector2i layout for any type T that matches Vector2i layout
template <typename T>
void register_vector2i_members(flecs::component<T> c) {
    c.template member<int32_t>("x");
    c.template member<int32_t>("y");
}

/// Helper to register Vector3 layout for any type T that matches Vector3 layout
template <typename T>
void register_vector3_members(flecs::component<T> c) {
    c.template member<real_t>("x");
    c.template member<real_t>("y");
    c.template member<real_t>("z");
}

/// Helper to register Vector3i layout for any type T that matches Vector3i layout
template <typename T>
void register_vector3i_members(flecs::component<T> c) {
    c.template member<int32_t>("x");
    c.template member<int32_t>("y");
    c.template member<int32_t>("z");
}

/// Helper to register Vector4 layout for any type T that matches Vector4 layout
template <typename T>
void register_vector4_members(flecs::component<T> c) {
    c.template member<real_t>("x");
    c.template member<real_t>("y");
    c.template member<real_t>("z");
    c.template member<real_t>("w");
}

/// Helper to register Vector4i layout for any type T that matches Vector4i layout
template <typename T>
void register_vector4i_members(flecs::component<T> c) {
    c.template member<int32_t>("x");
    c.template member<int32_t>("y");
    c.template member<int32_t>("z");
    c.template member<int32_t>("w");
}

/// Helper to register Rect2 layout for any type T that matches Rect2 layout
template <typename T>
void register_rect2_members(flecs::component<T> c) {
    c.template member<Vector2>("position");
    c.template member<Vector2>("size");
}

/// Helper to register Rect2i layout for any type T that matches Rect2i layout
template <typename T>
void register_rect2i_members(flecs::component<T> c) {
    c.template member<Vector2i>("position");
    c.template member<Vector2i>("size");
}

/// Helper to register Plane layout for any type T that matches Plane layout
template <typename T>
void register_plane_members(flecs::component<T> c) {
    c.template member<Vector3>("normal");
    c.template member<real_t>("d");
}

/// Helper to register Quaternion layout for any type T that matches Quaternion layout
template <typename T>
void register_quaternion_members(flecs::component<T> c) {
    c.template member<real_t>("x");
    c.template member<real_t>("y");
    c.template member<real_t>("z");
    c.template member<real_t>("w");
}

/// Helper to register Basis layout for any type T that matches Basis layout
template <typename T>
void register_basis_members(flecs::component<T> c) {
    c.template member<Vector3>("rows", 3);
}

/// Helper to register Transform2D layout for any type T that matches Transform2D layout
template <typename T>
void register_transform2d_members(flecs::component<T> c) {
    c.template member<Vector2>("columns", 3);
}

/// Helper to register Transform3D layout for any type T that matches Transform3D layout
template <typename T>
void register_transform3d_members(flecs::component<T> c) {
    c.template member<Basis>("basis");
    c.template member<Vector3>("origin");
}

/// Helper to register AABB layout for any type T that matches AABB layout
template <typename T>
void register_aabb_members(flecs::component<T> c) {
    c.template member<Vector3>("position");
    c.template member<Vector3>("size");
}

/// Helper to register Projection layout for any type T that matches Projection layout
template <typename T>
void register_projection_members(flecs::component<T> c) {
    c.template member<Vector4>("columns", 4);
}


/// Macro to define a component that wraps a Godot type, inheriting constructors and assignment.
/// Components are defined in the namespace where the macro is invoked.
#define GODOT_VARIANT(Name, Base, ...) \
    struct Name : public Base { \
        using base_type = Base; \
        using base_type::base_type; \
        Name() : Base(__VA_ARGS__) {} \
        Name(const Base& other) : Base(other) {} \
        Name& operator=(const Base& other) { \
            Base::operator=(other); \
            return *this; \
        } \
    }; \
    inline auto register_##Name##_variant = stagehand::ComponentRegistrar<Name>([](flecs::world& world) { \
        register_godot_members(world.component<Name>(), static_cast<Base*>(nullptr)); \
        stagehand::register_component_getter<Name, Base>(#Name); \
        stagehand::register_component_setter<Name, Base>(#Name); \
    })

// Dispatcher overloads to automatically select the correct registration function
// Struct type
template <typename T> void register_godot_members(flecs::component<T> c, Color*) { register_color_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector2*) { register_vector2_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector2i*) { register_vector2i_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector3*) { register_vector3_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector3i*) { register_vector3i_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector4*) { register_vector4_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Vector4i*) { register_vector4i_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Rect2*) { register_rect2_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Rect2i*) { register_rect2i_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Plane*) { register_plane_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Quaternion*) { register_quaternion_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Basis*) { register_basis_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Transform2D*) { register_transform2d_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Transform3D*) { register_transform3d_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, AABB*) { register_aabb_members(c); }
template <typename T> void register_godot_members(flecs::component<T> c, Projection*) { register_projection_members(c); }

// Class types
template <typename T> void register_godot_members(flecs::component<T> c, Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, Dictionary*) {}
template <typename T> void register_godot_members(flecs::component<T> c, String*) {}
template <typename T> void register_godot_members(flecs::component<T> c, StringName*) {}
template <typename T> void register_godot_members(flecs::component<T> c, NodePath*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedByteArray*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedColorArray*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedFloat32Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedFloat64Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedInt32Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedInt64Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedStringArray*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedVector2Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedVector3Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, PackedVector4Array*) {}
template <typename T> void register_godot_members(flecs::component<T> c, Callable*) {}
template <typename T> void register_godot_members(flecs::component<T> c, RID*) {}
template <typename T> void register_godot_members(flecs::component<T> c, Signal*) {}

inline stagehand::Registry register_godot_variant_components([](flecs::world& world)
{
    register_color_members(world.component<Color>()); // 16 bytes
    stagehand::register_component_getter<Color>("Color");
    stagehand::register_component_setter<Color>("Color");
    register_vector2_members(world.component<Vector2>()); // 8 bytes
    stagehand::register_component_getter<Vector2>("Vector2");
    stagehand::register_component_setter<Vector2>("Vector2");
    register_vector2i_members(world.component<Vector2i>()); // 8 bytes
    stagehand::register_component_getter<Vector2i>("Vector2i");
    stagehand::register_component_setter<Vector2i>("Vector2i");
    register_vector3_members(world.component<Vector3>()); // 12 bytes
    stagehand::register_component_getter<Vector3>("Vector3");
    stagehand::register_component_setter<Vector3>("Vector3");
    register_vector3i_members(world.component<Vector3i>()); // 12 bytes
    stagehand::register_component_getter<Vector3i>("Vector3i");
    stagehand::register_component_setter<Vector3i>("Vector3i");
    register_vector4_members(world.component<Vector4>()); // 16 bytes
    stagehand::register_component_getter<Vector4>("Vector4");
    stagehand::register_component_setter<Vector4>("Vector4");
    register_vector4i_members(world.component<Vector4i>()); // 16 bytes
    stagehand::register_component_getter<Vector4i>("Vector4i");
    stagehand::register_component_setter<Vector4i>("Vector4i");
    register_rect2_members(world.component<Rect2>()); // 16 bytes
    stagehand::register_component_getter<Rect2>("Rect2");
    stagehand::register_component_setter<Rect2>("Rect2");
    register_rect2i_members(world.component<Rect2i>()); // 16 bytes
    stagehand::register_component_getter<Rect2i>("Rect2i");
    stagehand::register_component_setter<Rect2i>("Rect2i");
    register_plane_members(world.component<Plane>()); // 16 bytes
    stagehand::register_component_getter<Plane>("Plane");
    stagehand::register_component_setter<Plane>("Plane");
    register_quaternion_members(world.component<Quaternion>()); // 16 bytes
    stagehand::register_component_getter<Quaternion>("Quaternion");
    stagehand::register_component_setter<Quaternion>("Quaternion");
    register_basis_members(world.component<Basis>()); // 36 bytes - acceptable
    stagehand::register_component_getter<Basis>("Basis");
    stagehand::register_component_setter<Basis>("Basis");
    register_transform2d_members(world.component<Transform2D>()); // 24 bytes
    stagehand::register_component_getter<Transform2D>("Transform2D");
    stagehand::register_component_setter<Transform2D>("Transform2D");
    register_transform3d_members(world.component<Transform3D>()); // 48 bytes - borderline large, but acceptable for transform components
    stagehand::register_component_getter<Transform3D>("Transform3D");
    stagehand::register_component_setter<Transform3D>("Transform3D");
    register_aabb_members(world.component<AABB>()); // 24 bytes
    stagehand::register_component_getter<AABB>("AABB");
    stagehand::register_component_setter<AABB>("AABB");
    register_projection_members(world.component<Projection>()); // 64 bytes - large, use sparingly
    stagehand::register_component_getter<Projection>("Projection");
    stagehand::register_component_setter<Projection>("Projection");

    // Class types
    world.component<Array>();
    stagehand::register_component_getter<Array>("Array");
    stagehand::register_component_setter<Array>("Array");
    world.component<Dictionary>();
    stagehand::register_component_getter<Dictionary>("Dictionary");
    stagehand::register_component_setter<Dictionary>("Dictionary");
    world.component<String>();
    stagehand::register_component_getter<String>("String");
    stagehand::register_component_setter<String>("String");
    world.component<StringName>();
    stagehand::register_component_getter<StringName>("StringName");
    stagehand::register_component_setter<StringName>("StringName");
    world.component<NodePath>();
    stagehand::register_component_getter<NodePath>("NodePath");
    stagehand::register_component_setter<NodePath>("NodePath");
    world.component<PackedByteArray>();
    stagehand::register_component_getter<PackedByteArray>("PackedByteArray");
    stagehand::register_component_setter<PackedByteArray>("PackedByteArray");
    world.component<PackedColorArray>();
    stagehand::register_component_getter<PackedColorArray>("PackedColorArray");
    stagehand::register_component_setter<PackedColorArray>("PackedColorArray");
    world.component<PackedFloat32Array>();
    stagehand::register_component_getter<PackedFloat32Array>("PackedFloat32Array");
    stagehand::register_component_setter<PackedFloat32Array>("PackedFloat32Array");
    world.component<PackedFloat64Array>();
    stagehand::register_component_getter<PackedFloat64Array>("PackedFloat64Array");
    stagehand::register_component_setter<PackedFloat64Array>("PackedFloat64Array");
    world.component<PackedInt32Array>();
    stagehand::register_component_getter<PackedInt32Array>("PackedInt32Array");
    stagehand::register_component_setter<PackedInt32Array>("PackedInt32Array");
    world.component<PackedInt64Array>();
    stagehand::register_component_getter<PackedInt64Array>("PackedInt64Array");
    stagehand::register_component_setter<PackedInt64Array>("PackedInt64Array");
    world.component<PackedStringArray>();
    stagehand::register_component_getter<PackedStringArray>("PackedStringArray");
    stagehand::register_component_setter<PackedStringArray>("PackedStringArray");
    world.component<PackedVector2Array>();
    stagehand::register_component_getter<PackedVector2Array>("PackedVector2Array");
    stagehand::register_component_setter<PackedVector2Array>("PackedVector2Array");
    world.component<PackedVector3Array>();
    stagehand::register_component_getter<PackedVector3Array>("PackedVector3Array");
    stagehand::register_component_setter<PackedVector3Array>("PackedVector3Array");
    world.component<PackedVector4Array>();
    stagehand::register_component_getter<PackedVector4Array>("PackedVector4Array");
    stagehand::register_component_setter<PackedVector4Array>("PackedVector4Array");
    world.component<Callable>();
    stagehand::register_component_getter<Callable>("Callable");
    stagehand::register_component_setter<Callable>("Callable");
    world.component<RID>();
    stagehand::register_component_getter<RID>("RID");
    stagehand::register_component_setter<RID>("RID");
    world.component<Signal>();
    stagehand::register_component_getter<Signal>("Signal");
    stagehand::register_component_setter<Signal>("Signal");
});
