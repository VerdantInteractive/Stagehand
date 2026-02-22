#pragma once

#include <cstdint>

using std::int16_t;
using std::int32_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint8_t;

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
#define FLOAT(Name, ...)                                                                                                                                       \
    struct Name {                                                                                                                                              \
        float value{__VA_ARGS__};                                                                                                                              \
        Name() = default;                                                                                                                                      \
        Name(float v) : value(v) {}                                                                                                                            \
        NUMERIC_COMPONENT_OPERATORS(Name, float)                                                                                                               \
    };                                                                                                                                                         \
    inline auto register_##Name##_float = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                        \
        world.component<Name>().member<float>("value");                                                                                                        \
        stagehand::register_component<Name, float>(#Name);                                                                                                     \
    })

/// Macro that defines a component wrapping a double-precision floating-point number.
#define DOUBLE(Name, ...)                                                                                                                                      \
    struct Name {                                                                                                                                              \
        double value{__VA_ARGS__};                                                                                                                             \
        Name() = default;                                                                                                                                      \
        Name(double v) : value(v) {}                                                                                                                           \
        NUMERIC_COMPONENT_OPERATORS(Name, double)                                                                                                              \
    };                                                                                                                                                         \
    inline auto register_##Name##_double = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                       \
        world.component<Name>().member<double>("value");                                                                                                       \
        stagehand::register_component<Name, double>(#Name);                                                                                                    \
    })

/// Macro that defines a component wrapping a signed 32-bit integer.
#define INT32(Name, ...)                                                                                                                                       \
    struct Name {                                                                                                                                              \
        int32_t value{__VA_ARGS__};                                                                                                                            \
        Name() = default;                                                                                                                                      \
        Name(int32_t v) : value(v) {}                                                                                                                          \
        NUMERIC_COMPONENT_OPERATORS(Name, int32_t)                                                                                                             \
    };                                                                                                                                                         \
    inline auto register_##Name##_int32 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                        \
        world.component<Name>().member<int32_t>("value");                                                                                                      \
        stagehand::register_component<Name, int32_t>(#Name);                                                                                                   \
    })

/// Macro that defines a component wrapping an unsigned 32-bit integer.
#define UINT32(Name, ...)                                                                                                                                      \
    struct Name {                                                                                                                                              \
        uint32_t value{__VA_ARGS__};                                                                                                                           \
        Name() = default;                                                                                                                                      \
        Name(uint32_t v) : value(v) {}                                                                                                                         \
        NUMERIC_COMPONENT_OPERATORS(Name, uint32_t)                                                                                                            \
    };                                                                                                                                                         \
    inline auto register_##Name##_uint32 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                       \
        world.component<Name>().member<uint32_t>("value");                                                                                                     \
        stagehand::register_component<Name, uint32_t>(#Name);                                                                                                  \
    })

/// Macro that defines a component wrapping a signed 16-bit integer (-32,768 to 32,767).
#define INT16(Name, ...)                                                                                                                                       \
    struct Name {                                                                                                                                              \
        int16_t value{__VA_ARGS__};                                                                                                                            \
        Name() = default;                                                                                                                                      \
        Name(int16_t v) : value(v) {}                                                                                                                          \
        NUMERIC_COMPONENT_OPERATORS(Name, int16_t)                                                                                                             \
    };                                                                                                                                                         \
    inline auto register_##Name##_int16 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                        \
        world.component<Name>().member<int16_t>("value");                                                                                                      \
        stagehand::register_component<Name, int16_t>(#Name);                                                                                                   \
    })

/// Macro that defines a component wrapping an unsigned 16-bit integer (0 to 65,535).
#define UINT16(Name, ...)                                                                                                                                      \
    struct Name {                                                                                                                                              \
        uint16_t value{__VA_ARGS__};                                                                                                                           \
        Name() = default;                                                                                                                                      \
        Name(uint16_t v) : value(v) {}                                                                                                                         \
        NUMERIC_COMPONENT_OPERATORS(Name, uint16_t)                                                                                                            \
    };                                                                                                                                                         \
    inline auto register_##Name##_uint16 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                       \
        world.component<Name>().member<uint16_t>("value");                                                                                                     \
        stagehand::register_component<Name, uint16_t>(#Name);                                                                                                  \
    })

/// Macro that defines a component wrapping a signed 8-bit integer (-128 to 127).
#define INT8(Name, ...)                                                                                                                                        \
    struct Name {                                                                                                                                              \
        int8_t value{__VA_ARGS__};                                                                                                                             \
        Name() = default;                                                                                                                                      \
        Name(int8_t v) : value(v) {}                                                                                                                           \
        NUMERIC_COMPONENT_OPERATORS(Name, int8_t)                                                                                                              \
    };                                                                                                                                                         \
    inline auto register_##Name##_int8 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                         \
        world.component<Name>().member<int8_t>("value");                                                                                                       \
        stagehand::register_component<Name, int8_t>(#Name);                                                                                                    \
    })

/// Macro that defines a component wrapping an unsigned 8-bit integer (0 to 255).
#define UINT8(Name, ...)                                                                                                                                       \
    struct Name {                                                                                                                                              \
        uint8_t value{__VA_ARGS__};                                                                                                                            \
        Name() = default;                                                                                                                                      \
        Name(uint8_t v) : value(v) {}                                                                                                                          \
        NUMERIC_COMPONENT_OPERATORS(Name, uint8_t)                                                                                                             \
    };                                                                                                                                                         \
    inline auto register_##Name##_uint8 = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                        \
        world.component<Name>().member<uint8_t>("value");                                                                                                      \
        stagehand::register_component<Name, uint8_t>(#Name);                                                                                                   \
    })

/// Macro that defines a component wrapping a pointer type.
#define POINTER(Name, Type, ...)                                                                                                                               \
    struct Name {                                                                                                                                              \
        Type *ptr{__VA_ARGS__};                                                                                                                                \
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
    inline auto register_##Name##_pointer = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                      \
        world.component<Name>().member<std::uintptr_t>("ptr");                                                                                                 \
        stagehand::register_component<Name, uint64_t>(#Name);                                                                                                  \
    })

/// Macro that defines a tag component (empty struct).
#define TAG(Name)                                                                                                                                              \
    struct Name {};                                                                                                                                            \
    inline auto register_##Name##_tag = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                          \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component_inspector<Name>(#Name);                                                                                                  \
    })

/// Macro that defines an enum component wrapper.
/// Usage: ENUM(Name) or ENUM(Name, UnderlyingType). Default UnderlyingType is uint8_t.
#define ENUM_IMPL(Name, Type)                                                                                                                                  \
    inline auto register_##Name##_enum = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                         \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component<Name, Type>(#Name);                                                                                                      \
    })

#define ENUM_1(Name) ENUM_IMPL(Name, uint8_t)
#define ENUM_2(Name, Type) ENUM_IMPL(Name, Type)
#define GET_ENUM_MACRO(_1, _2, NAME, ...) NAME
#define ENUM(...) GET_ENUM_MACRO(__VA_ARGS__, ENUM_2, ENUM_1)(__VA_ARGS__)

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
#define VECTOR(Name, ElementType, ...)                                                                                                                         \
    struct Name {                                                                                                                                              \
        std::vector<ElementType> value{__VA_ARGS__};                                                                                                           \
        CONTAINER_COMPONENT_BODY(Name, ElementType, std::vector<ElementType>)                                                                                  \
        std::size_t size() const { return value.size(); }                                                                                                      \
    };                                                                                                                                                         \
    inline auto register_##Name##_vector = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                       \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component<Name>(#Name);                                                                                                            \
    })

/// Macro that defines a component wrapping a std::array.
/// @param Name The name of the component struct.
/// @param ElementType The type of elements in the array.
/// @param Size The size of the array (must be a compile-time constant).
/// @param ... Optional initializer for the array (e.g., {1, 2, 3}).
///
/// Example: ARRAY(MyArrayComponent, int, 5, {10, 20, 30, 40, 50})
#define ARRAY(Name, ElementType, Size, ...)                                                                                                                    \
    struct Name {                                                                                                                                              \
        std::array<ElementType, Size> value{__VA_ARGS__};                                                                                                      \
        CONTAINER_COMPONENT_BODY(Name, ElementType, std::array<ElementType, Size>)                                                                             \
        constexpr std::size_t size() const { return Size; }                                                                                                    \
    };                                                                                                                                                         \
    inline auto register_##Name##_array = stagehand::ComponentRegistrar<Name>([](flecs::world &world) {                                                        \
        world.component<Name>();                                                                                                                               \
        stagehand::register_component<Name>(#Name);                                                                                                            \
    })
