#pragma once

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/scene_children.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/entity.h"
#include "stagehand/utilities/emit_signal.h"

#include "components.h"
#include "names.h"

namespace stagehand_tests {

    // Register a no-op module callback so FlecsWorld module import checks succeed
    // for the module-loading integration tests.
    REGISTER_IN_MODULE(foo::bar, [](flecs::world &) {});

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
        // via run_system(). It constructs EventPayload from parameters.
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
                stagehand::EventPayload payload;
                payload.name = signal_name;
                payload.data = signal_data;
                stagehand::emit_signal(signal_source, payload);
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
                        lod_info["visibility_range_begin"] = lod.visibility_range_begin;
                        lod_info["visibility_range_end"] = lod.visibility_range_end;
                        lod_info["visibility_range_begin_margin"] = lod.visibility_range_begin_margin;
                        lod_info["visibility_range_end_margin"] = lod.visibility_range_end_margin;
                        lod_info["visibility_range_fade_mode"] = static_cast<int>(lod.visibility_range_fade_mode);
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

    // ── Query Physics Bodies (on-demand) ─────────────────────────────────
    // Queries entities with physics body components and returns state info.
    // Parameters: { "prefab": "prefab_name" }
    // Result Dictionary stored in SceneChildrenResult:
    //   { "count": N, "bodies": [{ "rid_valid": bool, "in_space": bool,
    //     "body_type": int, "has_collision_layer": bool, "has_velocity": bool }] }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::QUERY_PHYSICS_BODIES)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("prefab")) {
                    godot::UtilityFunctions::push_warning("Query Physics Bodies: 'prefab' parameter required.");
                    return;
                }

                godot::String prefab_name = (*parameters)["prefab"];
                std::string prefab_str = prefab_name.utf8().get_data();

                flecs::world world = it.world();
                flecs::entity prefab = world.lookup(prefab_str.c_str());
                if (!prefab.is_valid()) {
                    godot::UtilityFunctions::push_warning("Query Physics Bodies: prefab not found: " + prefab_name);
                    world.set<SceneChildrenResult>(SceneChildrenResult(godot::Dictionary()));
                    return;
                }

                godot::Dictionary result;
                godot::Array bodies;
                int count = 0;

                auto query =
                    world.query_builder<const stagehand::physics::PhysicsBodyRID, const stagehand::physics::PhysicsBodyType>().with(flecs::IsA, prefab).build();

                query.each([&](flecs::entity e, const stagehand::physics::PhysicsBodyRID &rid, const stagehand::physics::PhysicsBodyType &body_type) {
                    godot::Dictionary body_info;
                    body_info["rid_valid"] = rid.is_valid();
                    body_info["in_space"] = e.has<stagehand::physics::PhysicsBodyInSpace>();
                    body_info["body_type"] = static_cast<int>(body_type);

                    const stagehand::physics::CollisionLayer *layer = e.try_get<stagehand::physics::CollisionLayer>();
                    body_info["has_collision_layer"] = (layer != nullptr);
                    body_info["collision_layer"] = layer ? static_cast<int>(layer->value) : 0;

                    const stagehand::physics::CollisionMask *mask = e.try_get<stagehand::physics::CollisionMask>();
                    body_info["has_collision_mask"] = (mask != nullptr);
                    body_info["collision_mask"] = mask ? static_cast<int>(mask->value) : 0;

                    const stagehand::physics::Velocity3D *vel3d = e.try_get<stagehand::physics::Velocity3D>();
                    body_info["has_velocity_3d"] = (vel3d != nullptr);
                    if (vel3d) {
                        body_info["velocity_3d"] = godot::Vector3(vel3d->x, vel3d->y, vel3d->z);
                    }

                    const stagehand::physics::Velocity2D *vel2d = e.try_get<stagehand::physics::Velocity2D>();
                    body_info["has_velocity_2d"] = (vel2d != nullptr);
                    if (vel2d) {
                        body_info["velocity_2d"] = godot::Vector2(vel2d->x, vel2d->y);
                    }

                    // Read position
                    const stagehand::transform::Position3D *pos3d = e.try_get<stagehand::transform::Position3D>();
                    if (pos3d) {
                        body_info["position_3d"] = godot::Vector3(pos3d->x, pos3d->y, pos3d->z);
                    }
                    const stagehand::transform::Position2D *pos2d = e.try_get<stagehand::transform::Position2D>();
                    if (pos2d) {
                        body_info["position_2d"] = godot::Vector2(pos2d->x, pos2d->y);
                    }

                    body_info["entity_id"] = static_cast<int64_t>(e.id());
                    bodies.push_back(body_info);
                    count++;
                });

                result["count"] = count;
                result["bodies"] = bodies;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

    // ── Query Physics Body State (on-demand) ─────────────────────────────
    // Queries the PhysicsServer directly for a body's state.
    // Parameters: { "prefab": "prefab_name", "dimension": "2d" or "3d" }
    // Result Dictionary stored in SceneChildrenResult:
    //   { "count": N, "states": [{ "transform": ..., "linear_velocity": ..., "angular_velocity": ... }] }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::QUERY_PHYSICS_BODY_STATE)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("prefab")) {
                    godot::UtilityFunctions::push_warning("Query Physics Body State: 'prefab' parameter required.");
                    return;
                }

                godot::String prefab_name = (*parameters)["prefab"];
                godot::String dimension = parameters->get("dimension", "3d");
                std::string prefab_str = prefab_name.utf8().get_data();

                flecs::world world = it.world();
                flecs::entity prefab = world.lookup(prefab_str.c_str());
                if (!prefab.is_valid()) {
                    godot::UtilityFunctions::push_warning("Query Physics Body State: prefab not found: " + prefab_name);
                    world.set<SceneChildrenResult>(SceneChildrenResult(godot::Dictionary()));
                    return;
                }

                godot::Dictionary result;
                godot::Array states;
                int count = 0;

                auto query = world.query_builder<const stagehand::physics::PhysicsBodyRID>()
                                 .with(flecs::IsA, prefab)
                                 .with<stagehand::physics::PhysicsBodyInSpace>()
                                 .build();

                query.each([&](flecs::entity e, const stagehand::physics::PhysicsBodyRID &rid) {
                    godot::Dictionary state;
                    if (!rid.is_valid()) {
                        states.push_back(state);
                        count++;
                        return;
                    }

                    if (dimension == "3d") {
                        godot::PhysicsServer3D *server = godot::PhysicsServer3D::get_singleton();
                        if (server) {
                            godot::Transform3D t = server->body_get_state(rid, godot::PhysicsServer3D::BODY_STATE_TRANSFORM);
                            state["origin"] = t.origin;
                            godot::Vector3 vel = server->body_get_state(rid, godot::PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY);
                            state["linear_velocity"] = vel;
                            godot::Vector3 avel = server->body_get_state(rid, godot::PhysicsServer3D::BODY_STATE_ANGULAR_VELOCITY);
                            state["angular_velocity"] = avel;
                        }
                    } else {
                        godot::PhysicsServer2D *server = godot::PhysicsServer2D::get_singleton();
                        if (server) {
                            godot::Transform2D t = server->body_get_state(rid, godot::PhysicsServer2D::BODY_STATE_TRANSFORM);
                            state["origin"] = t.get_origin();
                            godot::Vector2 vel = server->body_get_state(rid, godot::PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY);
                            state["linear_velocity"] = vel;
                            float avel = server->body_get_state(rid, godot::PhysicsServer2D::BODY_STATE_ANGULAR_VELOCITY);
                            state["angular_velocity"] = avel;
                        }
                    }

                    states.push_back(state);
                    count++;
                });

                result["count"] = count;
                result["states"] = states;
                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

    // ── Query Physics Spaces (on-demand) ─────────────────────────────────
    // Returns info about the physics space pair singletons.
    // Spaces are stored as (PhysicsSpaceRID, DimensionTag) pairs on the
    // component entity, with ownership tracked via (OwnedPhysicsSpace, DimensionTag).
    // Result Dictionary stored in SceneChildrenResult:
    //   { "has_space_2d": bool, "space_2d_valid": bool, "space_2d_owned": bool,
    //     "has_space_3d": bool, "space_3d_valid": bool, "space_3d_owned": bool }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::QUERY_PHYSICS_SPACES)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                flecs::world world = it.world();
                godot::Dictionary result;

                const auto *space2d_rid = world.try_get<stagehand::physics::PhysicsSpaceRID, stagehand::physics::PhysicsSpace2D>();
                result["has_space_2d"] = (space2d_rid != nullptr);
                result["space_2d_valid"] = space2d_rid ? space2d_rid->is_valid() : false;
                result["space_2d_owned"] = world.has<stagehand::physics::OwnedPhysicsSpace, stagehand::physics::PhysicsSpace2D>();

                const auto *space3d_rid = world.try_get<stagehand::physics::PhysicsSpaceRID, stagehand::physics::PhysicsSpace3D>();
                result["has_space_3d"] = (space3d_rid != nullptr);
                result["space_3d_valid"] = space3d_rid ? space3d_rid->is_valid() : false;
                result["space_3d_owned"] = world.has<stagehand::physics::OwnedPhysicsSpace, stagehand::physics::PhysicsSpace3D>();

                world.set<SceneChildrenResult>(SceneChildrenResult(result));
            });
    });

    // ── Update Entity Physics (on-demand) ──────────────────────────────
    // Sets transform/velocity/collision components on a specific entity
    // using stagehand::entity, which triggers HasChanged tags for change
    // detection.  This enables testing the sync pipeline end-to-end.
    // Parameters: { "entity_id": int,
    //   "position_3d": Vector3,  (optional)
    //   "position_2d": Vector2,  (optional)
    //   "rotation_3d": Quaternion, (optional)
    //   "rotation_2d": float,   (optional)
    //   "scale_3d": Vector3,    (optional)
    //   "scale_2d": Vector2,    (optional)
    //   "velocity_3d": Vector3, (optional)
    //   "velocity_2d": Vector2, (optional)
    //   "angular_velocity_3d": Vector3, (optional)
    //   "angular_velocity_2d": float, (optional)
    //   "collision_layer": int, (optional)
    //   "collision_mask": int   (optional)
    // }
    REGISTER([](flecs::world &world) {
        world.system(names::systems::UPDATE_ENTITY_PHYSICS)
            .kind(0) // on-demand
            .run([](flecs::iter &it) {
                const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
                if (!parameters || !parameters->has("entity_id")) {
                    godot::UtilityFunctions::push_warning("Update Entity Physics: 'entity_id' parameter required.");
                    return;
                }

                flecs::world world = it.world();
                int64_t entity_id = (*parameters)["entity_id"];
                if (!world.is_alive(static_cast<ecs_entity_t>(entity_id))) {
                    godot::UtilityFunctions::push_warning("Update Entity Physics: entity is not alive.");
                    return;
                }

                // Use stagehand::entity to trigger HasChanged tags
                stagehand::entity e(flecs::entity(world, static_cast<ecs_entity_t>(entity_id)));

                if (parameters->has("position_3d")) {
                    godot::Vector3 pos = (*parameters)["position_3d"];
                    e.set<stagehand::transform::Position3D>(stagehand::transform::Position3D(pos));
                }
                if (parameters->has("position_2d")) {
                    godot::Vector2 pos = (*parameters)["position_2d"];
                    e.set<stagehand::transform::Position2D>(stagehand::transform::Position2D(pos));
                }
                if (parameters->has("rotation_3d")) {
                    godot::Quaternion rot = (*parameters)["rotation_3d"];
                    e.set<stagehand::transform::Rotation3D>(stagehand::transform::Rotation3D(rot));
                }
                if (parameters->has("rotation_2d")) {
                    float rot = (*parameters)["rotation_2d"];
                    e.set<stagehand::transform::Rotation2D>(stagehand::transform::Rotation2D(rot));
                }
                if (parameters->has("scale_3d")) {
                    godot::Vector3 scl = (*parameters)["scale_3d"];
                    e.set<stagehand::transform::Scale3D>(stagehand::transform::Scale3D(scl));
                }
                if (parameters->has("scale_2d")) {
                    godot::Vector2 scl = (*parameters)["scale_2d"];
                    e.set<stagehand::transform::Scale2D>(stagehand::transform::Scale2D(scl));
                }
                if (parameters->has("velocity_3d")) {
                    godot::Vector3 vel = (*parameters)["velocity_3d"];
                    e.set<stagehand::physics::Velocity3D>(stagehand::physics::Velocity3D(vel));
                }
                if (parameters->has("velocity_2d")) {
                    godot::Vector2 vel = (*parameters)["velocity_2d"];
                    e.set<stagehand::physics::Velocity2D>(stagehand::physics::Velocity2D(vel));
                }
                if (parameters->has("angular_velocity_3d")) {
                    godot::Vector3 avel = (*parameters)["angular_velocity_3d"];
                    e.set<stagehand::physics::AngularVelocity3D>(stagehand::physics::AngularVelocity3D(avel));
                }
                if (parameters->has("angular_velocity_2d")) {
                    float avel = (*parameters)["angular_velocity_2d"];
                    e.set<stagehand::physics::AngularVelocity2D>(stagehand::physics::AngularVelocity2D(avel));
                }
                if (parameters->has("collision_layer")) {
                    uint32_t layer = static_cast<uint32_t>(static_cast<int>((*parameters)["collision_layer"]));
                    e.set<stagehand::physics::CollisionLayer>(stagehand::physics::CollisionLayer(layer));
                }
                if (parameters->has("collision_mask")) {
                    uint32_t mask = static_cast<uint32_t>(static_cast<int>((*parameters)["collision_mask"]));
                    e.set<stagehand::physics::CollisionMask>(stagehand::physics::CollisionMask(mask));
                }
            });

        // ── Event Emission Test Observers ────────────────────────────────────

        // Initialize event test singletons
        world.set<EventReceivedCount>({0});
        world.set<TestEventACount>({0});
        world.set<TestEventBCount>({0});
        world.set<LastEventData>({godot::Dictionary()});
        world.set<TestEventAData>({godot::Dictionary()});
        world.set<TestEventBData>({godot::Dictionary()});

        // Universal event observer - listens to all events emitted via emit_event
        world.observer(names::systems::UNIVERSAL_EVENT_OBSERVER).event<stagehand::EventPayload>().with(flecs::Any).each([](flecs::iter &it, size_t index) {
            flecs::world world = it.world();
            (void)index;

            // Get the Signal payload from the parameter
            const stagehand::EventPayload *signal_payload = it.param<stagehand::EventPayload>();
            if (!signal_payload) {
                return;
            }

            // Update counters and store data
            EventReceivedCount &counter = world.ensure<EventReceivedCount>();
            counter.value++;

            // Store the event data
            godot::Dictionary event_info;
            event_info["name"] = signal_payload->name;
            event_info["data"] = signal_payload->data;
            event_info["count"] = counter.value;

            event_info["source_entity_id"] = signal_payload->source_entity_id;

            world.set<LastEventData>({event_info});
        });

        // Observer for TestEventA - filters by payload name
        world.observer(names::systems::TEST_EVENT_A_OBSERVER).with(flecs::Any).event<stagehand::EventPayload>().each([](flecs::iter &it, size_t index) {
            flecs::world world = it.world();
            (void)index;

            const stagehand::EventPayload *signal_payload = it.param<stagehand::EventPayload>();
            if (!signal_payload) {
                return;
            }

            if (signal_payload->name != godot::StringName("TestEventA")) {
                return;
            }

            TestEventACount &counter = world.ensure<TestEventACount>();
            counter.value++;

            godot::Dictionary event_data;
            event_data["name"] = signal_payload->name;
            event_data["data"] = signal_payload->data;
            event_data["count"] = counter.value;

            event_data["source_entity_id"] = signal_payload->source_entity_id;

            world.set<TestEventAData>({event_data});
        });

        // Observer for TestEventB - filters by payload name
        world.observer(names::systems::TEST_EVENT_B_OBSERVER).with(flecs::Any).event<stagehand::EventPayload>().each([](flecs::iter &it, size_t index) {
            flecs::world world = it.world();
            (void)index;

            const stagehand::EventPayload *signal_payload = it.param<stagehand::EventPayload>();
            if (!signal_payload) {
                return;
            }

            if (signal_payload->name != godot::StringName("TestEventB")) {
                return;
            }

            TestEventBCount &counter = world.ensure<TestEventBCount>();
            counter.value++;

            godot::Dictionary event_data;
            event_data["name"] = signal_payload->name;
            event_data["data"] = signal_payload->data;
            event_data["count"] = counter.value;

            event_data["source_entity_id"] = signal_payload->source_entity_id;

            world.set<TestEventBData>({event_data});
        });
    });

} // namespace stagehand_tests
