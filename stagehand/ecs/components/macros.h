#pragma once

#include <cstdint>
#include <memory>
#include <pfr/core.hpp>
#include <pfr/core_name.hpp>
#include <pfr/tuple_size.hpp>
#include <string>
#include <utility>

#include "stagehand/ecs/components/traits.h" // IWYU pragma: keep

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/// Macro that defines common operators for numeric component wrappers.
#define NUMERIC_COMPONENT_OPERATORS(Name, Type)                                                                                                                \
    operator Type &() { return value; }                                                                                                                        \
    operator Type() const { return value; }                                                                                                                    \
    Name &operator=(Type v) {                                                                                                                                  \
        value = v;                                                                                                                                             \
        return *this;                                                                                                                                          \
    }                                                                                                                                                          \
    Name &operator++() { /* pre-increment */                                                                                                                   \
        ++value;                                                                                                                                               \
        return *this;                                                                                                                                          \
    }                                                                                                                                                          \
    Name operator++(int) { /* post-increment */                                                                                                                \
        Name temp = *this;                                                                                                                                     \
        ++value;                                                                                                                                               \
        return temp;                                                                                                                                           \
    }                                                                                                                                                          \
    Name &operator--() { /* pre-decrement */                                                                                                                   \
        --value;                                                                                                                                               \
        return *this;                                                                                                                                          \
    }                                                                                                                                                          \
    Name operator--(int) { /* post-decrement */                                                                                                                \
        Name temp = *this;                                                                                                                                     \
        --value;                                                                                                                                               \
        return temp;                                                                                                                                           \
    }

/// Macro that defines a component wrapping a single-precision floating-point number.
#define STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, Type, RegisterSuffix, ChangeTagDecl, ChangeTagAlias, ...)                                                       \
    struct Name;                                                                                                                                               \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    ChangeTagDecl struct Name {                                                                                                                                \
        ChangeTagAlias Type value{__VA_ARGS__};                                                                                                                \
        Name() = default;                                                                                                                                      \
        Name(Type v) : value(v) {}                                                                                                                             \
        NUMERIC_COMPONENT_OPERATORS(Name, Type)                                                                                                                \
    };                                                                                                                                                         \
    inline flecs::entity operator<<(flecs::entity e, const Name &value) {                                                                                      \
        e.set<Name>(value);                                                                                                                                    \
        stagehand::internal::mark_component_changed_if_needed<Name>(e);                                                                                        \
        return e;                                                                                                                                              \
    }                                                                                                                                                          \
    inline auto register_##Name##_##RegisterSuffix = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                             \
        world.component<Name>().member<Type>("value");                                                                                                         \
        stagehand::register_component_with_world_name<Name, Type>(world, #Name);                                                                               \
        stagehand::internal::register_change_detection_if_needed<Name>(world);                                                                                 \
    })

#define FLOAT(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, float, float, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define FLOAT_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, float, float_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a double-precision floating-point number.
#define DOUBLE(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, double, double, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define DOUBLE_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, double, double_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a signed 8-bit integer (-128 to 127).
#define INT8(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int8_t, int8, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define INT8_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int8_t, int8_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping an unsigned 8-bit integer (0 to 255).
#define UINT8(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint8_t, uint8, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define UINT8_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint8_t, uint8_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a signed 16-bit integer (-32,768 to 32,767).
#define INT16(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int16_t, int16, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define INT16_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int16_t, int16_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping an unsigned 16-bit integer (0 to 65,535).
#define UINT16(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint16_t, uint16, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define UINT16_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint16_t, uint16_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a signed 32-bit integer.
#define INT32(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int32_t, int32, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define INT32_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int32_t, int32_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping an unsigned 32-bit integer.
#define UINT32(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint32_t, uint32, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define UINT32_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint32_t, uint32_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a signed 64-bit integer.
#define INT64(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int64_t, int64, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define INT64_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, int64_t, int64_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping an unsigned 64-bit integer.
#define UINT64(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint64_t, uint64, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define UINT64_(Name, ...) STAGEHAND_NUMERIC_COMPONENT_IMPL(Name, uint64_t, uint64_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a pointer type.
#define STAGEHAND_POINTER_COMPONENT_IMPL(Name, Type, RegisterSuffix, ChangeTagDecl, ChangeTagAlias, ...)                                                       \
    struct Name;                                                                                                                                               \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    ChangeTagDecl struct Name {                                                                                                                                \
        ChangeTagAlias Type *ptr{__VA_ARGS__};                                                                                                                 \
        Name() = default;                                                                                                                                      \
        Name(Type *p) : ptr(p) {}                                                                                                                              \
        Name(std::uintptr_t p) : ptr(reinterpret_cast<Type *>(p)) {}                                                                                           \
        operator Type *&() { return ptr; }                                                                                                                     \
        operator Type *() const { return ptr; }                                                                                                                \
        operator std::uintptr_t() const { return reinterpret_cast<std::uintptr_t>(ptr); }                                                                      \
        Name &operator=(Type *p) {                                                                                                                             \
            ptr = p;                                                                                                                                           \
            return *this;                                                                                                                                      \
        }                                                                                                                                                      \
        Type *operator->() const { return ptr; }                                                                                                               \
        bool operator==(const Name &other) const { return ptr == other.ptr; }                                                                                  \
        bool operator!=(const Name &other) const { return ptr != other.ptr; }                                                                                  \
        explicit operator bool() const { return ptr != nullptr; }                                                                                              \
    };                                                                                                                                                         \
    inline flecs::entity operator<<(flecs::entity e, const Name &value) {                                                                                      \
        e.set<Name>(value);                                                                                                                                    \
        stagehand::internal::mark_component_changed_if_needed<Name>(e);                                                                                        \
        return e;                                                                                                                                              \
    }                                                                                                                                                          \
    inline auto register_##Name##_##RegisterSuffix = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                             \
        world.component<Name>().member<std::uintptr_t>("ptr");                                                                                                 \
        stagehand::register_component_with_world_name<Name, uint64_t>(world, #Name);                                                                           \
        stagehand::internal::register_change_detection_if_needed<Name>(world);                                                                                 \
    })

#define POINTER(Name, Type, ...)                                                                                                                               \
    STAGEHAND_POINTER_COMPONENT_IMPL(Name, Type, pointer, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define POINTER_(Name, Type, ...) STAGEHAND_POINTER_COMPONENT_IMPL(Name, Type, pointer_no_change, , , __VA_ARGS__)

/// Macro that defines a tag component (empty struct).
#define TAG(Name)                                                                                                                                              \
    struct Name {};                                                                                                                                            \
    inline auto register_##Name##_tag = stagehand::ComponentRegistrar<Name>([](flecs::world &world) { world.component<Name>(); })
// Change detection doesn't apply to tags
#define TAG_(Name) TAG(Name)

/// Macro that defines an enum component wrapper.
/// Usage: ENUM(Name) or ENUM(Name, UnderlyingType). Default UnderlyingType is uint8_t.
#define ENUM_IMPL(Name, Type)                                                                                                                                  \
    struct HasChanged##Name {};                                                                                                                                \
    inline auto register_##Name##_enum = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                         \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component_with_world_name<Name, Type>(world, #Name);                                                                               \
        stagehand::internal::register_change_detection_for_component<Name, HasChanged##Name>(world);                                                           \
    })

#define ENUM_IMPL_(Name, Type)                                                                                                                                 \
    inline auto register_##Name##_enum_no_change = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                               \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component_with_world_name<Name, Type>(world, #Name);                                                                               \
    })

#define ENUM_1(Name) ENUM_IMPL(Name, uint8_t)
#define ENUM_2(Name, Type) ENUM_IMPL(Name, Type)
#define GET_ENUM_MACRO(_1, _2, NAME, ...) NAME
#define ENUM(...) GET_ENUM_MACRO(__VA_ARGS__, ENUM_2, ENUM_1)(__VA_ARGS__)

#define ENUM__1(Name) ENUM_IMPL_(Name, uint8_t)
#define ENUM__2(Name, Type) ENUM_IMPL_(Name, Type)
#define GET_ENUM_MACRO_(_1, _2, NAME, ...) NAME
#define ENUM_(...) GET_ENUM_MACRO_(__VA_ARGS__, ENUM__2, ENUM__1)(__VA_ARGS__)

/// Macros that wrap various std:: container types
/// The components work fully with Flecs ECS operations (add, remove, get, queries, systems).
/// Godot getter/setter functions are provided for GDScript integration.
/// This macro defines the common boilerplate for container-like components.
#define CONTAINER_COMPONENT_BODY(Name, ElementType, ...)                                                                                                       \
    using ContainerType = __VA_ARGS__;                                                                                                                         \
    Name() = default;                                                                                                                                          \
    Name(const ContainerType &v) : value(v) {}                                                                                                                 \
    Name(ContainerType &&v) : value(std::move(v)) {}                                                                                                           \
    Name &operator=(const ContainerType &v) {                                                                                                                  \
        value = v;                                                                                                                                             \
        return *this;                                                                                                                                          \
    }                                                                                                                                                          \
    Name &operator=(ContainerType &&v) {                                                                                                                       \
        value = std::move(v);                                                                                                                                  \
        return *this;                                                                                                                                          \
    }                                                                                                                                                          \
    ElementType &operator[](std::size_t i) { return value[i]; }                                                                                                \
    const ElementType &operator[](std::size_t i) const { return value[i]; }                                                                                    \
    auto begin() { return value.begin(); }                                                                                                                     \
    auto end() { return value.end(); }                                                                                                                         \
    auto begin() const { return value.begin(); }                                                                                                               \
    auto end() const { return value.end(); }

/// Macro that defines a component wrapping a std::vector.
/// @param Name The name of the component struct.
/// @param ElementType The type of elements in the vector.
/// @param ... Optional initializer for the vector (e.g., {1, 2, 3}).
///
/// Example: VECTOR(MyVectorComponent, float, {1.0f, 2.0f, 3.0f})
#define STAGEHAND_VECTOR_COMPONENT_IMPL(Name, ElementType, RegisterSuffix, ChangeTagDecl, ChangeTagAlias, ...)                                                 \
    struct Name;                                                                                                                                               \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    ChangeTagDecl struct Name {                                                                                                                                \
        ChangeTagAlias std::vector<ElementType> value{__VA_ARGS__};                                                                                            \
        CONTAINER_COMPONENT_BODY(Name, ElementType, std::vector<ElementType>)                                                                                  \
        std::size_t size() const { return value.size(); }                                                                                                      \
    };                                                                                                                                                         \
    inline flecs::entity operator<<(flecs::entity e, const Name &value) {                                                                                      \
        e.set<Name>(value);                                                                                                                                    \
        stagehand::internal::mark_component_changed_if_needed<Name>(e);                                                                                        \
        return e;                                                                                                                                              \
    }                                                                                                                                                          \
    inline auto register_##Name##_##RegisterSuffix = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                             \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component_with_world_name<Name>(world, #Name);                                                                                     \
        stagehand::internal::register_change_detection_if_needed<Name>(world);                                                                                 \
    })

#define VECTOR(Name, ElementType, ...)                                                                                                                         \
    STAGEHAND_VECTOR_COMPONENT_IMPL(Name, ElementType, vector, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define VECTOR_(Name, ElementType, ...) STAGEHAND_VECTOR_COMPONENT_IMPL(Name, ElementType, vector_no_change, , , __VA_ARGS__)

/// Macro that defines a component wrapping a std::array.
/// @param Name The name of the component struct.
/// @param ElementType The type of elements in the array.
/// @param Size The size of the array (must be a compile-time constant).
/// @param ... Optional initializer for the array (e.g., {1, 2, 3}).
///
/// Example: ARRAY(MyArrayComponent, int, 5, {10, 20, 30, 40, 50})
#define STAGEHAND_ARRAY_COMPONENT_IMPL(Name, ElementType, Size, RegisterSuffix, ChangeTagDecl, ChangeTagAlias, ...)                                            \
    struct Name;                                                                                                                                               \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    ChangeTagDecl struct Name {                                                                                                                                \
        ChangeTagAlias std::array<ElementType, Size> value{__VA_ARGS__};                                                                                       \
        CONTAINER_COMPONENT_BODY(Name, ElementType, std::array<ElementType, Size>)                                                                             \
        constexpr std::size_t size() const { return Size; }                                                                                                    \
    };                                                                                                                                                         \
    inline flecs::entity operator<<(flecs::entity e, const Name &value) {                                                                                      \
        e.set<Name>(value);                                                                                                                                    \
        stagehand::internal::mark_component_changed_if_needed<Name>(e);                                                                                        \
        return e;                                                                                                                                              \
    }                                                                                                                                                          \
    inline auto register_##Name##_##RegisterSuffix = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                             \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component_with_world_name<Name>(world, #Name);                                                                                     \
        stagehand::internal::register_change_detection_if_needed<Name>(world);                                                                                 \
    })

#define ARRAY(Name, ElementType, Size, ...)                                                                                                                    \
    STAGEHAND_ARRAY_COMPONENT_IMPL(Name, ElementType, Size, array, struct HasChanged##Name{};, using ChangeTag = HasChanged##Name;, __VA_ARGS__)
#define ARRAY_(Name, ElementType, Size, ...) STAGEHAND_ARRAY_COMPONENT_IMPL(Name, ElementType, Size, array_no_change, , , __VA_ARGS__)

// ═══════════════════════════════════════════════════════════════════════════════
// Struct component macros — PFR-based reflection for multi-field aggregates
// ═══════════════════════════════════════════════════════════════════════════════

namespace stagehand::internal {
    /// Registers all members of an aggregate struct with Flecs using PFR reflection.
    /// Includes explicit offset information for each member for proper reflection support.
    template <typename T> void register_pfr_members(flecs::world &world) {
        T instance{};
        const uintptr_t instance_addr = reinterpret_cast<uintptr_t>(std::addressof(instance));
        flecs::component<T> comp = world.component<T>();
        
        pfr::for_each_field_with_name(instance, [&comp, instance_addr](std::string_view name, const auto &field) {
            using FieldType = std::remove_cvref_t<decltype(field)>;
            const std::string name_str(name);
            const uintptr_t field_addr = reinterpret_cast<uintptr_t>(std::addressof(field));
            size_t offset = field_addr - instance_addr;
            comp.template member<FieldType>(name_str.c_str(), 0, offset);
        });
    }
} // namespace stagehand::internal

namespace stagehand {
    /// Registers a PFR-reflectable struct as a Flecs component with member metadata
    /// and Dictionary-based getter/setter for GDScript integration.
    template <typename T> void register_struct_component(flecs::world &world, const char *fallback_name) {
        internal::register_pfr_members<T>(world);

        const flecs::component<T> component_handle = world.component<T>();
        const flecs::entity_t comp_id = component_handle.id();

        ComponentGetter getter = [](const flecs::world &w, flecs::entity_t eid) -> godot::Variant {
            const T *data = nullptr;
            if (eid == 0) {
                data = w.try_get<T>();
            } else {
                if (!w.is_alive(eid)) {
                    godot::UtilityFunctions::push_warning(godot::String("Get Component: Entity ") + godot::String::num_uint64(eid) + " is not alive.");
                    return godot::Variant();
                }
                data = flecs::entity(w, eid).try_get<T>();
            }
            if (data) {
                godot::Dictionary dict;
                pfr::for_each_field_with_name(
                    *data, [&dict](std::string_view name, const auto &field) { dict[godot::String(std::string(name).c_str())] = godot::Variant(field); });
                return godot::Variant(dict);
            }
            return godot::Variant(godot::Dictionary());
        };

        ComponentSetter setter = [component_name = std::string(fallback_name)](flecs::world &w, flecs::entity_t eid, const godot::Variant &v) {
            if (v.get_type() != godot::Variant::DICTIONARY) {
                godot::UtilityFunctions::push_warning(godot::String("Failed to set struct component '") + component_name.c_str() +
                                                      "'. Expected Dictionary, got " + godot::Variant::get_type_name(v.get_type()));
                return;
            }
            godot::Dictionary dict = v;
            T new_value{};
            pfr::for_each_field_with_name(new_value, [&dict](std::string_view name, auto &field) {
                godot::String key(std::string(name).c_str());
                if (dict.has(key)) {
                    field = static_cast<std::remove_cvref_t<decltype(field)>>(godot::Variant(dict[key]));
                }
            });
            if (eid == 0) {
                w.set<T>(new_value);
            } else {
                if (!w.is_alive(eid)) {
                    godot::UtilityFunctions::push_warning(godot::String("Set Component: Entity ") + godot::String::num_uint64(eid) + " is not alive.");
                    return;
                }
                flecs::entity(w, eid).set<T>(new_value);
            }
        };

        flecs::string component_path = component_handle.path();
        std::string qualified_name = component_path.c_str() != nullptr ? std::string(component_path.c_str()) : std::string();
        qualified_name = normalize_registered_component_name(std::move(qualified_name));
        if (!qualified_name.empty()) {
            auto &entry = get_component_registry()[qualified_name];
            entry.getter = getter;
            entry.setter = setter;
            entry.data_type = "struct";
            entry.entity_id = comp_id;
        }

        auto &entry = get_component_registry()[std::string(fallback_name)];
        entry.getter = getter;
        entry.setter = setter;
        entry.data_type = "struct";
        entry.entity_id = comp_id;
    }
} // namespace stagehand

/// Macro that defines and registers a struct component with PFR-based reflection.
/// All fields of the struct are automatically registered as Flecs members for
/// web UI visibility. A Dictionary-based getter/setter is registered for GDScript.
///
/// STRUCT registers with change detection. STRUCT_ registers without change detection.
///
/// The struct must be an aggregate (no user-declared constructors, no virtual functions,
/// no private/protected non-static data members).
///
/// Example:
///   STRUCT_(PlayerSettings, {
///       float speed = 5.0f;
///       int health = 100;
///   }).then([](auto c) { c.add(flecs::Singleton); });
#define STRUCT(Name, ...)                                                                                                                                      \
    struct Name __VA_ARGS__;                                                                                                                                   \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    struct HasChanged##Name {};                                                                                                                                \
    inline auto register_##Name##_struct = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                       \
        stagehand::register_struct_component<Name>(world, #Name);                                                                                              \
        stagehand::internal::register_change_detection_for_component<Name, HasChanged##Name>(world);                                                           \
    })

#define STRUCT_(Name, ...)                                                                                                                                     \
    struct Name __VA_ARGS__;                                                                                                                                   \
    constexpr bool stagehand_auto_seed_singleton(Name *) { return true; }                                                                                      \
    inline auto register_##Name##_struct_no_change =                                                                                                           \
        stagehand::ComponentRegistrar<Name>([](flecs::world &world) { stagehand::register_struct_component<Name>(world, #Name); })
