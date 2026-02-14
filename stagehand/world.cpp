#include <thread>
#include <cctype>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/world.h"
#include "stagehand/registry.h"
#include "stagehand/script_loader.h"
#include "stagehand/ecs/components/entity_rendering.h"
#include "stagehand/ecs/components/scene_children.h"
#include "stagehand/ecs/components/world_configuration.h"
#include "stagehand/ecs/systems/entity_rendering_multimesh.h"
#include "stagehand/nodes/multi_mesh_renderer.h"
#include "stagehand/utilities/godot_signal.h"
#include "stagehand/utilities/platform.h"

namespace stagehand {

    FlecsWorld::FlecsWorld()
    {
        if (is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld's constructor was called when it was already initialised"));
            return;
        }

        // Don't initialise if we're running in the editor
        if (godot::Engine::get_singleton()->is_editor_hint())
        {
            return;
        }

        // Enable Flecs REST, statistics and extra logging verbosity in debug builds
#if defined(DEBUG_ENABLED)
        godot::UtilityFunctions::print(godot::String("Debug build. Enabling Flecs Explorer and verbose logging ..."));
        world.set<flecs::Rest>({});
        world.import <flecs::stats>();
        // flecs::log::set_level(1);
#endif

        // Set the number of threads Flecs should use based on CPU thread count
        unsigned int num_threads = ::utilities::Platform::get_thread_count();

        // I'm suspecting that the thread exhaustion in the Web builds are caused by this
        // In this project the systems are single-threaded anyway.
        // world.set_threads(static_cast<int>(num_threads));

        register_components_and_systems_with_world(world);

        // Populate the instance's component setters from the global registry
        for (const auto& [component_name, global_setter] : get_component_setters())
        {
            component_setters[component_name] = [this, global_setter](flecs::entity_t entity_id, const godot::Variant& data) { global_setter(this->world, entity_id, data); };
        }

        // Populate the instance's component getters from the global registry
        for (const auto& [component_name, global_getter] : get_component_getters())
        {
            component_getters[component_name] = [this, global_getter](flecs::entity_t entity_id) { return global_getter(this->world, entity_id); };
        }
        // Load Flecs script files that live in the project's flecs_scripts folder.
        // Use a Godot resource path so the loader can resolve it via ProjectSettings.
        ScriptsLoader loader;
        loader.load(world);

        world.observer("stagehand::SignalObserver")
            .event<GodotSignal>()
            .with(flecs::Any) // Tells the observer: "I don't care what components the entity has. If any entity emits this event, trigger the callback."
            .each([this](flecs::iter& it, size_t index) {
            const GodotSignal* signal = it.param<GodotSignal>();
            if (signal) {
                this->emit_signal("flecs_signal_emitted", signal->name, signal->data);
            }
        });

        is_initialised = true;
    }


    void FlecsWorld::populate_scene_children_singleton()
    {
        Dictionary children;
        godot::TypedArray<Node> child_nodes = get_children();
        for (int i = 0; i < child_nodes.size(); ++i)
        {
            Node* child_node = Object::cast_to<Node>(child_nodes[i]);
            if (child_node)
            {
                children[child_node->get_name()] = child_node;
            }
        }
        world.set<SceneChildren>(children);
    }


    void FlecsWorld::setup_entity_renderers_multimesh()
    {
        entity_rendering::Renderers renderers;
        int renderer_count = 0;

        godot::TypedArray<Node> child_nodes = get_children();
        for (int i = 0; i < child_nodes.size(); ++i) {
            // Only register nodes which are MultiMeshRenderer2D or MultiMeshRenderer3D
            if (auto mm2d = godot::Object::cast_to<MultiMeshRenderer2D>(child_nodes[i])) {
                register_multimesh_renderer(world, mm2d, renderers, renderer_count);
            }
            else if (auto mm3d = godot::Object::cast_to<MultiMeshRenderer3D>(child_nodes[i])) {
                register_multimesh_renderer(world, mm3d, renderers, renderer_count);
            }
        }

        if (renderer_count > 0)
        {
            world.component<entity_rendering::Renderers>();
            world.set<entity_rendering::Renderers>(renderers);
            godot::UtilityFunctions::print(godot::String("Registered ") + godot::String::num_int64(renderer_count) + " MultiMesh entity renderers.");
        }
        else
        {
            // No multimesh instances found to use as entity renderers, disable the system.
            stagehand::entity_rendering::EntityRenderingMultiMesh.disable();
        }
    }


    void FlecsWorld::set_component(const godot::String& component_name, const godot::Variant& data, ecs_entity_t entity_id)
    {
        if (!is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::set_component was called before world was initialised"));
            return;
        }

        std::string name = component_name.utf8().get_data();
        if (component_setters.contains(name))
        {
            if (component_name == "WorldConfiguration" && entity_id == 0 && data.get_type() == godot::Variant::DICTIONARY)
            {
                const godot::Dictionary data_dictionary = static_cast<godot::Dictionary>(data);
                godot::TypedDictionary<godot::String, godot::Variant> typed_configuration;
                typed_configuration.assign(data_dictionary);
                world_configuration = typed_configuration;
            }
            component_setters[name](entity_id, data);
        }
        else
        {
            godot::UtilityFunctions::push_warning(godot::String("No setter for component '") + component_name + "' found.");
        }
    }


    godot::Variant FlecsWorld::get_component(const godot::String& component_name, ecs_entity_t entity_id)
    {
        if (!is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::get_component was called before world was initialised"));
            return godot::Variant();
        }

        std::string name = component_name.utf8().get_data();
        if (!component_getters.contains(name))
        {
            godot::UtilityFunctions::push_warning(godot::String("No getter for component '") + component_name + "' found.");
            return godot::Variant();
        }

        return component_getters[name](entity_id);
    }


    void FlecsWorld::set_world_configuration(const godot::TypedDictionary<godot::String, godot::Variant>& p_configuration)
    {
        if (!is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::set_world_configuration was called before world was initialised"));
            return;
        }

        // Avoid self-assignment which can crash
        if (&p_configuration != &world_configuration)
        {
            world_configuration = p_configuration;
        }

        // Replace the singleton configuration with the latest property value.
        set_component("WorldConfiguration", world_configuration);
    }


    godot::TypedDictionary<godot::String, godot::Variant> FlecsWorld::get_world_configuration() const
    {
        return world_configuration;
    }


    flecs::system FlecsWorld::get_system(const godot::String& system_name)
    {
        if (!is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::get_system was called before world was initialised"));
            return flecs::system();
        }

        std::string name = system_name.utf8().get_data();
        flecs::entity entity = world.lookup(name.c_str());
        if (!entity.is_valid())
        {
            godot::UtilityFunctions::push_warning("System not found: " + system_name);
            return flecs::system();
        }
        flecs::system sys = world.system(entity);
        if (!sys.is_valid())
        {
            godot::UtilityFunctions::push_warning(system_name + godot::String(" is not a valid system"));
            return flecs::system();
        }
        return sys;
    }


    bool FlecsWorld::enable_system(const godot::String& system_name, bool enabled)
    {
        flecs::system sys = get_system(system_name);
        if (!sys.is_valid())
        {
            return false;
        }

        enabled ? sys.enable() : sys.disable();
        return true;
    }


    bool FlecsWorld::run_system(const godot::String& system_name, const Dictionary& parameters)
    {
        flecs::system sys = get_system(system_name);
        if (!sys.is_valid())
        {
            return false;
        }

        parameters.is_empty() ? sys.run() : sys.run(0.0f, (void*)&parameters);
        return true;
    }


    void FlecsWorld::progress(double delta)
    {
        if (!is_initialised)
        {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::progress was called before world was initialised"));
            return;
        }

        world.progress(static_cast<ecs_ftime_t>(delta));
    }


    void FlecsWorld::_notification(const int p_what)
    {
        if (p_what == NOTIFICATION_READY)
        {
            set_world_configuration(world_configuration);
            populate_scene_children_singleton();
            setup_entity_renderers_multimesh();
        }
    }


    void FlecsWorld::_exit_tree()
    {
        if (!is_initialised)
        {
            return;
        }

        is_initialised = false;
    }


    void FlecsWorld::_bind_methods()
    {
        godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &FlecsWorld::progress);
        godot::ClassDB::bind_method(godot::D_METHOD("set_component", "component_name", "data", "entity_id"), &FlecsWorld::set_component, DEFVAL(0));
        godot::ClassDB::bind_method(godot::D_METHOD("get_component", "component_name", "entity_id"), &FlecsWorld::get_component, DEFVAL(0));
        godot::ClassDB::bind_method(godot::D_METHOD("set_world_configuration", "configuration"), &FlecsWorld::set_world_configuration);
        godot::ClassDB::bind_method(godot::D_METHOD("get_world_configuration"), &FlecsWorld::get_world_configuration);
        godot::ClassDB::bind_method(godot::D_METHOD("enable_system", "system_name", "enabled"), &FlecsWorld::enable_system, DEFVAL(true));
        godot::ClassDB::bind_method(godot::D_METHOD("run_system", "system_name", "data"), &FlecsWorld::run_system, DEFVAL(Dictionary()));

        ADD_PROPERTY(
            godot::PropertyInfo(godot::Variant::DICTIONARY, "world_configuration",
                godot::PROPERTY_HINT_TYPE_STRING, godot::String::num_int64(godot::Variant::STRING)
                + "/" + godot::String::num_int64(godot::Variant::VARIANT_MAX)
                + ":", godot::PROPERTY_USAGE_DEFAULT
            ),
            "set_world_configuration",
            "get_world_configuration"
        );

        ADD_SIGNAL(godot::MethodInfo("flecs_signal_emitted", godot::PropertyInfo(godot::Variant::STRING_NAME, "name"), godot::PropertyInfo(godot::Variant::DICTIONARY, "data")));
    }


    FlecsWorld::~FlecsWorld() {}

} // namespace stagehand
