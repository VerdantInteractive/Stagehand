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

    inline void ensure_instanced_slot_capacity(InstancedRendererConfig &renderer, size_t required_slot_count, size_t lod_count) {
        size_t current_capacity = renderer.slot_entities.size();
        if (current_capacity >= required_slot_count) {
            return;
        }

        size_t new_capacity = current_capacity == 0 ? 16 : current_capacity;
        while (new_capacity < required_slot_count) {
            new_capacity *= 2;
        }

        renderer.slot_entities.resize(new_capacity, 0);
        renderer.slot_generations.resize(new_capacity, 0);
        renderer.slot_created_generations.resize(new_capacity, 0);
        renderer.instance_rids.resize(new_capacity * lod_count);
    }

    inline void ensure_slot_instances(InstancedRendererConfig &renderer, size_t slot_index, godot::RenderingServer *rendering_server) {
        const size_t lod_count = renderer.lod_configs.size();
        const size_t slot_offset = slot_index * lod_count;

        for (size_t lod_index = 0; lod_index < lod_count; ++lod_index) {
            godot::RID &instance_rid = renderer.instance_rids[slot_offset + lod_index];
            if (instance_rid.is_valid()) {
                continue;
            }

            const InstancedRendererLODConfig &lod_config = renderer.lod_configs[lod_index];
            instance_rid = rendering_server->instance_create2(lod_config.mesh_rid, renderer.scenario_rid);
            rendering_server->instance_geometry_set_visibility_range(instance_rid, lod_config.visibility_range_begin, lod_config.visibility_range_end,
                                                                     lod_config.visibility_range_begin_margin, lod_config.visibility_range_end_margin,
                                                                     lod_config.visibility_range_fade_mode);

            if (renderer.material_rid.is_valid()) {
                rendering_server->instance_geometry_set_material_override(instance_rid, renderer.material_rid);
            }
        }
    }

    inline void set_slot_visibility(const InstancedRendererConfig &renderer, size_t slot_index, bool visible, godot::RenderingServer *rendering_server) {
        const size_t lod_count = renderer.lod_configs.size();
        const size_t slot_offset = slot_index * lod_count;

        for (size_t lod_index = 0; lod_index < lod_count; ++lod_index) {
            const godot::RID &instance_rid = renderer.instance_rids[slot_offset + lod_index];
            if (instance_rid.is_valid()) {
                rendering_server->instance_set_visible(instance_rid, visible);
            }
        }
    }

    inline void set_slot_transform(const InstancedRendererConfig &renderer,
                                   size_t slot_index,
                                   const stagehand::transform::Transform3D &transform,
                                   godot::RenderingServer *rendering_server) {
        const size_t lod_count = renderer.lod_configs.size();
        const size_t slot_offset = slot_index * lod_count;

        for (size_t lod_index = 0; lod_index < lod_count; ++lod_index) {
            const godot::RID &instance_rid = renderer.instance_rids[slot_offset + lod_index];
            if (instance_rid.is_valid()) {
                rendering_server->instance_set_transform(instance_rid, transform);
            }
        }
    }

    inline void set_slot_uniform(const InstancedRendererConfig &renderer,
                                 size_t slot_index,
                                 const godot::StringName &parameter_name,
                                 const godot::Vector4 &value,
                                 godot::RenderingServer *rendering_server) {
        const size_t lod_count = renderer.lod_configs.size();
        const size_t slot_offset = slot_index * lod_count;

        for (size_t lod_index = 0; lod_index < lod_count; ++lod_index) {
            const godot::RID &instance_rid = renderer.instance_rids[slot_offset + lod_index];
            if (instance_rid.is_valid()) {
                rendering_server->instance_geometry_set_shader_parameter(instance_rid, parameter_name, value);
            }
        }
    }

    REGISTER([](flecs::world &world) {
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
                    const size_t lod_count = renderer.lod_configs.size();
                    if (lod_count == 0) {
                        continue;
                    }

                    renderer.current_generation += 1;
                    if (renderer.current_generation == 0) {
                        renderer.current_generation = 1;
                    }

                    renderer.reconcile_query.run([&](flecs::iter &query_it) {
                        while (query_it.next()) {
                            auto transform_field = query_it.field<const stagehand::transform::Transform3D>(0);

                            for (auto i : query_it) {
                                const ecs_entity_t entity_id = query_it.entity(i).id();
                                const stagehand::transform::Transform3D &transform = transform_field[i];

                                auto existing_slot = renderer.entity_to_slot.find(entity_id);
                                if (existing_slot != renderer.entity_to_slot.end()) {
                                    renderer.slot_generations[existing_slot->second] = renderer.current_generation;
                                    continue;
                                }

                                if (renderer.free_slots.empty()) {
                                    ensure_instanced_slot_capacity(renderer, renderer.active_entity_count + 1, lod_count);
                                }

                                size_t slot_index = 0;
                                if (!renderer.free_slots.empty()) {
                                    slot_index = renderer.free_slots.back();
                                    renderer.free_slots.pop_back();
                                } else {
                                    slot_index = renderer.active_entity_count;
                                }

                                renderer.entity_to_slot[entity_id] = slot_index;
                                renderer.slot_entities[slot_index] = entity_id;
                                renderer.slot_generations[slot_index] = renderer.current_generation;
                                renderer.slot_created_generations[slot_index] = renderer.current_generation;
                                renderer.active_entity_count += 1;

                                ensure_slot_instances(renderer, slot_index, rendering_server);
                                set_slot_visibility(renderer, slot_index, true, rendering_server);
                                set_slot_transform(renderer, slot_index, transform, rendering_server);

                                for (const InstancedRendererConfig::UniformInitConfig &uniform_config : renderer.initial_uniforms) {
                                    godot::Vector4 uniform_value;
                                    if (query_it.is_set(uniform_config.value_field_index)) {
                                        flecs::untyped_field values_field = query_it.field(uniform_config.value_field_index);
                                        const bool is_shared = !query_it.is_self(uniform_config.value_field_index);
                                        const void *raw_value = is_shared ? values_field[0] : values_field[i];
                                        uniform_value = *static_cast<const godot::Vector4 *>(raw_value);
                                    }

                                    set_slot_uniform(renderer, slot_index, uniform_config.parameter_name, uniform_value, rendering_server);
                                }
                            }
                        }
                    });

                    for (auto slot_it = renderer.entity_to_slot.begin(); slot_it != renderer.entity_to_slot.end();) {
                        const size_t slot_index = slot_it->second;
                        if (renderer.slot_generations[slot_index] == renderer.current_generation) {
                            ++slot_it;
                            continue;
                        }

                        set_slot_visibility(renderer, slot_index, false, rendering_server);
                        renderer.slot_entities[slot_index] = 0;
                        renderer.slot_generations[slot_index] = 0;
                        renderer.slot_created_generations[slot_index] = 0;
                        renderer.free_slots.push_back(slot_index);
                        renderer.active_entity_count -= 1;
                        slot_it = renderer.entity_to_slot.erase(slot_it);
                    }

                    renderer.transform_update_query.run([&](flecs::iter &query_it) {
                        while (query_it.next()) {
                            auto transform_field = query_it.field<const stagehand::transform::Transform3D>(0);
                            for (auto i : query_it) {
                                const ecs_entity_t entity_id = query_it.entity(i).id();
                                const auto slot_it = renderer.entity_to_slot.find(entity_id);
                                if (slot_it == renderer.entity_to_slot.end()) {
                                    continue;
                                }

                                const size_t slot_index = slot_it->second;
                                if (renderer.slot_created_generations[slot_index] == renderer.current_generation) {
                                    continue;
                                }

                                set_slot_transform(renderer, slot_index, transform_field[i], rendering_server);
                            }
                        }
                    });

                    for (const InstancedRendererConfig::UniformUpdateConfig &uniform_config : renderer.uniform_updates) {
                        uniform_config.query.run([&](flecs::iter &query_it) {
                            while (query_it.next()) {
                                flecs::untyped_field values_field = query_it.field(uniform_config.value_field_index);
                                const bool is_shared = !query_it.is_self(uniform_config.value_field_index);

                                for (auto i : query_it) {
                                    const ecs_entity_t entity_id = query_it.entity(i).id();
                                    const auto slot_it = renderer.entity_to_slot.find(entity_id);
                                    if (slot_it == renderer.entity_to_slot.end()) {
                                        continue;
                                    }

                                    const size_t slot_index = slot_it->second;
                                    if (renderer.slot_created_generations[slot_index] == renderer.current_generation) {
                                        continue;
                                    }

                                    const void *raw_value = is_shared ? values_field[0] : values_field[i];
                                    const godot::Vector4 &value = *static_cast<const godot::Vector4 *>(raw_value);
                                    set_slot_uniform(renderer, slot_index, uniform_config.parameter_name, value, rendering_server);
                                }
                            }
                        });
                    }
                }
            });
    });
} // namespace stagehand::rendering
