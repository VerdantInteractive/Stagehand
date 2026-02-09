#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <flecs.h>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace stagehand {

    /// Callback function type for registering components and systems with the Flecs world.
    using RegistrationCallback = std::function<void(flecs::world&)>;

    /// Call all registered callbacks to configure the ECS world.
    /// @param world The Flecs world to configure.
    void register_components_and_systems_with_world(flecs::world& world);

    /// Register a callback to be called during world initialization.
    /// @param callback The callback to register.
    void register_callback(RegistrationCallback callback);

    /// Helper struct for static auto-registration from translation units.
    /// Instantiating this struct with a callback will register it to be called during world initialization.
    struct Registry
    {
        explicit Registry(RegistrationCallback callback);
    };

    /// Template class for component registration with method chaining support.
    /// Used by component definition macros to allow chaining modifiers, e.g.:
    ///     INT16(Foo).add(flecs::CanToggle);
    template <typename T>
    class ComponentRegistrar {
    public:
        explicit ComponentRegistrar(RegistrationCallback base_callback) {
            register_callback(std::move(base_callback));
        }

        /// Chain an arbitrary callable that receives the component entity.
        /// @param f A callable taking flecs::component<T>.
        template <typename F>
        ComponentRegistrar& then(F&& f) {
            register_callback([f = std::forward<F>(f)](flecs::world& world) {
                f(world.component<T>());
            });
            return *this;
        }

        /// Add a component, tag, or trait by entity id.
        ComponentRegistrar& add(flecs::id_t id) {
            register_callback([id](flecs::world& world) {
                world.component<T>().add(id);
            });
            return *this;
        }

        /// Add a pair relationship by entity ids.
        ComponentRegistrar& add(flecs::entity_t first, flecs::entity_t second) {
            register_callback([first, second](flecs::world& world) {
                world.component<T>().add(first, second);
            });
            return *this;
        }

        /// Add a component or tag by type.
        template <typename U>
        ComponentRegistrar& add() {
            register_callback([](flecs::world& world) {
                world.component<T>().template add<U>();
            });
            return *this;
        }

        /// Add a pair relationship by types.
        template <typename First, typename Second>
        ComponentRegistrar& add() {
            register_callback([](flecs::world& world) {
                world.component<T>().template add<First, Second>();
            });
            return *this;
        }

        /// Set a value on the component entity.
        template <typename U>
        ComponentRegistrar& set(const U& value) {
            register_callback([value](flecs::world& world) {
                world.component<T>().template set<U>(value);
            });
            return *this;
        }
    };

    /// Function type for retrieving a component value as a Godot Variant.
    using ComponentGetter = std::function<godot::Variant(const flecs::world&, flecs::entity_t)>;

    /// Function type for setting a component value from a Godot Variant.
    using ComponentSetter = std::function<void(flecs::world&, flecs::entity_t, const godot::Variant&)>;

    /// Returns the global map of component getters, keyed by component name.
    std::unordered_map<std::string, ComponentGetter>& get_component_getters();

    /// Returns the global map of component setters, keyed by component name.
    std::unordered_map<std::string, ComponentSetter>& get_component_setters();

    /// Registers a getter function for a specific component type.
    template <typename T, typename StorageType = T>
    void register_component_getter(const char* name)
    {
        get_component_getters()[name] = [name](const flecs::world& world, flecs::entity_t entity_id) -> godot::Variant
        {
            const T* data = nullptr;
            if (entity_id == 0) {
                data = world.try_get<T>();
            }
            else {
                if (!world.is_alive(entity_id)) {
                    godot::UtilityFunctions::push_warning(godot::String("Get Component: Entity ") + godot::String::num_uint64(entity_id) + " is not alive.");
                    return godot::Variant();
                }
                flecs::entity e(world, entity_id);
                data = e.try_get<T>();
            }

            if (data) {
                return godot::Variant(static_cast<StorageType>(*data));
            }
            godot::UtilityFunctions::push_warning(godot::String("Get Component: Entity ") + godot::String::num_uint64(entity_id) + " returned empty component data for " + name + ". Returning empty Variant.");
            return godot::Variant();
        };
    }

    /// Registers a setter function for a specific component type.
    template <typename T, typename StorageType = T>
    void register_component_setter(const char* name)
    {
        get_component_setters()[name] = [name](flecs::world& world, flecs::entity_t entity_id, const godot::Variant& v) {
            const auto expected_type = static_cast<godot::Variant::Type>(godot::GetTypeInfo<StorageType>::VARIANT_TYPE);
            if (godot::Variant::can_convert(v.get_type(), expected_type))
            {
                if (entity_id == 0) {
                    world.set<T>(T(static_cast<StorageType>(v)));
                }
                else {
                    if (!world.is_alive(entity_id)) {
                        godot::UtilityFunctions::push_warning(godot::String("Set Component: Entity ") + godot::String::num_uint64(entity_id) + " is not alive.");
                        return;
                    }
                    flecs::entity e(world, entity_id);
                    e.set<T>(T(static_cast<StorageType>(v)));
                }
            }
            else
            {
                godot::String warning_message = "Failed to set component '{0}'. Cannot convert provided data from type '{1}' to the expected type '{2}'.";
                godot::UtilityFunctions::push_warning(warning_message.format(godot::Array::make(name, godot::Variant::get_type_name(v.get_type()), godot::Variant::get_type_name(expected_type))));
            }
        };
    }

} // namespace stagehand
