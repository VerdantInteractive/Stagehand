#include "stagehand/nodes/instanced_renderer_3d.h"

#include <unordered_set>

#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/traits.h"
#include "stagehand/ecs/components/transform.h"

void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::rendering::Renderers &renderers, int &renderer_count) {
    if (!renderer->validate_configuration()) {
        return;
    }

    const godot::PackedStringArray prefabs = renderer->get_prefabs_rendered();
    const godot::TypedArray<InstancedRenderer3DLODConfiguration> &lod_levels = renderer->get_lod_levels();

    std::vector<flecs::entity> prefab_entities;
    prefab_entities.reserve(prefabs.size());
    for (int i = 0; i < prefabs.size(); ++i) {
        const godot::String prefab_name = prefabs[i];
        const std::string prefab_name_str = prefab_name.utf8().get_data();
        flecs::entity prefab_entity = world.lookup(prefab_name_str.c_str());
        if (!prefab_entity.is_valid()) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': Prefab not found: " + prefab_name);
            continue;
        }
        prefab_entities.push_back(prefab_entity);
    }

    if (prefab_entities.empty()) {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': No valid prefabs found.");
        return;
    }

    const auto add_prefab_filters = [&](auto &builder) {
        for (size_t prefab_index = 0; prefab_index < prefab_entities.size(); ++prefab_index) {
            builder.with(flecs::IsA, prefab_entities[prefab_index]);
            if (prefab_index + 1 < prefab_entities.size()) {
                builder.or_();
            }
        }
    };

    auto reconcile_query_builder = world.query_builder<const stagehand::transform::Transform3D>();
    add_prefab_filters(reconcile_query_builder);

    auto transform_update_query_builder = world.query_builder<const stagehand::transform::Transform3D, const stagehand::transform::HasChangedTransform3D>();
    add_prefab_filters(transform_update_query_builder);

    std::vector<flecs::entity> found_instance_uniform_components;
    found_instance_uniform_components.reserve(16);
    std::unordered_set<ecs_entity_t> seen_uniform_component_ids;
    seen_uniform_component_ids.reserve(16);

    for (const flecs::entity prefab_entity : prefab_entities) {
        prefab_entity.each([&](flecs::id id) {
            if (id.is_pair()) {
                return;
            }

            const flecs::entity component = id.entity();
            if (!component.has<stagehand::rendering::IsInstanceUniform>()) {
                return;
            }

            if (seen_uniform_component_ids.insert(component.id()).second) {
                found_instance_uniform_components.push_back(component);
            }
        });
    }

    std::vector<stagehand::rendering::InstancedRendererConfig::UniformInitConfig> initial_uniform_configs;
    std::vector<stagehand::rendering::InstancedRendererConfig::UniformUpdateConfig> uniform_update_configs;
    initial_uniform_configs.reserve(found_instance_uniform_components.size());
    uniform_update_configs.reserve(found_instance_uniform_components.size());

    int current_field_index = 1 + static_cast<int>(prefab_entities.size());

    godot::PackedStringArray _discovered_instance_uniforms;
    for (int i = 0; i < found_instance_uniform_components.size(); ++i) {
        if (i >= 16) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() +
                                                  "': Too many instance uniforms. Only the first 16 will be used.");
            break;
        }

        flecs::entity instance_uniform_component = found_instance_uniform_components[i];
        std::string instance_uniform_component_name_str = instance_uniform_component.name().c_str();

        // Validate component size (must match Vector4)
        const flecs::Component *c_info = instance_uniform_component.try_get<flecs::Component>();
        if (!c_info || c_info->size != sizeof(godot::Vector4)) {
            godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': Component '" +
                                                  instance_uniform_component_name_str.c_str() +
                                                  "' has IsInstanceUniform trait but is not compatible with Vector4 (size mismatch).");
            continue;
        }

        _discovered_instance_uniforms.push_back(godot::String(instance_uniform_component_name_str.c_str()));

        flecs::entity changed_tag;
        instance_uniform_component.each(flecs::With, [&](flecs::entity target) {
            if (target.has<stagehand::IsChangeDetectionTag>()) {
                changed_tag = target;
            }
        });

        reconcile_query_builder.with(instance_uniform_component).in().optional();

        stagehand::rendering::InstancedRendererConfig::UniformInitConfig init_config;
        init_config.value_field_index = current_field_index;
        init_config.parameter_name = godot::StringName(instance_uniform_component_name_str.c_str());
        initial_uniform_configs.push_back(init_config);
        current_field_index += 1;

        auto uniform_update_query_builder = world.query_builder<>();
        add_prefab_filters(uniform_update_query_builder);
        uniform_update_query_builder.with(instance_uniform_component).in();
        if (changed_tag) {
            uniform_update_query_builder.with(changed_tag);
        }

        stagehand::rendering::InstancedRendererConfig::UniformUpdateConfig update_config;
        update_config.value_field_index = static_cast<int>(prefab_entities.size());
        update_config.parameter_name = godot::StringName(instance_uniform_component_name_str.c_str());
        update_config.query = uniform_update_query_builder.build();
        uniform_update_configs.push_back(std::move(update_config));
    }

    renderer->set_discovered_instance_uniforms(_discovered_instance_uniforms);

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

    godot::RID scenario_rid;
    godot::Ref<godot::World3D> world_3d = renderer->get_world_3d();
    if (world_3d.is_valid()) {
        scenario_rid = world_3d->get_scenario();
    } else {
        godot::UtilityFunctions::push_warning(godot::String("InstancedRenderer3D '") + renderer->get_name() + "': Could not get World3D scenario.");
        return;
    }

    godot::RID material_rid;
    if (renderer->get_material().is_valid()) {
        material_rid = renderer->get_material()->get_rid();
    }

    stagehand::rendering::InstancedRendererConfig config;
    config.scenario_rid = scenario_rid;
    config.material_rid = material_rid;
    config.lod_configs = std::move(lod_configs);
    config.reconcile_query = reconcile_query_builder.build();
    config.transform_update_query = transform_update_query_builder.build();
    config.initial_uniforms = std::move(initial_uniform_configs);
    config.uniform_updates = std::move(uniform_update_configs);
    renderers.instanced_renderers.push_back(std::move(config));
    renderer_count++;
}

InstancedRenderer3D::InstancedRenderer3D() {
    connect("tree_entered", callable_mp(this, &InstancedRenderer3D::on_enter_tree));
    connect("tree_exiting", callable_mp(this, &InstancedRenderer3D::on_exit_tree));
}

void InstancedRenderer3D::on_enter_tree() {
    const godot::Callable warnings_callable(this, "update_configuration_warnings");
    for (const godot::Ref<InstancedRenderer3DLODConfiguration> &lod_level : lod_levels) {
        if (lod_level.is_valid() && !lod_level->is_connected("changed", warnings_callable)) {
            lod_level->connect("changed", warnings_callable);
        }
    }
}

void InstancedRenderer3D::on_exit_tree() {
    const godot::Callable warnings_callable(this, "update_configuration_warnings");
    for (const godot::Ref<InstancedRenderer3DLODConfiguration> &lod_level : lod_levels) {
        if (lod_level.is_valid() && lod_level->is_connected("changed", warnings_callable)) {
            lod_level->disconnect("changed", warnings_callable);
        }
    }
}

void InstancedRenderer3D::set_prefabs_rendered(const godot::PackedStringArray &p_prefabs) {
    prefabs_rendered = p_prefabs;
    update_configuration_warnings();
}

void InstancedRenderer3D::set_lod_levels(const godot::TypedArray<InstancedRenderer3DLODConfiguration> &p_lod_levels) {
    if (is_inside_tree()) {
        const godot::Callable warnings_callable(this, "update_configuration_warnings");
        for (const godot::Ref<InstancedRenderer3DLODConfiguration> &previous_lod : lod_levels) {
            if (previous_lod.is_valid() && previous_lod->is_connected("changed", warnings_callable)) {
                previous_lod->disconnect("changed", warnings_callable);
            }
        }
    }

    lod_levels = p_lod_levels;

    if (is_inside_tree()) {
        const godot::Callable warnings_callable(this, "update_configuration_warnings");
        for (const godot::Ref<InstancedRenderer3DLODConfiguration> &lod_level : lod_levels) {
            if (lod_level.is_valid() && !lod_level->is_connected("changed", warnings_callable)) {
                lod_level->connect("changed", warnings_callable);
            }
        }
    }

    update_configuration_warnings();
}

godot::PackedStringArray InstancedRenderer3D::_get_configuration_warnings() const {
    godot::PackedStringArray warnings;

    if (lod_levels.is_empty()) {
        warnings.push_back("No LOD levels configured. At least one LOD level with a mesh is required.");
    }

    if (prefabs_rendered.is_empty()) {
        warnings.push_back("'prefabs_rendered' is empty.");
    }

    for (int i = 0; i < lod_levels.size(); ++i) {
        godot::Ref<InstancedRenderer3DLODConfiguration> lod = lod_levels[i];
        if (!lod.is_valid()) {
            warnings.push_back("LOD " + godot::String::num_int64(i) + " is null.");
            continue;
        }

        if (!lod->get_mesh().is_valid()) {
            warnings.push_back("LOD " + godot::String::num_int64(i) + " has no mesh assigned.");
        }

        if (lod->get_visibility_range_end() < lod->get_visibility_range_begin()) {
            warnings.push_back("LOD " + godot::String::num_int64(i) +
                               " has visibility_range_end < visibility_range_begin. This may cause incorrect visibility ranges.");
        }

        if (lod->get_visibility_range_begin_margin() < 0.0f) {
            warnings.push_back("LOD " + godot::String::num_int64(i) + " has negative visibility_range_begin_margin.");
        }

        if (lod->get_visibility_range_end_margin() < 0.0f) {
            warnings.push_back("LOD " + godot::String::num_int64(i) + " has negative visibility_range_end_margin.");
        }

        const godot::RenderingServer::VisibilityRangeFadeMode visibility_fade_mode = lod->get_visibility_range_fade_mode();
        if (visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_DISABLED &&
            visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF &&
            visibility_fade_mode != godot::RenderingServer::VISIBILITY_RANGE_FADE_DEPENDENCIES) {
            warnings.push_back("LOD " + godot::String::num_int64(i) + " has invalid visibility_range_fade_mode.");
        }
    }

    return warnings;
}

bool InstancedRenderer3D::validate_configuration() const {
    if (lod_levels.is_empty() || prefabs_rendered.is_empty()) {
        return false;
    }

    for (int i = 0; i < lod_levels.size(); ++i) {
        godot::Ref<InstancedRenderer3DLODConfiguration> lod = lod_levels[i];
        if (!lod.is_valid() || !lod->get_mesh().is_valid()) {
            return false;
        }
    }

    return true;
}

void InstancedRenderer3D::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_prefabs_rendered", "prefabs"), &InstancedRenderer3D::set_prefabs_rendered);
    godot::ClassDB::bind_method(godot::D_METHOD("get_prefabs_rendered"), &InstancedRenderer3D::get_prefabs_rendered);

    godot::ClassDB::bind_method(godot::D_METHOD("set_lod_levels", "lod_levels"), &InstancedRenderer3D::set_lod_levels);
    godot::ClassDB::bind_method(godot::D_METHOD("get_lod_levels"), &InstancedRenderer3D::get_lod_levels);

    godot::ClassDB::bind_method(godot::D_METHOD("set_material", "material"), &InstancedRenderer3D::set_material);
    godot::ClassDB::bind_method(godot::D_METHOD("get_material"), &InstancedRenderer3D::get_material);

    godot::ClassDB::bind_method(godot::D_METHOD("set_discovered_instance_uniforms", "uniforms"), &InstancedRenderer3D::set_discovered_instance_uniforms);
    godot::ClassDB::bind_method(godot::D_METHOD("get_discovered_instance_uniforms"), &InstancedRenderer3D::get_discovered_instance_uniforms);

    godot::ClassDB::add_property("InstancedRenderer3D", godot::PropertyInfo(godot::Variant::PACKED_STRING_ARRAY, "prefabs_rendered"), "set_prefabs_rendered",
                                 "get_prefabs_rendered");
    godot::ClassDB::add_property(
        "InstancedRenderer3D", godot::PropertyInfo(godot::Variant::ARRAY, "lod_levels", godot::PROPERTY_HINT_ARRAY_TYPE, "InstancedRenderer3DLODConfiguration"),
        "set_lod_levels", "get_lod_levels");
    godot::ClassDB::add_property("InstancedRenderer3D", godot::PropertyInfo(godot::Variant::OBJECT, "material", godot::PROPERTY_HINT_RESOURCE_TYPE, "Material"),
                                 "set_material", "get_material");
    godot::ClassDB::add_property("InstancedRenderer3D",
                                 godot::PropertyInfo(godot::Variant::PACKED_STRING_ARRAY, "discovered_instance_uniforms", godot::PROPERTY_HINT_NONE, "",
                                                     godot::PROPERTY_USAGE_EDITOR | godot::PROPERTY_USAGE_READ_ONLY),
                                 "set_discovered_instance_uniforms", "get_discovered_instance_uniforms");
}
