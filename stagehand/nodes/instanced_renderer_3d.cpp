#include "stagehand/nodes/instanced_renderer_3d.h"

#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

void InstancedRenderer3DLODConfiguration::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_mesh", "mesh"), &InstancedRenderer3DLODConfiguration::set_mesh);
    godot::ClassDB::bind_method(godot::D_METHOD("get_mesh"), &InstancedRenderer3DLODConfiguration::get_mesh);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_begin", "visibility_range_begin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_begin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_begin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_begin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_end", "visibility_range_end"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_end);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_end"), &InstancedRenderer3DLODConfiguration::get_visibility_range_end);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_begin_margin", "visibility_range_begin_margin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_begin_margin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_begin_margin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_begin_margin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_end_margin", "visibility_range_end_margin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_end_margin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_end_margin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_end_margin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_fade_mode", "visibility_range_fade_mode"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_fade_mode);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_fade_mode"), &InstancedRenderer3DLODConfiguration::get_visibility_range_fade_mode);

    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::OBJECT, "mesh", godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_begin", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"),
                                 "set_visibility_range_begin", "get_visibility_range_begin");
    godot::ClassDB::add_property(
        "InstancedRenderer3DLODConfiguration",
        godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_begin_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"),
        "set_visibility_range_begin_margin", "get_visibility_range_begin_margin");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_end", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"),
                                 "set_visibility_range_end", "get_visibility_range_end");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_end_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"),
                                 "set_visibility_range_end_margin", "get_visibility_range_end_margin");
    godot::ClassDB::add_property(
        "InstancedRenderer3DLODConfiguration",
        godot::PropertyInfo(godot::Variant::INT, "visibility_range_fade_mode", godot::PROPERTY_HINT_ENUM, "Disabled:0,Self:1,Dependencies:2"),
        "set_visibility_range_fade_mode", "get_visibility_range_fade_mode");

    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_DISABLED",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);
    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_SELF",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF);
    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_DEPENDENCIES",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_DEPENDENCIES);
}

bool InstancedRenderer3D::validate_configuration() const {
    bool valid = true;

    if (lod_levels.is_empty()) {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() +
                                              "': No LOD levels configured. At least one LOD level with a mesh is required.");
        valid = false;
    }

    if (prefabs_rendered.is_empty()) {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': 'prefabs_rendered' is empty.");
        valid = false;
    }

    for (int i = 0; i < lod_levels.size(); ++i) {
        godot::Ref<InstancedRenderer3DLODConfiguration> lod = lod_levels[i];
        if (!lod.is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) + " is null.");
            valid = false;
            continue;
        }

        if (!lod->get_mesh().is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has no mesh assigned.");
            valid = false;
        }

        if (lod->get_visibility_range_end() < lod->get_visibility_range_begin()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has visibility_range_end < visibility_range_begin. This may cause incorrect visibility ranges.");
        }

        if (lod->get_visibility_range_begin_margin() < 0.0f) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has negative visibility_range_begin_margin.");
        }

        if (lod->get_visibility_range_end_margin() < 0.0f) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has negative visibility_range_end_margin.");
        }

        const godot::RenderingServer::VisibilityRangeFadeMode visibility_fade_mode = lod->get_visibility_range_fade_mode();
        if (visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_DISABLED &&
            visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF &&
            visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_DEPENDENCIES) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has invalid visibility_range_fade_mode.");
            valid = false;
        }
    }

    return valid;
}

void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::rendering::Renderers &renderers, int &renderer_count) {
    if (!renderer->validate_configuration()) {
        return;
    }

    const godot::PackedStringArray prefabs = renderer->get_prefabs_rendered();
    const godot::TypedArray<InstancedRenderer3DLODConfiguration> &lod_levels = renderer->get_lod_levels();

    // Build a query for all prefabs with Transform3D
    auto query_builder = world.query_builder();
    query_builder.with<const Transform3D>();

    int prefab_count = prefabs.size();
    for (int j = 0; j < prefab_count; ++j) {
        godot::String prefab_name = prefabs[j];
        std::string prefab_name_str = prefab_name.utf8().get_data();
        flecs::entity prefab_entity = world.lookup(prefab_name_str.c_str());
        if (!prefab_entity.is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': Prefab not found: " + prefab_name);
            continue;
        }
        query_builder.with(flecs::IsA, prefab_entity);
        if (j < prefab_count - 1) {
            query_builder.or_();
        }
    }

    flecs::query<> query = query_builder.build();

    // Build LOD configuration for this renderer
    std::vector<stagehand::rendering::InstancedRendererLODConfig> lod_configs;
    lod_configs.reserve(lod_levels.size());
    for (int i = 0; i < lod_levels.size(); ++i) {
        godot::Ref<InstancedRenderer3DLODConfiguration> lod_resource = lod_levels[i];
        if (!lod_resource.is_valid()) {
            continue;
        }

        stagehand::rendering::InstancedRendererLODConfig lod_config;
        godot::Ref<godot::Mesh> mesh = lod_resource->get_mesh();
        if (mesh.is_valid()) {
            lod_config.mesh_rid = mesh->get_rid();
        }
        lod_config.visibility_range_begin = lod_resource->get_visibility_range_begin();
        lod_config.visibility_range_end = lod_resource->get_visibility_range_end();
        lod_config.visibility_range_begin_margin = lod_resource->get_visibility_range_begin_margin();
        lod_config.visibility_range_end_margin = lod_resource->get_visibility_range_end_margin();
        lod_config.visibility_range_fade_mode = lod_resource->get_visibility_range_fade_mode();
        lod_configs.push_back(lod_config);
    }

    // Get the scenario RID from the renderer's world_3d
    godot::RID scenario_rid;
    godot::Ref<godot::World3D> world_3d = renderer->get_world_3d();
    if (world_3d.is_valid()) {
        scenario_rid = world_3d->get_scenario();
    } else {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': Could not get World3D scenario.");
        return;
    }

    stagehand::rendering::InstancedRendererConfig config;
    config.scenario_rid = scenario_rid;
    config.lod_configs = std::move(lod_configs);
    config.query = query;
    renderers.instanced_renderers.push_back(std::move(config));
    renderer_count++;
}

void InstancedRenderer3D::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_prefabs_rendered", "prefabs"), &InstancedRenderer3D::set_prefabs_rendered);
    godot::ClassDB::bind_method(godot::D_METHOD("get_prefabs_rendered"), &InstancedRenderer3D::get_prefabs_rendered);

    godot::ClassDB::bind_method(godot::D_METHOD("set_lod_levels", "lod_levels"), &InstancedRenderer3D::set_lod_levels);
    godot::ClassDB::bind_method(godot::D_METHOD("get_lod_levels"), &InstancedRenderer3D::get_lod_levels);

    godot::ClassDB::add_property("InstancedRenderer3D", godot::PropertyInfo(godot::Variant::PACKED_STRING_ARRAY, "prefabs_rendered"), "set_prefabs_rendered",
                                 "get_prefabs_rendered");
    godot::ClassDB::add_property(
        "InstancedRenderer3D", godot::PropertyInfo(godot::Variant::ARRAY, "lod_levels", godot::PROPERTY_HINT_ARRAY_TYPE, "InstancedRenderer3DLODConfiguration"),
        "set_lod_levels", "get_lod_levels");
}
