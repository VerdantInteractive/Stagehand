#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

namespace stagehand {

    class FlecsWorld;

    class Prefab : public godot::Resource {
        GDCLASS(Prefab, godot::Resource)

      public:
        Prefab();
        ~Prefab();

        void set_prefab_name(const godot::String &p_name);
        godot::String get_prefab_name() const;

        void set_parents(const godot::TypedArray<Prefab> &p_parents);
        godot::TypedArray<Prefab> get_parents() const;

        void set_components(const godot::TypedDictionary<godot::String, godot::Variant> &p_components);
        godot::TypedDictionary<godot::String, godot::Variant> get_components() const;

        /// Registers this prefab definition into the Flecs world.
        /// Creates the prefab entity, sets components, and sets up inheritance.
        /// @return The entity ID of the registered prefab.
        uint64_t register_with_world(FlecsWorld *world);

      protected:
        static void _bind_methods();

      private:
        godot::String prefab_name;
        godot::TypedArray<Prefab> parents;
        godot::TypedDictionary<godot::String, godot::Variant> components;
    };

} // namespace stagehand
