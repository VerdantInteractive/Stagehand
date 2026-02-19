#pragma once

#include <array>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "flecs.h"

namespace stagehand {

    /// Callback function type for registering components and systems with the Flecs world.
    using RegistrationCallback = std::function<void(flecs::world &)>;

    /// Call all registered callbacks to configure the ECS world.
    /// @param world The Flecs world to configure.
    void register_components_and_systems_with_world(flecs::world &world);

    /// Run registration callbacks that were registered for a specific module.
    /// These callbacks are executed when the module is imported/loaded into the given world.
    void run_module_callbacks_for(flecs::world &world, const std::string &module_name);

    /// Returns true if any module-scoped callbacks were registered for
    /// `module_name` in this process.
    bool has_module_callbacks_for(const std::string &module_name);

    /// Register a callback to be called during world initialization.
    /// @param callback The callback to register.
    void register_callback(RegistrationCallback callback);

    /// Helper struct for static auto-registration from translation units.
    /// Instantiating this struct with a callback will register it to be called during world initialization.
    struct Registry {
        explicit Registry(RegistrationCallback callback);
        // Construct a Registry that runs the callback inside a Flecs module
        // with the given name. The module entity will be created (idempotent)
        // and the world's scope will be temporarily set to that module
        // while the callback executes.
        Registry(const char *module_name, RegistrationCallback callback);
    };

// Helper macros to create inline, translation-unit-unique registration objects.
// Usage:
//   REGISTER([](flecs::world &w){ ... });
//   REGISTER_IN_MODULE(stagehand::transform, [](flecs::world &w){ ... });
#define STAGEHAND_CONCAT2(a, b) a##b
#define STAGEHAND_CONCAT(a, b) STAGEHAND_CONCAT2(a, b)
#define STAGEHAND_CONCAT3(a, b, c) STAGEHAND_CONCAT(a, STAGEHAND_CONCAT(b, c))
#if defined(__COUNTER__)
#define STAGEHAND_UNIQUE_NAME(prefix) STAGEHAND_CONCAT3(prefix, __COUNTER__, __LINE__)
#else
#define STAGEHAND_UNIQUE_NAME(prefix) STAGEHAND_CONCAT(prefix, __LINE__)
#endif
#define REGISTER(...) inline stagehand::Registry STAGEHAND_UNIQUE_NAME(_stagehand_reg_)(__VA_ARGS__)
#define REGISTER_IN_MODULE(module, ...) inline stagehand::Registry STAGEHAND_UNIQUE_NAME(_stagehand_reg_)(#module, __VA_ARGS__)

    /// Template class for component registration with deferred chaining support.
    /// Used by component definition macros to allow configuring the flecs::component<T>
    /// via the general-purpose then() method, e.g.:
    ///     FLOAT(Foo).then([](auto c) { c.on_add([](Foo& f) { f.value = 1.0f; }); });
    ///     GODOT_VARIANT(Bar, Vector2).then([](auto c) { c.add(flecs::Singleton); });
    template <typename T> class ComponentRegistrar {
      public:
        explicit ComponentRegistrar(RegistrationCallback base_callback) { register_callback(std::move(base_callback)); }

        /// Chain an arbitrary callable that receives flecs::component<T>.
        /// This is the single general-purpose chaining mechanism — any operation
        /// available on flecs::component<T> can be performed inside the callable.
        /// Multiple then() calls can be chained; each is executed in order during
        /// world initialization.
        /// @param f A callable taking flecs::component<T>.
        template <typename F> ComponentRegistrar &then(F &&f) {
            register_callback([f = std::forward<F>(f)](flecs::world &world) { f(world.component<T>()); });
            return *this;
        }
    };

    /// Function type for retrieving a component value as a Godot Variant.
    using ComponentGetter = std::function<godot::Variant(const flecs::world &, flecs::entity_t)>;

    /// Function type for setting a component value from a Godot Variant.
    using ComponentSetter = std::function<void(flecs::world &, flecs::entity_t, const godot::Variant &)>;

    /// Function type for creating a default Godot Variant for a component.
    using ComponentDefaulter = std::function<godot::Variant()>;

    struct ComponentInfo {
        bool is_singleton = false;
        godot::String name; // Full name from Flecs (e.g. "namespace::Component")
    };

    using ComponentInspector = std::function<void(flecs::world &, ComponentInfo &)>;

    struct ComponentFunctions {
        ComponentGetter getter;
        ComponentSetter setter;
        ComponentDefaulter defaulter;
        ComponentInspector inspector;
    };

    /// Returns the global map of component functions, keyed by component name.
    std::unordered_map<std::string, ComponentFunctions> &get_component_registry();

    namespace internal {
        template <typename T> struct is_vector : std::false_type {};
        template <typename T, typename A> struct is_vector<std::vector<T, A>> : std::true_type {};

        template <typename T> struct is_array : std::false_type {};
        template <typename T, std::size_t N> struct is_array<std::array<T, N>> : std::true_type {};
    } // namespace internal

    template <typename T>
    concept StdVector = internal::is_vector<std::remove_cvref_t<T>>::value;
    template <typename T>
    concept StdArray = internal::is_array<std::remove_cvref_t<T>>::value;

    // Concepts to detect if a component struct wraps a container in a 'value' member
    template <typename T>
    concept HasVectorValue = requires(T t) {
        { t.value } -> StdVector;
    };

    template <typename T>
    concept HasArrayValue = requires(T t) {
        { t.value } -> StdArray;
    };

    /// Registers an inspector function for a specific component type.
    template <typename T> void register_component_inspector(const char *name) {
        get_component_registry()[name].inspector = [](flecs::world &world, ComponentInfo &info) {
            auto comp = world.component<T>();
            info.is_singleton = comp.has(flecs::Singleton);
            info.name = comp.name().c_str();
        };
    }

    /// Unified component registration for scalars, vectors, and arrays.
    /// Wires up inspector, defaulter, getter, and setter based on the type traits of T.
    template <typename T, typename StorageType = T> void register_component(const char *name) {
        auto &registry = get_component_registry()[name];

        // 1. Register Inspector
        register_component_inspector<T>(name);

        // 2. Register Defaulter
        registry.defaulter = []() -> godot::Variant {
            if constexpr (HasVectorValue<T>) {
                return godot::Variant(godot::Array());
            } else if constexpr (HasArrayValue<T>) {
                using ArrayType = decltype(T::value);
                godot::Array arr;
                arr.resize(static_cast<int>(std::tuple_size<ArrayType>::value));
                return godot::Variant(arr);
            } else {
                return godot::Variant(static_cast<StorageType>(T()));
            }
        };

        // 3. Register Getter
        registry.getter = [name](const flecs::world &world, flecs::entity_t entity_id) -> godot::Variant {
            const T *data = nullptr;
            if (entity_id == 0) {
                data = world.try_get<T>();
            } else {
                if (!world.is_alive(entity_id)) {
                    godot::UtilityFunctions::push_warning(godot::String("Get Component: Entity ") + godot::String::num_uint64(entity_id) + " is not alive.");
                    return godot::Variant();
                }
                flecs::entity e(world, entity_id);
                data = e.try_get<T>();
            }

            if (data) {
                if constexpr (HasVectorValue<T> || HasArrayValue<T>) {
                    godot::Array arr;
                    for (const auto &elem : data->value) {
                        arr.push_back(godot::Variant(elem));
                    }
                    return godot::Variant(arr);
                } else {
                    return godot::Variant(static_cast<StorageType>(*data));
                }
            }
            godot::UtilityFunctions::push_warning(godot::String("Get Component: Entity ") + godot::String::num_uint64(entity_id) +
                                                  " returned empty component data for " + name + ". Returning empty Variant.");
            return godot::Variant();
        };

        // 4. Register Setter
        registry.setter = [name](flecs::world &world, flecs::entity_t entity_id, const godot::Variant &v) {
            if constexpr (HasVectorValue<T>) {
                if (v.get_type() != godot::Variant::ARRAY) {
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + name + "'. Expected Array, got " +
                                                          godot::Variant::get_type_name(v.get_type()));
                    return;
                }
                godot::Array arr = v;
                using VecType = decltype(T::value);
                using ElemType = typename VecType::value_type;
                VecType vec;
                vec.reserve(arr.size());
                for (int i = 0; i < arr.size(); ++i) {
                    vec.push_back(static_cast<ElemType>(arr[i]));
                }

                if (entity_id == 0)
                    world.set<T>(T(std::move(vec)));
                else {
                    if (!world.is_alive(entity_id)) {
                        godot::UtilityFunctions::push_warning(godot::String("Set Component: Entity ") + godot::String::num_uint64(entity_id) +
                                                              " is not alive.");
                        return;
                    }
                    flecs::entity(world, entity_id).set<T>(T(std::move(vec)));
                }
            } else if constexpr (HasArrayValue<T>) {
                if (v.get_type() != godot::Variant::ARRAY) {
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + name + "'. Expected Array, got " +
                                                          godot::Variant::get_type_name(v.get_type()));
                    return;
                }
                godot::Array arr = v;
                using ArrType = decltype(T::value);
                constexpr size_t N = std::tuple_size<ArrType>::value;
                if (arr.size() != static_cast<int>(N)) {
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + name + "'. Expected array size " +
                                                          godot::String::num_int64(N) + ", got " + godot::String::num_int64(arr.size()));
                    return;
                }
                using ElemType = typename ArrType::value_type;
                ArrType array;
                for (size_t i = 0; i < N; ++i) {
                    array[i] = static_cast<ElemType>(arr[static_cast<int>(i)]);
                }

                if (entity_id == 0)
                    world.set<T>(T(std::move(array)));
                else {
                    if (!world.is_alive(entity_id)) {
                        godot::UtilityFunctions::push_warning(godot::String("Set Component: Entity ") + godot::String::num_uint64(entity_id) +
                                                              " is not alive.");
                        return;
                    }
                    flecs::entity(world, entity_id).set<T>(T(std::move(array)));
                }
            } else {
                const auto expected_type = static_cast<godot::Variant::Type>(godot::GetTypeInfo<StorageType>::VARIANT_TYPE);
                if (godot::Variant::can_convert(v.get_type(), expected_type)) {
                    if (entity_id == 0) {
                        world.set<T>(T(static_cast<StorageType>(v)));
                    } else {
                        if (!world.is_alive(entity_id)) {
                            godot::UtilityFunctions::push_warning(godot::String("Set Component: Entity ") + godot::String::num_uint64(entity_id) +
                                                                  " is not alive.");
                            return;
                        }
                        flecs::entity e(world, entity_id);
                        e.set<T>(T(static_cast<StorageType>(v)));
                    }
                } else {
                    godot::String warning_message = "Failed to set component '{0}'. Cannot convert provided data from type '{1}' to the expected type '{2}'.";
                    godot::UtilityFunctions::push_warning(warning_message.format(
                        godot::Array::make(name, godot::Variant::get_type_name(v.get_type()), godot::Variant::get_type_name(expected_type))));
                }
            }
        };
    }
} // namespace stagehand
