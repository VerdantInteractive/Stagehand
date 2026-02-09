#pragma once

#include <cstdint>

#include "stagehand/registry.h"

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;


/// Macro that defines a component wrapping a single-precision floating-point number.
#define FLOAT(Name, ...) \
    struct Name { \
        float value{__VA_ARGS__}; \
        Name() = default; \
        Name(float v) : value(v) {} \
        operator float&() { return value; } \
        operator float() const { return value; } \
        Name& operator=(float v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_float([](flecs::world& world) { \
        world.component<Name>().member<float>("value"); \
        stagehand::register_component_getter<Name, float>(#Name); \
        stagehand::register_component_setter<Name, float>(#Name); \
    })


/// Macro that defines a component wrapping a double-precision floating-point number.
#define DOUBLE(Name, ...) \
    struct Name { \
        double value{__VA_ARGS__}; \
        Name() = default; \
        Name(double v) : value(v) {} \
        operator double&() { return value; } \
        operator double() const { return value; } \
        Name& operator=(double v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_double([](flecs::world& world) { \
        world.component<Name>().member<double>("value"); \
        stagehand::register_component_getter<Name, double>(#Name); \
        stagehand::register_component_setter<Name, double>(#Name); \
    })


/// Macro that defines a component wrapping a signed 32-bit integer.
#define INT32(Name, ...) \
    struct Name { \
        int32_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(int32_t v) : value(v) {} \
        operator int32_t&() { return value; } \
        operator int32_t() const { return value; } \
        Name& operator=(int32_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_int32([](flecs::world& world) { \
        world.component<Name>().member<int32_t>("value"); \
        stagehand::register_component_getter<Name, int32_t>(#Name); \
        stagehand::register_component_setter<Name, int32_t>(#Name); \
    })


/// Macro that defines a component wrapping an unsigned 32-bit integer.
#define UINT32(Name, ...) \
    struct Name { \
        uint32_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(uint32_t v) : value(v) {} \
        operator uint32_t&() { return value; } \
        operator uint32_t() const { return value; } \
        Name& operator=(uint32_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_uint32([](flecs::world& world) { \
        world.component<Name>().member<uint32_t>("value"); \
        stagehand::register_component_getter<Name, uint32_t>(#Name); \
        stagehand::register_component_setter<Name, uint32_t>(#Name); \
    })


/// Macro that defines a component wrapping a signed 16-bit integer (-32,768 to 32,767).
#define INT16(Name, ...) \
    struct Name { \
        int16_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(int16_t v) : value(v) {} \
        operator int16_t&() { return value; } \
        operator int16_t() const { return value; } \
        Name& operator=(int16_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_int16([](flecs::world& world) { \
        world.component<Name>().member<int16_t>("value"); \
        stagehand::register_component_getter<Name, int16_t>(#Name); \
        stagehand::register_component_setter<Name, int16_t>(#Name); \
    })


/// Macro that defines a component wrapping an unsigned 16-bit integer (0 to 65,535).
#define UINT16(Name, ...) \
    struct Name { \
        uint16_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(uint16_t v) : value(v) {} \
        operator uint16_t&() { return value; } \
        operator uint16_t() const { return value; } \
        Name& operator=(uint16_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_uint16([](flecs::world& world) { \
        world.component<Name>().member<uint16_t>("value"); \
        stagehand::register_component_getter<Name, uint16_t>(#Name); \
        stagehand::register_component_setter<Name, uint16_t>(#Name); \
    })


/// Macro that defines a component wrapping a signed 8-bit integer (-128 to 127).
#define INT8(Name, ...) \
    struct Name { \
        int8_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(int8_t v) : value(v) {} \
        operator int8_t&() { return value; } \
        operator int8_t() const { return value; } \
        Name& operator=(int8_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_int8([](flecs::world& world) { \
        world.component<Name>().member<int8_t>("value"); \
        stagehand::register_component_getter<Name, int8_t>(#Name); \
        stagehand::register_component_setter<Name, int8_t>(#Name); \
    })


/// Macro that defines a component wrapping an unsigned 8-bit integer (0 to 255).
#define UINT8(Name, ...) \
    struct Name { \
        uint8_t value{__VA_ARGS__}; \
        Name() = default; \
        Name(uint8_t v) : value(v) {} \
        operator uint8_t&() { return value; } \
        operator uint8_t() const { return value; } \
        Name& operator=(uint8_t v) { value = v; return *this; } \
    }; \
    inline stagehand::Registry register_##Name##_uint8([](flecs::world& world) { \
        world.component<Name>().member<uint8_t>("value"); \
        stagehand::register_component_getter<Name, uint8_t>(#Name); \
        stagehand::register_component_setter<Name, uint8_t>(#Name); \
    })


/// Macro that defines a component wrapping a pointer type.
#define POINTER(Name, Type, ...) \
    struct Name { \
        Type* ptr{__VA_ARGS__}; \
        Name() = default; \
        Name(Type* p) : ptr(p) {} \
        Name(std::uintptr_t p) : ptr(reinterpret_cast<Type*>(p)) {} \
        operator Type*&() { return ptr; } \
        operator Type*() const { return ptr; } \
        operator std::uintptr_t() const { return reinterpret_cast<std::uintptr_t>(ptr); } \
        Name& operator=(Type* p) { ptr = p; return *this; } \
        Type* operator->() const { return ptr; } \
        bool operator==(const Name& other) const { return ptr == other.ptr; } \
        bool operator!=(const Name& other) const { return ptr != other.ptr; } \
        explicit operator bool() const { return ptr != nullptr; } \
    }; \
    inline stagehand::Registry register_##Name##_pointer([](flecs::world& world) { \
        world.component<Name>().member<std::uintptr_t>("ptr"); \
        stagehand::register_component_getter<Name, std::uintptr_t>(#Name); \
        stagehand::register_component_setter<Name, std::uintptr_t>(#Name); \
    })


/// Macro that defines a tag component (empty struct).
#define TAG(Name) \
    struct Name {}; \
    inline stagehand::Registry register_##Name##_tag([](flecs::world& world) { \
        world.component<Name>(); \
    })
