#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/directional_light2d.hpp>
#include <godot_cpp/classes/mesh_instance2d.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs.h>

#include "stagehand/names.h"

using godot::Dictionary;
using godot::Node;

namespace stagehand {

    /// The main World node that integrates Flecs with Godot.
    class World : public Node
    {
        GDCLASS(World, Node)

    public:
        World();

        // GDScript-visible methods that we'll bind

        /// Advances the ECS world by a delta time.
        /// @param delta The time elapsed since the last frame.
        /// @note To be called every frame from GDScript attached to the World node.
        void progress(double delta);

        /// Sets a component value for an entity.
        void set_component(const godot::String& component_name, const godot::Variant& data, ecs_entity_t entity_id = 0);

        /// Gets a component value from an entity.
        godot::Variant get_component(const godot::String& component_name, ecs_entity_t entity_id = 0);

        /// Enables or disables a system by name.
        bool enable_system(const godot::String& system_name, bool enabled = true);

        /// Runs a specific system manually, optionally with parameters.
        /// @param system_name The name of the system to run.
        /// @param parameters A dictionary of parameters to pass to the system.
        /// @note Useful for triggering on-demand (kind: 0) Flecs systems from GDScript.
        bool run_system(const godot::String& system_name, const godot::Dictionary& parameters);

        // Virtual methods overridden from Node

        /// Called when the node is removed from the scene tree.
        void _exit_tree() override;

        ~World();

    protected:
        void _notification(const int p_what);
        static void _bind_methods();

    private:
        flecs::world world;
        bool is_initialised = false;

        /// Helper to look up a system entity by name.
        flecs::system get_system(const godot::String& system_name);
        std::unordered_map<std::string, std::function<void(flecs::entity_t, const godot::Variant&)>> component_setters;
        std::unordered_map<std::string, std::function<godot::Variant(flecs::entity_t)>> component_getters;

        /// Populates the SceneChildren singleton with references to child nodes.
        void populate_scene_children_singleton();
        void setup_entity_renderers_multimesh();
    };

} // namespace stagehand
