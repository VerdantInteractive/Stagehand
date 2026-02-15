#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/directional_light2d.hpp>
#include <godot_cpp/classes/mesh_instance2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

#include "flecs.h"

namespace stagehand {

    /// The main FlecsWorld node that integrates Flecs with Godot.
    class FlecsWorld : public godot::Node {
        GDCLASS(FlecsWorld, godot::Node)

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

        // GDScript-visible methods that we'll bind
        /// Sets a component value for an entity.
        void set_component(const godot::String &component_name, const godot::Variant &data, ecs_entity_t entity_id = 0);

        /// Gets a component value from an entity.
        [[nodiscard]] godot::Variant get_component(const godot::String &component_name, ecs_entity_t entity_id = 0);

        /// Sets the world configuration singleton. Format: { "key": value, ...
        /// }
        void set_world_configuration(const godot::TypedDictionary<godot::String, godot::Variant> &p_configuration);

        /// Gets the world configuration singleton.
        [[nodiscard]] godot::TypedDictionary<godot::String, godot::Variant> get_world_configuration() const;

        /// Enables or disables a system by name.
        bool enable_system(const godot::String &system_name, bool enabled = true);

        /// Runs a specific system manually, optionally with parameters.
        /// @param system_name The name of the system to run.
        /// @param parameters A dictionary of parameters to pass to the system.
        /// @note Useful for triggering on-demand (kind: 0) Flecs systems from
        /// GDScript.
        bool run_system(const godot::String &system_name, const godot::Dictionary &parameters);

        void set_progress_tick(ProgressTick p_progress_tick) { progress_tick = p_progress_tick; }
        ProgressTick get_progress_tick() const { return progress_tick; }

        /// Advances the ECS world by a delta time.
        /// @param delta The time elapsed since the last frame.
        /// @note To be called every frame from GDScript attached to the
        /// FlecsWorld node.
        void progress(double delta);

        ~FlecsWorld();

      protected:
        void _notification(const int p_what);
        static void _bind_methods();

      private:
        flecs::world world;
        bool is_initialised = false;
        godot::TypedDictionary<godot::String, godot::Variant> world_configuration;
        ProgressTick progress_tick = ProgressTick::PROGRESS_TICK_RENDERING;

        /// Helper to look up a system entity by name.
        flecs::system get_system(const godot::String &system_name);
        std::unordered_map<std::string, std::function<void(flecs::entity_t, const godot::Variant &)>> component_setters;
        std::unordered_map<std::string, std::function<godot::Variant(flecs::entity_t)>> component_getters;

        /// Populates the SceneChildren singleton with references to child
        /// nodes.
        void populate_scene_children_singleton();
        void setup_entity_renderers_multimesh();
    };

} // namespace stagehand

VARIANT_ENUM_CAST(stagehand::FlecsWorld::ProgressTick)
