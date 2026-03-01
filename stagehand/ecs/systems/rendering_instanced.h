#pragma once

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand::rendering {
    inline flecs::system EntityRenderingInstanced;

    REGISTER([](flecs::world &world) {
        struct InstanceUniformView {
            // untyped_field performs type erasure, allowing the system to hold a reference to the component data column regardless of its actual component type.
            // One entity might use InstanceColor (a component wrapping Vector4) as a uniform. Another might use InstanceEmission (a different component, also wrapping Vector4).
            flecs::untyped_field values;
            bool has_changed;
            bool is_shared;
            godot::StringName parameter_name;
        };

        // clang-format off
    stagehand::rendering::EntityRenderingInstanced = world.system(stagehand::names::systems::ENTITY_RENDERING_INSTANCED)
        .kind(stagehand::OnRender)
        .run([](flecs::iter &it) {
                // clang-format on
                if (!it.world().has<Renderers>()) {
                    return;
                }

                Renderers &renderers = it.world().ensure<Renderers>();

                if (renderers.instanced_renderers.empty()) {
                    return;
                }

                godot::RenderingServer *rendering_server = godot::RenderingServer::get_singleton();
                if (!rendering_server) {
                    godot::UtilityFunctions::push_error(godot::String(stagehand::names::systems::ENTITY_RENDERING_INSTANCED) +
                                                        ": RenderingServer singleton not available");
                    return;
                }

                for (InstancedRendererConfig &renderer : renderers.instanced_renderers) {
                    size_t lod_count = renderer.lod_configs.size();
                    if (lod_count == 0) {
                        continue;
                    }

                    // Count entities
                    size_t entity_count = 0;
                    if (renderer.query.is_true()) {
                        entity_count = static_cast<size_t>(renderer.query.count());
                    }

                    // Use a growth strategy (next power of 2) to avoid frequent reallocations when the instance count fluctuates.
                    size_t current_capacity = renderer.instance_rids.size() / lod_count;
                    size_t target_capacity = 16;
                    while (target_capacity < entity_count) {
                        target_capacity *= 2;
                    }

                    // Resize if outside hysteresis window (too small or much too large)
                    if (current_capacity < target_capacity || current_capacity > target_capacity * 2) {
                        size_t new_size = target_capacity * lod_count;
                        // Free RIDs if shrinking
                        if (new_size < renderer.instance_rids.size()) {
                            for (size_t i = new_size; i < renderer.instance_rids.size(); ++i) {
                                if (renderer.instance_rids[i].is_valid()) {
                                    rendering_server->free_rid(renderer.instance_rids[i]);
                                }
                            }
                        }
                        renderer.instance_rids.resize(new_size);
                    }

                    // Create new instances where needed and update transforms
                    size_t entity_index = 0;
                    renderer.query.run([&](flecs::iter &query_it) {
                        while (query_it.next()) {
                            auto transform_field = query_it.field<const stagehand::transform::Transform3D>(0);

                            std::vector<InstanceUniformView> instance_uniform_views;
                            instance_uniform_views.reserve(renderer.uniforms.size());

                            for (const auto &uniform_config : renderer.uniforms) {
                                if (query_it.is_set(uniform_config.value_field_index)) {
                                    if (query_it.size(uniform_config.value_field_index) != sizeof(godot::Vector4)) {
                                        continue;
                                    }

                                    flecs::untyped_field values_field = query_it.field(uniform_config.value_field_index);
                                    bool is_shared = !query_it.is_self(uniform_config.value_field_index);
                                    bool has_changed_field = false;
                                    if (uniform_config.changed_field_index >= 0) {
                                        has_changed_field = query_it.is_set(uniform_config.changed_field_index);
                                    }
                                    instance_uniform_views.push_back(
                                        InstanceUniformView{values_field, has_changed_field, is_shared, uniform_config.parameter_name});
                                }
                            }

                            for (auto i : query_it) {
                                if (entity_index >= entity_count) {
                                    break;
                                }

                                const stagehand::transform::Transform3D &transform = transform_field[i];

                                for (size_t lod = 0; lod < lod_count; ++lod) {
                                    size_t rid_index = entity_index * lod_count + lod;
                                    godot::RID &instance_rid = renderer.instance_rids[rid_index];

                                    bool is_new = !instance_rid.is_valid();
                                    // Create the instance if it doesn't exist yet
                                    if (is_new) {
                                        const InstancedRendererLODConfig &lod_config = renderer.lod_configs[lod];
                                        instance_rid = rendering_server->instance_create2(lod_config.mesh_rid, renderer.scenario_rid);

                                        // Set visibility range for this LOD level
                                        rendering_server->instance_geometry_set_visibility_range(
                                            instance_rid, lod_config.visibility_range_begin, lod_config.visibility_range_end,
                                            lod_config.visibility_range_begin_margin, lod_config.visibility_range_end_margin,
                                            lod_config.visibility_range_fade_mode);

                                        if (renderer.material_rid.is_valid()) {
                                            rendering_server->instance_geometry_set_material_override(instance_rid, renderer.material_rid);
                                        }
                                    }

                                    // Ensure visible if it was previously hidden or just created
                                    if (is_new || entity_index >= renderer.previous_entity_count) {
                                        rendering_server->instance_set_visible(instance_rid, true);
                                    }

                                    // Update the instance's transform
                                    if (is_new || query_it.is_set(1)) {
                                        rendering_server->instance_set_transform(instance_rid, transform);
                                    }

                                    // Update instance uniforms
                                    for (const auto &uniform_view : instance_uniform_views) {
                                        if (is_new || uniform_view.has_changed) {
                                            const void *raw_value = uniform_view.is_shared ? uniform_view.values[0] : uniform_view.values[i];
                                            const godot::Vector4 &val = *static_cast<const godot::Vector4 *>(raw_value); // std::memcpy(&val, raw_value, sizeof(godot::Vector4)); might be more robust
                                            rendering_server->instance_geometry_set_shader_parameter(instance_rid, uniform_view.parameter_name, val);
                                        }
                                    }
                                }

                                entity_index++;
                            }
                        }
                    });

                    // Hide instances that are no longer needed (but keep them allocated for reuse)
                    if (entity_count < renderer.previous_entity_count) {
                        size_t start_index = entity_count * lod_count;
                        size_t end_index = std::min(renderer.previous_entity_count * lod_count, renderer.instance_rids.size());

                        for (size_t i = start_index; i < end_index; ++i) {
                            if (renderer.instance_rids[i].is_valid()) {
                                rendering_server->instance_set_visible(renderer.instance_rids[i], false);
                            }
                        }
                    }

                    renderer.previous_entity_count = entity_count;
                }
            });
    });
} // namespace stagehand::rendering
