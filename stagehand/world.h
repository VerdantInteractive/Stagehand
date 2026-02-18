#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/directional_light2d.hpp>
#include <godot_cpp/classes/mesh_instance2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

#include "flecs.h"

namespace stagehand {
    /// The main FlecsWorld node that integrates Flecs with Godot.
    class FlecsWorld : public godot::Node {
        GDCLASS(FlecsWorld, godot::Node)

        friend class Prefab;

      public:
        enum ProgressTick {
            /// The world progresses in _process() (rendering tick)
            PROGRESS_TICK_RENDERING,
            /// The world progresses in _physics_process() (fixed-rate physics
            /// tick)
            PROGRESS_TICK_PHYSICS,
            /// No world.progress() call happens in either rendering or physics
            /// tick; the world must be progressed manually.
            PROGRESS_TICK_MANUAL
        };

      public:
        FlecsWorld();

        // GDScript-visible methods that we bind
        /// Sets a component value for an entity.
        void set_component(const godot::String &component_name, const godot::Variant &data, uint64_t entity_id = 0);
        /// Gets a component value from an entity.
        [[nodiscard]] godot::Variant get_component(const godot::String &component_name, uint64_t entity_id = 0);
        /// Checks if an entity has a component.
        [[nodiscard]] bool has_component(const godot::String &component_name, uint64_t entity_id);
        /// Adds a component (or tag) to an entity.
        void add_component(const godot::String &component_name, uint64_t entity_id);
        /// Removes a component from an entity.
        void remove_component(const godot::String &component_name, uint64_t entity_id);

        /// Enables or disables a system by name.
        bool enable_system(const godot::String &system_name, bool enabled = true);
        /// Runs a specific system manually, optionally with parameters.
        /// @param system_name The name of the system to run.
        /// @param parameters A dictionary of parameters to pass to the system.
        /// @note Useful for triggering on-demand (kind: 0) Flecs systems from
        /// GDScript.
        bool run_system(const godot::String &system_name, const godot::Dictionary &parameters);

        /// Creates a new entity, optionally with a name.
        uint64_t create_entity(const godot::String &name = "");
        /// Destroys an entity.
        void destroy_entity(uint64_t entity_id);
        /// Checks if an entity is alive.
        [[nodiscard]] bool is_alive(uint64_t entity_id);
        /// Looks up an entity by name.
        [[nodiscard]] uint64_t lookup(const godot::String &name);
        /// Gets the name of an entity.
        [[nodiscard]] godot::String get_entity_name(uint64_t entity_id);
        /// Creates a new prefab entity.
        uint64_t create_prefab(const godot::String &name = "");
        /// Checks if an entity is a prefab.
        [[nodiscard]] bool is_prefab(uint64_t entity_id);
        /// Checks if an entity is an instance of a specific prefab.
        [[nodiscard]] bool is_entity_a(uint64_t entity_id, uint64_t prefab_id);
        /// Instantiates a prefab by name.
        /// @param prefab_name The name of the prefab to instantiate.
        /// @param components A dictionary of component names to values to set on the instance.
        /// @return The entity ID of the new instance, or 0 if failed.
        uint64_t instantiate_prefab(const godot::String &prefab_name, const godot::Dictionary &components = {});

        void set_progress_tick(ProgressTick p_progress_tick);
        ProgressTick get_progress_tick() const { return progress_tick; }
        /// Advances the ECS world by a delta time.
        /// @param delta The time elapsed since the last frame.
        /// @note Can be called from GDScript attached to the FlecsWorld node.
        void progress(double delta);

        /// Sets the world configuration singleton. Format: { "key": value, ... }
        void set_world_configuration(const godot::TypedDictionary<godot::String, godot::Variant> &p_configuration);
        /// Gets the world configuration singleton.
        [[nodiscard]] godot::TypedDictionary<godot::String, godot::Variant> get_world_configuration() const;

        /// Sets the list of Flecs modules (library names) to import on startup.
        void set_modules_to_load(const godot::TypedArray<godot::String> &p_modules);
        /// Gets the list of Flecs modules configured for import.
        [[nodiscard]] godot::TypedArray<godot::String> get_modules_to_load() const;

        ~FlecsWorld();

      protected:
        void _notification(const int p_what);
        static void _bind_methods();

      private:
        flecs::world world;
        bool is_initialised = false;
        ProgressTick progress_tick = ProgressTick::PROGRESS_TICK_RENDERING;
        godot::TypedDictionary<godot::String, godot::Variant> world_configuration;
        godot::TypedArray<godot::String> modules_to_load;

        /// Helper to look up a system entity by name.
        flecs::system get_system(const godot::String &system_name);

        std::unordered_map<std::string, std::function<void(flecs::entity_t, const godot::Variant &)>> component_setters;
        std::unordered_map<std::string, std::function<godot::Variant(flecs::entity_t)>> component_getters;

        void populate_scene_children_singleton();

        void setup_entity_renderers_instanced();
        void cleanup_instanced_renderer_rids();

        void setup_entity_renderers_multimesh();

        void import_configured_modules();
    };
} // namespace stagehand

/// Tells godot-cpp how to convert the C++ enum (ProgressTick) into a godot::Variant (which is usually just an integer at runtime) and back.
VARIANT_ENUM_CAST(stagehand::FlecsWorld::ProgressTick)
