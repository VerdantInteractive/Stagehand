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

    // Flecs entity ids are versioned. We use the stable low bits as the direct lookup index and then validate the full id at the slot to reject stale entries
    // after entity recycling.
    inline size_t get_entity_lookup_index(ecs_entity_t entity_id) { return static_cast<size_t>(ecs_strip_generation(entity_id)); }

    inline void ensure_entity_slot_lookup_capacity(InstancedRendererConfig &renderer, ecs_entity_t entity_id) {
        const size_t lookup_index = get_entity_lookup_index(entity_id);
        if (lookup_index < renderer.slot_by_entity_id.size()) {
            return;
        }

        size_t new_capacity = renderer.slot_by_entity_id.empty() ? 64 : renderer.slot_by_entity_id.size();
        while (new_capacity <= lookup_index) {
            new_capacity *= 2;
        }

        renderer.slot_by_entity_id.resize(new_capacity, InstancedRendererConfig::INVALID_SLOT);
    }

    inline uint32_t try_get_entity_slot(const InstancedRendererConfig &renderer, ecs_entity_t entity_id) {
        const size_t lookup_index = get_entity_lookup_index(entity_id);
        if (lookup_index >= renderer.slot_by_entity_id.size()) {
            return InstancedRendererConfig::INVALID_SLOT;
        }

        const uint32_t slot_index = renderer.slot_by_entity_id[lookup_index];
        if (slot_index == InstancedRendererConfig::INVALID_SLOT || slot_index >= renderer.slot_entities.size()) {
            return InstancedRendererConfig::INVALID_SLOT;
        }

        // Compare against the full Flecs id, including generation/version, so a recycled entity record cannot resolve to the previous occupant.
        return renderer.slot_entities[slot_index] == entity_id ? slot_index : InstancedRendererConfig::INVALID_SLOT;
    }

    inline void assign_entity_slot(InstancedRendererConfig &renderer, ecs_entity_t entity_id, uint32_t slot_index) {
        ensure_entity_slot_lookup_capacity(renderer, entity_id);
        renderer.slot_by_entity_id[get_entity_lookup_index(entity_id)] = slot_index;
    }

    inline void clear_entity_slot(InstancedRendererConfig &renderer, ecs_entity_t entity_id, uint32_t slot_index) {
        const size_t lookup_index = get_entity_lookup_index(entity_id);
        if (lookup_index < renderer.slot_by_entity_id.size() && renderer.slot_by_entity_id[lookup_index] == slot_index) {
            renderer.slot_by_entity_id[lookup_index] = InstancedRendererConfig::INVALID_SLOT;
        }
    }

    inline void ensure_instanced_slot_capacity(InstancedRendererConfig &renderer, uint32_t required_slot_count, size_t lod_count) {
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

    inline void ensure_slot_instances(InstancedRendererConfig &renderer, uint32_t slot_index, godot::RenderingServer *rendering_server) {
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

    inline void set_slot_visibility(const InstancedRendererConfig &renderer, uint32_t slot_index, bool visible, godot::RenderingServer *rendering_server) {
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
                                   uint32_t slot_index,
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
                                 uint32_t slot_index,
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

                    // Reconcile (Mark): Iterates all relevant entities to establish liveness.
                    renderer.reconcile_query.run([&](flecs::iter &query_it) {
                        while (query_it.next()) {
                            auto transform_field = query_it.field<const stagehand::transform::Transform3D>(0);

                            for (auto i : query_it) {
                                const ecs_entity_t entity_id = query_it.entity(i).id();
                                const stagehand::transform::Transform3D &transform = transform_field[i];

                                const uint32_t existing_slot = try_get_entity_slot(renderer, entity_id);
                                if (existing_slot != InstancedRendererConfig::INVALID_SLOT) {
                                    renderer.slot_generations[existing_slot] = renderer.current_generation;
                                    continue;
                                }

                                if (renderer.free_slots.empty()) {
                                    ensure_instanced_slot_capacity(renderer, renderer.active_entity_count + 1, lod_count);
                                }

                                uint32_t slot_index = 0;
                                if (!renderer.free_slots.empty()) {
                                    slot_index = renderer.free_slots.back();
                                    renderer.free_slots.pop_back();
                                } else {
                                    slot_index = renderer.active_entity_count;
                                }

                                assign_entity_slot(renderer, entity_id, slot_index);
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

                    // This stays as an indexed loop because slot_index is the key that ties slot_entities, instance_rids and the generation arrays together.
                    for (uint32_t slot_index = 0; slot_index < renderer.slot_entities.size(); ++slot_index) {
                        const ecs_entity_t entity_id = renderer.slot_entities[slot_index];
                        if (entity_id == 0) {
                            continue;
                        }

                        if (renderer.slot_generations[slot_index] == renderer.current_generation) {
                            continue;
                        }

                        set_slot_visibility(renderer, slot_index, false, rendering_server);
                        clear_entity_slot(renderer, entity_id, slot_index);
                        renderer.slot_entities[slot_index] = 0;
                        renderer.slot_generations[slot_index] = 0;
                        renderer.slot_created_generations[slot_index] = 0;
                        renderer.free_slots.push_back(slot_index);
                        renderer.active_entity_count -= 1;
                    }

                    renderer.transform_update_query.run([&](flecs::iter &query_it) {
                        while (query_it.next()) {
                            auto transform_field = query_it.field<const stagehand::transform::Transform3D>(0);
                            for (auto i : query_it) {
                                const ecs_entity_t entity_id = query_it.entity(i).id();
                                const uint32_t slot_index = try_get_entity_slot(renderer, entity_id);
                                if (slot_index == InstancedRendererConfig::INVALID_SLOT) {
                                    continue;
                                }

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
                                    const uint32_t slot_index = try_get_entity_slot(renderer, entity_id);
                                    if (slot_index == InstancedRendererConfig::INVALID_SLOT) {
                                        continue;
                                    }

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
