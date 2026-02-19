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

#include "stagehand/script_loader.h"

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
        /// Enables or disables a system by name.
        bool enable_system(const godot::String &system_name, bool enabled = true);
        /// Runs a specific system manually, optionally with parameters.
        /// @param system_name The name of the system to run.
        /// @param parameters A dictionary of parameters to pass to the system.
        /// @note Useful for triggering on-demand (kind: 0) Flecs systems from
        /// GDScript.
        bool run_system(const godot::String &system_name, const godot::Dictionary &parameters);

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
        ScriptLoader script_loader;

        /// Helper to look up a system entity by name.
        flecs::system get_system(const godot::String &system_name);

        std::unordered_map<std::string, std::function<void(flecs::entity_t, const godot::Variant &)>> component_setters;
        std::unordered_map<std::string, std::function<godot::Variant(flecs::entity_t)>> component_getters;

        void register_signal_observer();
        void populate_scene_children_singleton();
        void import_configured_modules();
        void setup_entity_renderers_instanced();
        void setup_entity_renderers_multimesh();
        void cleanup_instanced_renderer_rids();
    };
} // namespace stagehand

/// Tells godot-cpp how to convert the C++ enum (ProgressTick) into a godot::Variant (which is usually just an integer at runtime) and back.
VARIANT_ENUM_CAST(stagehand::FlecsWorld::ProgressTick)
