#pragma once

#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/godot_signal.h"
#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/scene_children.h"
#include "stagehand/ecs/components/world_configuration.h"
#include "stagehand/ecs/pipeline_phases.h"

#include "components.h"
#include "names.h"

namespace stagehand_tests {

    REGISTER([](flecs::world &world) {
        // ── TickCount singleton ──────────────────────────────────────────────
        // Needed so that GDScript can read the singleton via get_component.
        world.set<TickCount>({0});

        // ── Tick Counter system ──────────────────────────────────────────────
        // Increments a singleton counter every frame. Used by system_control tests
        // to verify enable/disable and that progress() actually runs systems.
        world.system(names::systems::TICK_COUNTER).kind(flecs::OnUpdate).run([](flecs::iter &it) {
            flecs::world world = it.world();
            TickCount &tick_count = world.ensure<TickCount>();
            tick_count.value++;
        });

        // ── Emit Test Signal (on-demand) ─────────────────────────────────────
        // An on-demand system that emits a GodotSignal when run from GDScript
        // via run_system(). The signal name and data come from the parameters.
        world.system(names::systems::EMIT_TEST_SIGNAL)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || parameters->is_empty()) {
                    godot::UtilityFunctions::push_warning("Emit Test Signal: called without parameters.");
                    return;
                }

                godot::StringName signal_name = (*parameters).get("signal_name", "test_signal");
                godot::Dictionary signal_data;
                if (parameters->has("signal_data")) {
                    signal_data = (*parameters)["signal_data"];
                }

                // Create a temporary entity to emit the signal from
                flecs::world world = it.world();
                flecs::entity signal_source = world.entity("stagehand_tests::SignalSource");
                emit_godot_signal(world, signal_source, signal_name, signal_data);
            });

        // ── Read Scene Children (on-demand) ──────────────────────────────────
        // An on-demand system that reads the SceneChildren singleton and stores
        // the results in a SceneChildrenResult singleton for GDScript to read.
        world.set<SceneChildrenResult>({godot::Dictionary()});

        world.system(names::systems::READ_SCENE_CHILDREN)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                flecs::world world = it.world();
                const stagehand::SceneChildren *children = world.try_get<stagehand::SceneChildren>();
                if (!children) {
                    godot::UtilityFunctions::push_warning("Read Scene Children: SceneChildren singleton not found.");
                    return;
                }

                godot::Dictionary result;
                godot::Array keys = static_cast<const godot::Dictionary &>(*children).keys();
                result["count"] = keys.size();
                result["names"] = keys;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });

        // ── Accumulator (on-demand) ──────────────────────────────────────────
        // An on-demand system that adds a value to the AccumulatorValue singleton.
        // Used to test run_system with parameters.
        world.set<AccumulatorValue>({0});

        world.system(names::systems::ACCUMULATOR)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("amount")) {
                    godot::UtilityFunctions::push_warning("Accumulator: 'amount' parameter required.");
                    return;
                }

                int amount = (*parameters)["amount"];
                flecs::world world = it.world();
                AccumulatorValue &acc = world.ensure<AccumulatorValue>();
                acc.value += amount;
            });

        // ── Sum Query (on-demand) ─────────────────────────────────────────────
        // An on-demand system that computes the sum of EntityValue across all
        // entities and stores it in AccumulatorValue. Tests queries over
        // instantiated entities.
        world.system(names::systems::SUM_QUERY)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                flecs::world world = it.world();
                int sum = 0;
                world.each<const EntityValue>([&sum](const EntityValue &v) { sum += static_cast<int>(v.value); });
                AccumulatorValue &acc = world.ensure<AccumulatorValue>();
                acc.value = sum;
            });

        // ── Toggle Tag (on-demand) ────────────────────────────────────────────
        // An on-demand system that adds MarkerA to all entities with MarkerB
        // and removes MarkerB. Used to test tag operations across the boundary.
        world.system(names::systems::TOGGLE_TAG)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                flecs::world world = it.world();
                auto query = world.query_builder<>().with<MarkerB>().build();
                query.each([](flecs::entity e) {
                    e.add<MarkerA>();
                    e.remove<MarkerB>();
                });
            });

        // ── Count Rendered Entities (on-demand) ──────────────────────────────
        // Counts how many entities exist for a given prefab, and stores the count
        // plus some entity data in AccumulatorValue / a result dictionary.
        // Parameters: { "prefab": "prefab_name" }
        // Result: stores count in AccumulatorValue, and a summary Dictionary in SceneChildrenResult
        world.system(names::systems::COUNT_RENDERED_ENTITIES)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("prefab")) {
                    godot::UtilityFunctions::push_warning("Count Rendered Entities: 'prefab' parameter required.");
                    return;
                }

                godot::String prefab_name = (*parameters)["prefab"];
                std::string prefab_name_str = prefab_name.utf8().get_data();

                flecs::world world = it.world();
                flecs::entity prefab = world.lookup(prefab_name_str.c_str());
                if (!prefab.is_valid()) {
                    godot::UtilityFunctions::push_warning("Count Rendered Entities: prefab not found: " + prefab_name);
                    world.ensure<AccumulatorValue>().value = 0;
                    return;
                }

                // Count entities that are instances of this prefab
                auto query = world.query_builder<>().with(flecs::IsA, prefab).build();
                int count = static_cast<int>(query.count());
                world.ensure<AccumulatorValue>().value = count;
            });

        // ── Query Entity Transforms (on-demand) ─────────────────────────────
        // Queries entities of a given prefab and returns their transform data
        // as a Dictionary stored in SceneChildrenResult.
        // Parameters: { "prefab": "prefab_name", "dimension": "2d" or "3d" }
        // Result Dictionary: { "count": N, "transforms": [...], "has_colors": bool, "has_custom_data": bool }
        world.system(names::systems::QUERY_ENTITY_TRANSFORMS)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("prefab")) {
                    godot::UtilityFunctions::push_warning("Query Entity Transforms: 'prefab' parameter required.");
                    return;
                }

                godot::String prefab_name = (*parameters)["prefab"];
                godot::String dimension = parameters->get("dimension", "2d");
                std::string prefab_name_str = prefab_name.utf8().get_data();

                flecs::world world = it.world();
                flecs::entity prefab = world.lookup(prefab_name_str.c_str());
                if (!prefab.is_valid()) {
                    godot::UtilityFunctions::push_warning("Query Entity Transforms: prefab not found: " + prefab_name);
                    world.set<SceneChildrenResult>(SceneChildrenResult(godot::Dictionary()));
                    return;
                }

                godot::Dictionary result;
                godot::Array transforms;
                godot::Array colors;
                godot::Array custom_data_arr;
                bool has_colors = false;
                bool has_custom_data = false;
                int count = 0;

                if (dimension == "2d") {
                    auto query = world.query_builder<const Transform2D>().with(flecs::IsA, prefab).build();
                    query.each([&](flecs::entity e, const Transform2D &t) {
                        godot::Dictionary entry;
                        entry["origin_x"] = t.get_origin().x;
                        entry["origin_y"] = t.get_origin().y;
                        transforms.push_back(entry);

                        const Color *color = e.try_get<Color>();
                        if (color) {
                            has_colors = true;
                            colors.push_back(*color);
                        }

                        const stagehand::rendering::CustomData *cd = e.try_get<stagehand::rendering::CustomData>();
                        if (cd) {
                            has_custom_data = true;
                            custom_data_arr.push_back(godot::Vector4(cd->x, cd->y, cd->z, cd->w));
                        }

                        count++;
                    });
                } else {
                    auto query = world.query_builder<const Transform3D>().with(flecs::IsA, prefab).build();
                    query.each([&](flecs::entity e, const Transform3D &t) {
                        godot::Dictionary entry;
                        entry["origin_x"] = t.origin.x;
                        entry["origin_y"] = t.origin.y;
                        entry["origin_z"] = t.origin.z;
                        transforms.push_back(entry);

                        const Color *color = e.try_get<Color>();
                        if (color) {
                            has_colors = true;
                            colors.push_back(*color);
                        }

                        const stagehand::rendering::CustomData *cd = e.try_get<stagehand::rendering::CustomData>();
                        if (cd) {
                            has_custom_data = true;
                            custom_data_arr.push_back(godot::Vector4(cd->x, cd->y, cd->z, cd->w));
                        }

                        count++;
                    });
                }

                result["count"] = count;
                result["transforms"] = transforms;
                result["has_colors"] = has_colors;
                result["colors"] = colors;
                result["has_custom_data"] = has_custom_data;
                result["custom_data"] = custom_data_arr;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

    // ── Lookup Entities (on-demand) ───────────────────────────────────────
    // Accepts { "names": ["EntityA", "EntityB", ...] } and returns a
    // Dictionary stored in SceneChildrenResult with:
    //   { "found": ["EntityA", ...], "missing": ["EntityB", ...] }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::LOOKUP_ENTITIES)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("names")) {
                    godot::UtilityFunctions::push_warning("Lookup Entities: 'names' parameter required.");
                    return;
                }

                flecs::world world = it.world();
                godot::Array names = (*parameters)["names"];
                godot::Array found;
                godot::Array missing;

                for (int i = 0; i < names.size(); ++i) {
                    godot::String name = names[i];
                    std::string name_str = name.utf8().get_data();
                    flecs::entity entity = world.lookup(name_str.c_str());
                    if (entity.is_valid() && entity.is_alive()) {
                        found.push_back(name);
                    } else {
                        missing.push_back(name);
                    }
                }

                godot::Dictionary result;
                result["found"] = found;
                result["missing"] = missing;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

    // ── Query Instanced Renderers (on-demand) ────────────────────────────
    // Returns information about the instanced renderer state in a Dictionary
    // stored in SceneChildrenResult.
    // Result Dictionary: {
    //   "renderer_count": N,
    //   "renderers": [{ "lod_count": M, "entity_count": K, "instance_rid_count": L }, ...]
    // }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::QUERY_INSTANCED_RENDERERS)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                flecs::world world = it.world();
                const stagehand::rendering::Renderers *renderers = world.try_get<stagehand::rendering::Renderers>();

                godot::Dictionary result;
                if (!renderers) {
                    result["renderer_count"] = 0;
                    result["renderers"] = godot::Array();
                    world.set<SceneChildrenResult>(SceneChildrenResult(result));
                    return;
                }

                godot::Array renderer_array;
                for (const stagehand::rendering::InstancedRendererConfig &renderer : renderers->instanced_renderers) {
                    godot::Dictionary renderer_info;
                    renderer_info["lod_count"] = static_cast<int>(renderer.lod_configs.size());
                    renderer_info["entity_count"] = static_cast<int>(renderer.previous_entity_count);
                    renderer_info["instance_rid_count"] = static_cast<int>(renderer.instance_rids.size());

                    // Check how many instance RIDs are valid
                    int valid_rids = 0;
                    for (const godot::RID &rid : renderer.instance_rids) {
                        if (rid.is_valid()) {
                            valid_rids++;
                        }
                    }
                    renderer_info["valid_instance_rids"] = valid_rids;

                    // Report LOD config details
                    godot::Array lod_details;
                    for (const stagehand::rendering::InstancedRendererLODConfig &lod : renderer.lod_configs) {
                        godot::Dictionary lod_info;
                        lod_info["mesh_rid_valid"] = lod.mesh_rid.is_valid();
                        lod_info["fade_min"] = lod.fade_min;
                        lod_info["fade_max"] = lod.fade_max;
                        lod_info["fade_min_margin"] = lod.fade_min_margin;
                        lod_info["fade_max_margin"] = lod.fade_max_margin;
                        lod_info["visibility_fade_mode"] = static_cast<int>(lod.visibility_fade_mode);
                        lod_details.push_back(lod_info);
                    }
                    renderer_info["lod_details"] = lod_details;

                    renderer_array.push_back(renderer_info);
                }

                result["renderer_count"] = static_cast<int>(renderers->instanced_renderers.size());
                result["renderers"] = renderer_array;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

} // namespace stagehand_tests
