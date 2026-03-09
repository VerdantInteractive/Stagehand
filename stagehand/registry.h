#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "flecs.h"

namespace stagehand {
    namespace internal {
        constexpr bool stagehand_auto_seed_singleton(...) { return false; }

        template <typename T, typename = void> struct singleton_seed_basis {
            using type = std::remove_cvref_t<T>;
        };

        template <typename T> struct singleton_seed_basis<T, std::void_t<typename std::remove_cvref_t<T>::base_type>> {
            using type = typename std::remove_cvref_t<T>::base_type;
        };

        template <typename T> using singleton_seed_basis_t = typename singleton_seed_basis<T>::type;

        template <typename T>
        concept has_native_ptr_method = requires(const std::remove_cvref_t<T> &value) { value._native_ptr(); };

        template <typename T, typename = void> struct explicit_auto_seed_singleton : std::false_type {};

        template <typename T>
        struct explicit_auto_seed_singleton<T, std::void_t<decltype(stagehand_auto_seed_singleton(static_cast<T *>(nullptr)))>>
            : std::bool_constant<stagehand_auto_seed_singleton(static_cast<T *>(nullptr))> {};

        template <typename T>
        struct inferred_auto_seed_singleton : std::bool_constant<std::is_default_constructible_v<T> && !has_native_ptr_method<singleton_seed_basis_t<T>>> {};

        template <typename T>
        struct should_auto_seed_singleton
            : std::conditional_t<explicit_auto_seed_singleton<T>::value, explicit_auto_seed_singleton<T>, inferred_auto_seed_singleton<T>> {};

        template <typename T> void initialize_singleton_value_if_needed(flecs::world &world) {
            flecs::component<T> component = world.component<T>();
            if (!component.is_valid() || !component.has(flecs::Singleton)) {
                return;
            }

            if constexpr (should_auto_seed_singleton<T>::value && std::is_default_constructible_v<T>) {
                if (!component.template has<T>()) {
                    component.template set<T>(T{});
                }
            }
        }
    } // namespace internal

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

    /// Returns all module names that currently have module-scoped callbacks.
    /// Names are unique and sorted lexicographically.
    std::vector<std::string> get_registered_module_names();

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
        explicit ComponentRegistrar(RegistrationCallback base_callback) {
            register_callback([base_callback = std::move(base_callback)](flecs::world &world) {
                base_callback(world);
                internal::initialize_singleton_value_if_needed<T>(world);
            });
        }

        /// Chain an arbitrary callable that receives flecs::component<T>.
        /// This is the single general-purpose chaining mechanism — any operation
        /// available on flecs::component<T> can be performed inside the callable.
        /// Multiple then() calls can be chained; each is executed in order during
        /// world initialization.
        /// @param f A callable taking flecs::component<T>.
        template <typename F> ComponentRegistrar &then(F &&f) {
            register_callback([f = std::forward<F>(f)](flecs::world &world) {
                f(world.component<T>());
                internal::initialize_singleton_value_if_needed<T>(world);
            });
            return *this;
        }
    };

    /// Function type for retrieving a component value as a Godot Variant.
    using ComponentGetter = std::function<godot::Variant(const flecs::world &, flecs::entity_t)>;

    /// Function type for setting a component value from a Godot Variant.
    using ComponentSetter = std::function<void(flecs::world &, flecs::entity_t, const godot::Variant &)>;

    struct ComponentFunctions {
        ComponentGetter getter;
        ComponentSetter setter;
        flecs::entity_t entity_id = 0; ///< Populated by register_component_with_world_name
        std::string data_type;         ///< C++ storage type used by setter/getter plumbing
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

    // Concept to detect if a type is a Dictionary, TypedDictionary, or inherits from them
    template <typename T>
    concept IsDictionary = std::is_base_of_v<godot::Dictionary, std::remove_cvref_t<T>>;

    inline std::string normalize_registered_component_name(std::string name) {
        while (name.rfind("::", 0) == 0) {
            name.erase(0, 2);
        }
        return name;
    }

    template <typename T> inline std::string get_registered_cpp_type_name() {
        using BareType = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<BareType, std::int8_t>) {
            return "int8_t";
        } else if constexpr (std::is_same_v<BareType, std::int16_t>) {
            return "int16_t";
        } else if constexpr (std::is_same_v<BareType, std::int32_t>) {
            return "int32_t";
        } else if constexpr (std::is_same_v<BareType, std::int64_t>) {
            return "int64_t";
        } else if constexpr (std::is_same_v<BareType, std::uint8_t>) {
            return "uint8_t";
        } else if constexpr (std::is_same_v<BareType, std::uint16_t>) {
            return "uint16_t";
        } else if constexpr (std::is_same_v<BareType, std::uint32_t>) {
            return "uint32_t";
        } else if constexpr (std::is_same_v<BareType, std::uint64_t>) {
            return "uint64_t";
        } else {
            return std::string(flecs::_::type_name<BareType>());
        }
    }

    /// Unified component registration for scalars, vectors, and arrays.
    template <typename T, typename StorageType = T> void register_component(const std::string &name) {
        auto &registry = get_component_registry()[name];
        registry.data_type = get_registered_cpp_type_name<StorageType>();

        // Register Getter
        registry.getter = [component_name = name](const flecs::world &world, flecs::entity_t entity_id) -> godot::Variant {
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
                                                  " returned empty component data for " + component_name.c_str() + ". Returning empty Variant.");
            if constexpr (IsDictionary<T>) {
                return godot::Variant(godot::Dictionary());
            }
            return godot::Variant();
        };

        // Register Setter
        registry.setter = [component_name = name](flecs::world &world, flecs::entity_t entity_id, const godot::Variant &v) {
            if constexpr (HasVectorValue<T>) {
                if (v.get_type() != godot::Variant::ARRAY) {
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + component_name.c_str() + "'. Expected Array, got " +
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
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + component_name.c_str() + "'. Expected Array, got " +
                                                          godot::Variant::get_type_name(v.get_type()));
                    return;
                }
                godot::Array arr = v;
                using ArrType = decltype(T::value);
                constexpr size_t N = std::tuple_size<ArrType>::value;
                if (arr.size() != static_cast<int>(N)) {
                    godot::UtilityFunctions::push_warning(godot::String("Failed to set component '") + component_name.c_str() + "'. Expected array size " +
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
                        godot::Array::make(component_name.c_str(), godot::Variant::get_type_name(v.get_type()), godot::Variant::get_type_name(expected_type))));
                }
            }
        };
    }

    template <typename T, typename StorageType = T> void register_component_with_world_name(flecs::world &world, const char *fallback_name) {
        const flecs::component<T> component_handle = world.component<T>();
        const flecs::entity_t comp_id = component_handle.id();
        flecs::string component_path = component_handle.path();
        std::string qualified_name = component_path.c_str() != nullptr ? std::string(component_path.c_str()) : std::string();
        qualified_name = normalize_registered_component_name(std::move(qualified_name));

        if (!qualified_name.empty()) {
            register_component<T, StorageType>(qualified_name);
            get_component_registry()[qualified_name].entity_id = comp_id;
        }

        register_component<T, StorageType>(std::string(fallback_name));
        get_component_registry()[std::string(fallback_name)].entity_id = comp_id;
    }

    /// Introspection metadata for a registered ECS entity.
    /// Used to bridge registered ECS entity information to scripting.
    struct RegisteredEntityInfo {
        flecs::entity_t id = 0;
        std::string name;
        std::string path;
        std::string namespace_path;
        std::string module_path;
        std::string component_data_type;
        bool is_component = false;
        bool is_prefab = false;
        bool is_system = false;
        bool is_change_detection_tag = false;
        size_t component_size = 0;
        size_t component_alignment = 0;
    };

    /// Collects registered components, prefabs and systems from a world.
    /// @param world World to inspect.
    /// @param include_flecs_builtin When false, excludes entities in the flecs:: namespace.
    /// @return Flat list of metadata entries with name/path/module/type details.
    std::vector<RegisteredEntityInfo> collect_registered_entities(flecs::world &world, bool include_flecs_builtin = false);

    /// Converts a single registered entity entry into a GDScript-friendly dictionary.
    [[nodiscard]] godot::Dictionary to_registered_entity_dictionary(const RegisteredEntityInfo &info);
} // namespace stagehand
