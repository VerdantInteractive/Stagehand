#include "stagehand/nodes/instanced_renderer_3d.h"

#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/transform.h"

bool InstancedRenderer3D::validate_configuration() const {
    bool valid = true;

    if (lod_levels.empty()) {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() +
                                              "': No LOD levels configured. At least one LOD level with a mesh is required.");
        valid = false;
    }

    if (prefabs_rendered.is_empty()) {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': 'prefabs_rendered' is empty.");
        valid = false;
    }

    for (int i = 0; i < static_cast<int>(lod_levels.size()); ++i) {
        const InstancedRendererLOD &lod = lod_levels[i];
        if (!lod.mesh.is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has no mesh assigned.");
            valid = false;
        }

        if (lod.fade_max < lod.fade_min) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has fade_max < fade_min. This may cause incorrect visibility ranges.");
        }

        if (lod.fade_min_margin < 0.0f) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has negative fade_min_margin.");
        }

        if (lod.fade_max_margin < 0.0f) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + get_name() + "': LOD " + godot::String::num_int64(i) +
                                                  " has negative fade_max_margin.");
        }
    }

    return valid;
}

void InstancedRenderer3D::set_lod_count(int p_count) {
    if (p_count < 0) {
        p_count = 0;
    }
    if (p_count > MAX_LOD_LEVELS) {
        p_count = MAX_LOD_LEVELS;
    }
    lod_levels.resize(static_cast<size_t>(p_count));
    notify_property_list_changed();
}

void InstancedRenderer3D::set_lod_mesh(int index, const godot::Ref<godot::Mesh> &mesh) {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        lod_levels[index].mesh = mesh;
    }
}

godot::Ref<godot::Mesh> InstancedRenderer3D::get_lod_mesh(int index) const {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        return lod_levels[index].mesh;
    }
    return {};
}

void InstancedRenderer3D::set_lod_fade_min(int index, float value) {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        lod_levels[index].fade_min = value;
    }
}

float InstancedRenderer3D::get_lod_fade_min(int index) const {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        return lod_levels[index].fade_min;
    }
    return 0.0f;
}

void InstancedRenderer3D::set_lod_fade_max(int index, float value) {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        lod_levels[index].fade_max = value;
    }
}

float InstancedRenderer3D::get_lod_fade_max(int index) const {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        return lod_levels[index].fade_max;
    }
    return 0.0f;
}

void InstancedRenderer3D::set_lod_fade_min_margin(int index, float value) {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        lod_levels[index].fade_min_margin = value;
    }
}

float InstancedRenderer3D::get_lod_fade_min_margin(int index) const {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        return lod_levels[index].fade_min_margin;
    }
    return 0.0f;
}

void InstancedRenderer3D::set_lod_fade_max_margin(int index, float value) {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        lod_levels[index].fade_max_margin = value;
    }
}

float InstancedRenderer3D::get_lod_fade_max_margin(int index) const {
    if (index >= 0 && index < static_cast<int>(lod_levels.size())) {
        return lod_levels[index].fade_max_margin;
    }
    return 0.0f;
}

// Dynamic property handling for LOD levels using _set/_get/_get_property_list
bool InstancedRenderer3D::_set(const godot::StringName &p_name, const godot::Variant &p_value) {
    godot::String name = p_name;

    if (name == "lod_count") {
        set_lod_count(p_value);
        return true;
    }

    // Parse "lod_N/property" pattern
    if (name.begins_with("lod_")) {
        int slash_pos = name.find("/");
        if (slash_pos == -1) {
            return false;
        }

        godot::String index_str = name.substr(4, slash_pos - 4);
        int index = index_str.to_int();
        godot::String property = name.substr(slash_pos + 1);

        if (index < 0 || index >= static_cast<int>(lod_levels.size())) {
            return false;
        }

        if (property == "mesh") {
            set_lod_mesh(index, p_value);
            return true;
        }
        if (property == "fade_min") {
            set_lod_fade_min(index, p_value);
            return true;
        }
        if (property == "fade_max") {
            set_lod_fade_max(index, p_value);
            return true;
        }
        if (property == "fade_min_margin") {
            set_lod_fade_min_margin(index, p_value);
            return true;
        }
        if (property == "fade_max_margin") {
            set_lod_fade_max_margin(index, p_value);
            return true;
        }
    }

    return false;
}

bool InstancedRenderer3D::_get(const godot::StringName &p_name, godot::Variant &r_ret) const {
    godot::String name = p_name;

    if (name == "lod_count") {
        r_ret = get_lod_count();
        return true;
    }

    if (name.begins_with("lod_")) {
        int slash_pos = name.find("/");
        if (slash_pos == -1) {
            return false;
        }

        godot::String index_str = name.substr(4, slash_pos - 4);
        int index = index_str.to_int();
        godot::String property = name.substr(slash_pos + 1);

        if (index < 0 || index >= static_cast<int>(lod_levels.size())) {
            return false;
        }

        if (property == "mesh") {
            r_ret = get_lod_mesh(index);
            return true;
        }
        if (property == "fade_min") {
            r_ret = get_lod_fade_min(index);
            return true;
        }
        if (property == "fade_max") {
            r_ret = get_lod_fade_max(index);
            return true;
        }
        if (property == "fade_min_margin") {
            r_ret = get_lod_fade_min_margin(index);
            return true;
        }
        if (property == "fade_max_margin") {
            r_ret = get_lod_fade_max_margin(index);
            return true;
        }
    }

    return false;
}

void InstancedRenderer3D::_get_property_list(godot::List<godot::PropertyInfo> *p_list) const {
    // LOD count property
    p_list->push_back(godot::PropertyInfo(godot::Variant::INT, "lod_count", godot::PROPERTY_HINT_RANGE,
                                          godot::String("0,") + godot::String::num_int64(MAX_LOD_LEVELS) + ",1"));

    // Per-LOD properties
    for (int i = 0; i < static_cast<int>(lod_levels.size()); ++i) {
        godot::String prefix = "lod_" + godot::String::num_int64(i) + "/";

        p_list->push_back(godot::PropertyInfo(godot::Variant::OBJECT, prefix + "mesh", godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"));
        p_list->push_back(godot::PropertyInfo(godot::Variant::FLOAT, prefix + "fade_min", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"));
        p_list->push_back(godot::PropertyInfo(godot::Variant::FLOAT, prefix + "fade_max", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"));
        p_list->push_back(godot::PropertyInfo(godot::Variant::FLOAT, prefix + "fade_min_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"));
        p_list->push_back(godot::PropertyInfo(godot::Variant::FLOAT, prefix + "fade_max_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"));
    }
}

void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::entity_rendering::Renderers &renderers, int &renderer_count) {
    if (!renderer->validate_configuration()) {
        return;
    }

    const godot::PackedStringArray prefabs = renderer->get_prefabs_rendered();
    const std::vector<InstancedRendererLOD> &lod_levels = renderer->get_lod_levels();

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
    std::vector<stagehand::entity_rendering::InstancedRendererLODConfig> lod_configs;
    lod_configs.reserve(lod_levels.size());
    for (const InstancedRendererLOD &lod : lod_levels) {
        stagehand::entity_rendering::InstancedRendererLODConfig lod_config;
        if (lod.mesh.is_valid()) {
            lod_config.mesh_rid = lod.mesh->get_rid();
        }
        lod_config.fade_min = lod.fade_min;
        lod_config.fade_max = lod.fade_max;
        lod_config.fade_min_margin = lod.fade_min_margin;
        lod_config.fade_max_margin = lod.fade_max_margin;
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

    // Use the InstancedRenderer3D's own object ID as a unique key for the renderer map
    // We use a RID constructed from the first LOD mesh since we need a RID as key
    godot::RID renderer_key;
    if (!lod_configs.empty()) {
        renderer_key = lod_configs[0].mesh_rid;
    }

    stagehand::entity_rendering::InstancedRendererConfig config;
    config.scenario_rid = scenario_rid;
    config.lod_configs = std::move(lod_configs);
    config.query = query;
    renderers.instanced_renderers.push_back(std::move(config));
    renderer_count++;
}

void InstancedRenderer3D::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_prefabs_rendered", "prefabs"), &InstancedRenderer3D::set_prefabs_rendered);
    godot::ClassDB::bind_method(godot::D_METHOD("get_prefabs_rendered"), &InstancedRenderer3D::get_prefabs_rendered);

    godot::ClassDB::bind_method(godot::D_METHOD("set_lod_count", "count"), &InstancedRenderer3D::set_lod_count);
    godot::ClassDB::bind_method(godot::D_METHOD("get_lod_count"), &InstancedRenderer3D::get_lod_count);

    godot::ClassDB::add_property("InstancedRenderer3D", godot::PropertyInfo(godot::Variant::PACKED_STRING_ARRAY, "prefabs_rendered"), "set_prefabs_rendered",
                                 "get_prefabs_rendered");
}
