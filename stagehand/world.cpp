#include "stagehand/world.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/godot_signal.h"
#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/scene_children.h"
#include "stagehand/ecs/components/world_configuration.h"
#include "stagehand/ecs/systems/rendering_instanced.h"
#include "stagehand/ecs/systems/rendering_multimesh.h"
#include "stagehand/nodes/instanced_renderer_3d.h"
#include "stagehand/nodes/multi_mesh_renderer.h"
#include "stagehand/registry.h"
#include "stagehand/resources/prefab.h"
#include "stagehand/script_loader.h"
#include "stagehand/utilities/platform.h"

namespace stagehand {
    FlecsWorld::FlecsWorld() {
        if (is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld's constructor was called when it was "
                                                                "already initialised"));
            return;
        }

        // Don't initialise if we're running in the editor
        if (godot::Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        // Set up process callbacks based on the configured progress tick mode
        set_progress_tick(progress_tick);

        // Enable Flecs REST, statistics and extra logging verbosity in debug
        // builds
#if defined(DEBUG_ENABLED)
        godot::UtilityFunctions::print(godot::String("Debug build. Enabling Flecs Explorer and verbose logging ..."));
        world.set<flecs::Rest>({});
        world.import <flecs::stats>();
        // flecs::log::set_level(1);
#endif

        // Set the number of threads Flecs should use based on CPU thread count
        unsigned int num_threads = ::utilities::Platform::get_thread_count();

        // I'm suspecting that the thread exhaustion in the Web builds are
        // caused by this In this project the systems are single-threaded
        // anyway. world.set_threads(static_cast<int>(num_threads));

        register_components_and_systems_with_world(world);

        // Populate the instance's component setters from the global registry
        for (const auto &[component_name, global_setter] : get_component_setters()) {
            component_setters[component_name] = [this, global_setter](flecs::entity_t entity_id, const godot::Variant &data) {
                global_setter(this->world, entity_id, data);
            };
        }

        // Populate the instance's component getters from the global registry
        for (const auto &[component_name, global_getter] : get_component_getters()) {
            component_getters[component_name] = [this, global_getter](flecs::entity_t entity_id) { return global_getter(this->world, entity_id); };
        }
        // Load Flecs script files that live in the project's flecs_scripts
        // folder. Use a Godot resource path so the loader can resolve it via
        // ProjectSettings.
        ScriptsLoader loader;
        loader.load(world);

        world.observer("stagehand::SignalObserver")
            .event<GodotSignal>()
            .with(flecs::Any) // Tells the observer: "I don't care what components the entity has. If any entity emits this event, trigger the callback."
            .each([this](flecs::iter &it, size_t index) {
                const GodotSignal *signal = it.param<GodotSignal>();
                if (signal) {
                    this->emit_signal("stagehand_signal_emitted", signal->name, signal->data);
                }
            });

        is_initialised = true;
    }

    void FlecsWorld::populate_scene_children_singleton() {
        Dictionary children;
        godot::TypedArray<Node> child_nodes = get_children();
        for (int i = 0; i < child_nodes.size(); ++i) {
            Node *child_node = Object::cast_to<Node>(child_nodes[i]);
            if (child_node) {
                children[child_node->get_name()] = child_node;
            }
        }
        world.set<SceneChildren>(children);
    }

    void FlecsWorld::setup_entity_renderers_instanced() {
        int renderer_count = 0;

        // Get an existing copy of the Renderers singleton (may already contain multimesh configs) or start with a fresh one.
        world.component<rendering::Renderers>();
        rendering::Renderers renderers;
        const rendering::Renderers *existing = world.try_get<rendering::Renderers>();
        if (existing) {
            renderers = *existing;
        }

        godot::TypedArray<Node> child_nodes = get_children();
        for (int i = 0; i < child_nodes.size(); ++i) {
            if (auto ir3d = godot::Object::cast_to<InstancedRenderer3D>(child_nodes[i])) {
                register_instanced_renderer(world, ir3d, renderers, renderer_count);
            }
        }

        if (renderer_count > 0) {
            world.set<rendering::Renderers>(renderers);
            godot::UtilityFunctions::print(godot::String("Registered ") + godot::String::num_int64(renderer_count) + " Instanced entity renderers.");
        } else {
            stagehand::rendering::EntityRenderingInstanced.disable();
        }
    }

    void FlecsWorld::cleanup_instanced_renderer_rids() {
        const rendering::Renderers *renderers_ptr = world.try_get<rendering::Renderers>();
        if (!renderers_ptr) {
            return;
        }

        godot::RenderingServer *rendering_server = godot::RenderingServer::get_singleton();
        if (!rendering_server) {
            return;
        }

        // Free all RenderingServer instance RIDs created by the instanced rendering system
        for (const rendering::InstancedRendererConfig &renderer : renderers_ptr->instanced_renderers) {
            for (const godot::RID &rid : renderer.instance_rids) {
                if (rid.is_valid()) {
                    rendering_server->free_rid(rid);
                }
            }
        }
    }

    void FlecsWorld::setup_entity_renderers_multimesh() {
        rendering::Renderers renderers;
        int renderer_count = 0;

        godot::TypedArray<Node> child_nodes = get_children();
        for (int i = 0; i < child_nodes.size(); ++i) {
            // Only register nodes which are MultiMeshRenderer2D or
            // MultiMeshRenderer3D
            if (auto mm2d = godot::Object::cast_to<MultiMeshRenderer2D>(child_nodes[i])) {
                register_multimesh_renderer(world, mm2d, renderers, renderer_count);
            } else if (auto mm3d = godot::Object::cast_to<MultiMeshRenderer3D>(child_nodes[i])) {
                register_multimesh_renderer(world, mm3d, renderers, renderer_count);
            }
        }

        if (renderer_count > 0) {
            world.component<rendering::Renderers>();
            world.set<rendering::Renderers>(renderers);
            godot::UtilityFunctions::print(godot::String("Registered ") + godot::String::num_int64(renderer_count) + " MultiMesh entity renderers.");
        } else {
            // No multimesh instances found to use as entity renderers, disable
            // the system.
            stagehand::rendering::EntityRenderingMultiMesh.disable();
        }
    }

    void FlecsWorld::set_component(const godot::String &component_name, const godot::Variant &data, uint64_t entity_id) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::set_component was called before world "
                                                                "was initialised"));
            return;
        }

        std::string name = component_name.utf8().get_data();
        if (component_setters.contains(name)) {
            component_setters[name](static_cast<ecs_entity_t>(entity_id), data);
        } else {
            godot::UtilityFunctions::push_warning(godot::String("No setter for component '") + component_name + "' found.");
        }
    }

    godot::Variant FlecsWorld::get_component(const godot::String &component_name, uint64_t entity_id) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::get_component was called before world "
                                                                "was initialised"));
            return godot::Variant();
        }

        std::string name = component_name.utf8().get_data();
        if (!component_getters.contains(name)) {
            godot::UtilityFunctions::push_warning(godot::String("No getter for component '") + component_name + "' found.");
            return godot::Variant();
        }

        return component_getters[name](static_cast<ecs_entity_t>(entity_id));
    }

    void FlecsWorld::set_world_configuration(const godot::TypedDictionary<godot::String, godot::Variant> &p_configuration) {
        const godot::TypedDictionary<godot::String, godot::Variant> previous_configuration = world_configuration;

        // Avoid self-assignment which can crash
        if (&p_configuration != &world_configuration) {
            world_configuration = p_configuration;
        }

        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::set_world_configuration was called "
                                                                "before world was initialised"));
            return;
        }

        const WorldConfiguration *existing_configuration = world.try_get<WorldConfiguration>();
        if (existing_configuration != nullptr && p_configuration == previous_configuration && existing_configuration->value != world_configuration) {
            return;
        }

        // Replace the singleton configuration with the latest property value.
        set_component("WorldConfiguration", world_configuration);
    }

    godot::TypedDictionary<godot::String, godot::Variant> FlecsWorld::get_world_configuration() const {
        if (godot::Engine::get_singleton()->is_editor_hint()) {
            return world_configuration;
        }

        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::get_world_configuration was called "
                                                                "before world was initialised"));
            return world_configuration;
        }

        const WorldConfiguration *configuration = world.try_get<WorldConfiguration>();
        if (configuration != nullptr) {
            return configuration->value;
        }

        return world_configuration;
    }

    flecs::system FlecsWorld::get_system(const godot::String &system_name) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::get_system was called before world "
                                                                "was initialised"));
            return flecs::system();
        }

        std::string name = system_name.utf8().get_data();
        flecs::entity entity = world.lookup(name.c_str());
        if (!entity.is_valid()) {
            godot::UtilityFunctions::push_warning("System not found: " + system_name);
            return flecs::system();
        }
        if (!entity.has(flecs::System)) {
            godot::UtilityFunctions::push_warning(system_name + godot::String(" is not a system"));
            return flecs::system();
        }
        return world.system(entity);
    }

    bool FlecsWorld::enable_system(const godot::String &system_name, bool enabled) {
        flecs::system sys = get_system(system_name);
        if (!sys.is_valid()) {
            return false;
        }

        enabled ? sys.enable() : sys.disable();
        return true;
    }

    bool FlecsWorld::run_system(const godot::String &system_name, const Dictionary &parameters) {
        flecs::system sys = get_system(system_name);
        if (!sys.is_valid()) {
            return false;
        }

        parameters.is_empty() ? sys.run() : sys.run(0.0f, (void *)&parameters);
        return true;
    }

    bool FlecsWorld::has_component(const godot::String &component_name, uint64_t entity_id) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::has_component called before world initialised");
            return false;
        }
        std::string name = component_name.utf8().get_data();
        flecs::entity comp = world.lookup(name.c_str());
        if (!comp.is_valid()) {
            godot::UtilityFunctions::push_warning("Component not found: " + component_name);
            return false;
        }
        return world.entity(static_cast<ecs_entity_t>(entity_id)).has(comp);
    }

    void FlecsWorld::add_component(const godot::String &component_name, uint64_t entity_id) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::add_component called before world initialised");
            return;
        }
        std::string name = component_name.utf8().get_data();
        flecs::entity comp = world.lookup(name.c_str());
        if (!comp.is_valid()) {
            godot::UtilityFunctions::push_warning("Component not found: " + component_name);
            return;
        }
        world.entity(static_cast<ecs_entity_t>(entity_id)).add(comp);
    }

    void FlecsWorld::remove_component(const godot::String &component_name, uint64_t entity_id) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::remove_component called before world initialised");
            return;
        }
        std::string name = component_name.utf8().get_data();
        flecs::entity comp = world.lookup(name.c_str());
        if (!comp.is_valid()) {
            godot::UtilityFunctions::push_warning("Component not found: " + component_name);
            return;
        }
        world.entity(static_cast<ecs_entity_t>(entity_id)).remove(comp);
    }

    uint64_t FlecsWorld::create_entity(const godot::String &name) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::create_entity called before world initialised");
            return 0;
        }
        if (name.is_empty()) {
            return static_cast<uint64_t>(world.entity().id());
        }
        return static_cast<uint64_t>(world.entity(name.utf8().get_data()).id());
    }

    void FlecsWorld::destroy_entity(uint64_t entity_id) {
        if (!is_initialised)
            return;
        world.entity(static_cast<ecs_entity_t>(entity_id)).destruct();
    }

    bool FlecsWorld::is_alive(uint64_t entity_id) {
        if (!is_initialised)
            return false;
        return world.entity(static_cast<ecs_entity_t>(entity_id)).is_alive();
    }

    uint64_t FlecsWorld::lookup(const godot::String &name) {
        if (!is_initialised)
            return 0;
        return world.lookup(name.utf8().get_data()).id();
    }

    godot::String FlecsWorld::get_entity_name(uint64_t entity_id) {
        if (!is_initialised)
            return "";
        return godot::String(world.entity(static_cast<ecs_entity_t>(entity_id)).name().c_str());
    }

    uint64_t FlecsWorld::create_prefab(const godot::String &name) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::create_prefab called before world initialised");
            return 0;
        }
        if (name.is_empty()) {
            return world.prefab().id();
        }
        return world.prefab(name.utf8().get_data()).id();
    }

    bool FlecsWorld::is_prefab(uint64_t entity_id) {
        if (!is_initialised)
            return false;
        return world.entity(static_cast<ecs_entity_t>(entity_id)).has(flecs::Prefab);
    }

    bool FlecsWorld::is_entity_a(uint64_t entity_id, uint64_t prefab_id) {
        if (!is_initialised)
            return false;
        return world.entity(static_cast<ecs_entity_t>(entity_id)).has(flecs::IsA, world.entity(static_cast<ecs_entity_t>(prefab_id)));
    }

    uint64_t FlecsWorld::instantiate_prefab(const godot::String &prefab_name, const godot::Dictionary &components) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning("FlecsWorld::instantiate_prefab called before world initialised");
            return 0;
        }

        std::string name = prefab_name.utf8().get_data();
        flecs::entity prefab = world.lookup(name.c_str());
        if (!prefab.is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("Prefab '") + prefab_name + "' not found");
            return 0;
        }

        flecs::entity instance = world.entity().is_a(prefab);

        if (!components.is_empty()) {
            godot::Array keys = components.keys();
            for (int i = 0; i < keys.size(); ++i) {
                godot::String key = keys[i];
                std::string key_std = key.utf8().get_data();
                if (component_setters.count(key_std)) {
                    component_setters[key_std](instance.id(), components[key]);
                } else {
                    godot::UtilityFunctions::push_warning(godot::String("No setter found for component '") + key + "'");
                }
            }
        }

        return static_cast<uint64_t>(instance.id());
    }

    void FlecsWorld::set_progress_tick(ProgressTick p_progress_tick) {
        progress_tick = p_progress_tick;

        set_process(false);
        set_physics_process(false);

        if (progress_tick == ProgressTick::PROGRESS_TICK_RENDERING) {
            set_process(true);
        } else if (progress_tick == ProgressTick::PROGRESS_TICK_PHYSICS) {
            set_physics_process(true);
        }
    }

    void FlecsWorld::progress(double delta) {
        if (!is_initialised) {
            godot::UtilityFunctions::push_warning(godot::String("FlecsWorld::progress was called before world "
                                                                "was initialised"));
            return;
        }

        world.progress(static_cast<ecs_ftime_t>(delta));
    }

    void FlecsWorld::_notification(const int p_what) {
        switch (p_what) {
        case NOTIFICATION_READY: /// N.B. This fires *after* GDScript _ready()
            set_world_configuration(world_configuration);
            populate_scene_children_singleton();
            setup_entity_renderers_instanced();
            setup_entity_renderers_multimesh();
            break;
        case NOTIFICATION_PROCESS:
            if (progress_tick == ProgressTick::PROGRESS_TICK_RENDERING) {
                progress(get_process_delta_time());
            }
            break;
        case NOTIFICATION_PHYSICS_PROCESS:
            if (progress_tick == ProgressTick::PROGRESS_TICK_PHYSICS) {
                progress(get_physics_process_delta_time());
            }
            break;
        case NOTIFICATION_EXIT_TREE:
            if (is_initialised) {
                cleanup_instanced_renderer_rids();
                is_initialised = false;
            }
            break;
        }
    }

    void FlecsWorld::_bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("set_component", "component_name", "data", "entity_id"), &FlecsWorld::set_component, DEFVAL(0));
        godot::ClassDB::bind_method(godot::D_METHOD("get_component", "component_name", "entity_id"), &FlecsWorld::get_component, DEFVAL(0));
        godot::ClassDB::bind_method(godot::D_METHOD("has_component", "component_name", "entity_id"), &FlecsWorld::has_component);
        godot::ClassDB::bind_method(godot::D_METHOD("add_component", "component_name", "entity_id"), &FlecsWorld::add_component);
        godot::ClassDB::bind_method(godot::D_METHOD("remove_component", "component_name", "entity_id"), &FlecsWorld::remove_component);

        godot::ClassDB::bind_method(godot::D_METHOD("enable_system", "system_name", "enabled"), &FlecsWorld::enable_system, DEFVAL(true));
        godot::ClassDB::bind_method(godot::D_METHOD("run_system", "system_name", "data"), &FlecsWorld::run_system, DEFVAL(Dictionary()));

        godot::ClassDB::bind_method(godot::D_METHOD("create_entity", "name"), &FlecsWorld::create_entity, DEFVAL(""));
        godot::ClassDB::bind_method(godot::D_METHOD("destroy_entity", "entity_id"), &FlecsWorld::destroy_entity);
        godot::ClassDB::bind_method(godot::D_METHOD("is_alive", "entity_id"), &FlecsWorld::is_alive);
        godot::ClassDB::bind_method(godot::D_METHOD("lookup", "name"), &FlecsWorld::lookup);
        godot::ClassDB::bind_method(godot::D_METHOD("get_entity_name", "entity_id"), &FlecsWorld::get_entity_name);
        godot::ClassDB::bind_method(godot::D_METHOD("create_prefab", "name"), &FlecsWorld::create_prefab, DEFVAL(""));
        godot::ClassDB::bind_method(godot::D_METHOD("is_prefab", "entity_id"), &FlecsWorld::is_prefab);
        godot::ClassDB::bind_method(godot::D_METHOD("is_entity_a", "entity_id", "prefab_id"), &FlecsWorld::is_entity_a);
        godot::ClassDB::bind_method(godot::D_METHOD("instantiate_prefab", "prefab_name", "components"), &FlecsWorld::instantiate_prefab, DEFVAL(Dictionary()));

        godot::ClassDB::bind_method(godot::D_METHOD("set_progress_tick", "progress_tick"), &FlecsWorld::set_progress_tick);
        godot::ClassDB::bind_method(godot::D_METHOD("get_progress_tick"), &FlecsWorld::get_progress_tick);
        godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &FlecsWorld::progress);

        godot::ClassDB::bind_method(godot::D_METHOD("set_world_configuration", "configuration"), &FlecsWorld::set_world_configuration);
        godot::ClassDB::bind_method(godot::D_METHOD("get_world_configuration"), &FlecsWorld::get_world_configuration);

        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "progress_tick", godot::PROPERTY_HINT_ENUM, "Rendering,Physics,Manual"), "set_progress_tick",
                     "get_progress_tick");
        BIND_ENUM_CONSTANT(PROGRESS_TICK_RENDERING);
        BIND_ENUM_CONSTANT(PROGRESS_TICK_PHYSICS);
        BIND_ENUM_CONSTANT(PROGRESS_TICK_MANUAL);

        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "world_configuration", godot::PROPERTY_HINT_TYPE_STRING,
                                         godot::String::num_int64(godot::Variant::STRING) + "/" + godot::String::num_int64(godot::PROPERTY_HINT_NONE) + ":",
                                         godot::PROPERTY_USAGE_DEFAULT),
                     "set_world_configuration", "get_world_configuration");

        ADD_SIGNAL(godot::MethodInfo("stagehand_signal_emitted", godot::PropertyInfo(godot::Variant::STRING_NAME, "name"),
                                     godot::PropertyInfo(godot::Variant::DICTIONARY, "data")));
    }

    FlecsWorld::~FlecsWorld() {}
} // namespace stagehand
