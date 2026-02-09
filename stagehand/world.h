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

    class World : public Node
    {
        GDCLASS(World, Node)

    public:
        World();

        // GDScript-visible methods that we'll bind
        void progress(double delta); // To be called every frame from GDScript attached to the World node. Make sure ecs_ftime_t matches the type of delta.
        void set_component(const godot::String& component_name, const godot::Variant& data, ecs_entity_t entity_id = 0);
        godot::Variant get_component(const godot::String& component_name, ecs_entity_t entity_id = 0);
        bool enable_system(const godot::String& system_name, bool enabled = true);
        bool run_system(const godot::String& system_name, const godot::Dictionary& parameters); // For triggering on-demand (kind: 0) Flecs systems from GDScript

        // Virtual methods overridden from Node
        void _exit_tree() override;

        ~World();

    protected:
        void _notification(const int p_what);
        static void _bind_methods();

    private:
        flecs::world world;
        bool is_initialised = false;
        flecs::system get_system(const godot::String& system_name);
        std::unordered_map<std::string, std::function<void(flecs::entity_t, const godot::Variant&)>> component_setters;
        std::unordered_map<std::string, std::function<godot::Variant(flecs::entity_t)>> component_getters;
        void populate_scene_children_singleton();
        void setup_entity_renderers_multimesh();
    };

} // namespace stagehand
