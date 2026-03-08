#pragma once

#include <type_traits>

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand::physics {

    template <typename ServerT> struct PhysicsSystemNames {
        static constexpr const char *BODY_SPACE_ASSIGNMENT = std::is_same_v<ServerT, godot::PhysicsServer2D>
                                                                 ? stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_2D
                                                                 : stagehand::names::systems::PHYSICS_BODY_SPACE_ASSIGNMENT_3D;
        static constexpr const char *FEEDBACK_TRANSFORM = std::is_same_v<ServerT, godot::PhysicsServer2D>
                                                              ? stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_2D
                                                              : stagehand::names::systems::PHYSICS_FEEDBACK_TRANSFORM_3D;
        static constexpr const char *FEEDBACK_VELOCITY = std::is_same_v<ServerT, godot::PhysicsServer2D>
                                                             ? stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_2D
                                                             : stagehand::names::systems::PHYSICS_FEEDBACK_VELOCITY_3D;
        static constexpr const char *FEEDBACK_ANGULAR_VELOCITY = std::is_same_v<ServerT, godot::PhysicsServer2D>
                                                                     ? stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_2D
                                                                     : stagehand::names::systems::PHYSICS_FEEDBACK_ANGULAR_VELOCITY_3D;
        static constexpr const char *SYNC_TRANSFORM = std::is_same_v<ServerT, godot::PhysicsServer2D> ? stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_2D
                                                                                                      : stagehand::names::systems::PHYSICS_SYNC_TRANSFORM_3D;
        static constexpr const char *SYNC_VELOCITY = std::is_same_v<ServerT, godot::PhysicsServer2D> ? stagehand::names::systems::PHYSICS_SYNC_VELOCITY_2D
                                                                                                     : stagehand::names::systems::PHYSICS_SYNC_VELOCITY_3D;
        static constexpr const char *SYNC_ANGULAR_VELOCITY = std::is_same_v<ServerT, godot::PhysicsServer2D>
                                                                 ? stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_2D
                                                                 : stagehand::names::systems::PHYSICS_SYNC_ANGULAR_VELOCITY_3D;
        static constexpr const char *SYNC_COLLISION = std::is_same_v<ServerT, godot::PhysicsServer2D> ? stagehand::names::systems::PHYSICS_SYNC_COLLISION_2D
                                                                                                      : stagehand::names::systems::PHYSICS_SYNC_COLLISION_3D;
        static constexpr const char *BODY_SPACE_CLEANUP =
            std::is_same_v<ServerT, godot::PhysicsServer2D> ? "stagehand::physics::Body Space Cleanup (2D)" : "stagehand::physics::Body Space Cleanup (3D)";
    };

    // --- PhysicsDimensionTraits ------------------------------------------
    //
    // Maps a PhysicsServer type (2D or 3D) to the associated ECS component types and physics server constants.
    // Only the type aliases and static functions that genuinely differ between 2D and 3D are specialised.

    template <typename ServerT> struct PhysicsDimensionTraits {
        static constexpr bool IS_2D = std::is_same_v<ServerT, godot::PhysicsServer2D>;
        static constexpr bool IS_3D = std::is_same_v<ServerT, godot::PhysicsServer3D>;

        static_assert(IS_2D || IS_3D, "PhysicsDimensionTraits only supports godot::PhysicsServer2D and godot::PhysicsServer3D.");

        using Server = ServerT;

        using Position = std::conditional_t<IS_2D, transform::Position2D, transform::Position3D>;
        using Rotation = std::conditional_t<IS_2D, transform::Rotation2D, transform::Rotation3D>;
        using Scale = std::conditional_t<IS_2D, transform::Scale2D, transform::Scale3D>;
        using Transform = std::conditional_t<IS_2D, transform::Transform2D, transform::Transform3D>;
        using HasChangedPosition = std::conditional_t<IS_2D, transform::HasChangedPosition2D, transform::HasChangedPosition3D>;
        using HasChangedRotation = std::conditional_t<IS_2D, transform::HasChangedRotation2D, transform::HasChangedRotation3D>;
        using HasChangedScale = std::conditional_t<IS_2D, transform::HasChangedScale2D, transform::HasChangedScale3D>;
        using Velocity = std::conditional_t<IS_2D, Velocity2D, Velocity3D>;
        using HasChangedVelocity = std::conditional_t<IS_2D, HasChangedVelocity2D, HasChangedVelocity3D>;
        using AngularVelocity = std::conditional_t<IS_2D, AngularVelocity2D, AngularVelocity3D>;
        using HasChangedAngularVelocity = std::conditional_t<IS_2D, HasChangedAngularVelocity2D, HasChangedAngularVelocity3D>;
        using SpaceDimensionTag = std::conditional_t<IS_2D, PhysicsSpace2D, PhysicsSpace3D>;

        using GodotTransform = std::conditional_t<IS_2D, godot::Transform2D, godot::Transform3D>;
        using GodotLinearVelocity = std::conditional_t<IS_2D, godot::Vector2, godot::Vector3>;
        using GodotAngularVelocity = std::conditional_t<IS_2D, float, godot::Vector3>;

        static constexpr auto BODY_STATE_TRANSFORM = Server::BODY_STATE_TRANSFORM;
        static constexpr auto BODY_STATE_LINEAR_VELOCITY = Server::BODY_STATE_LINEAR_VELOCITY;
        static constexpr auto BODY_STATE_ANGULAR_VELOCITY = Server::BODY_STATE_ANGULAR_VELOCITY;

        static GodotTransform compose_transform(const Position &pos, const Rotation &rot, const Scale &scl) {
            if constexpr (IS_2D) {
                godot::Transform2D transform;
                transform.set_origin(pos);
                transform.set_rotation_and_scale(rot, scl);
                return transform;
            } else {
                return godot::Transform3D(godot::Basis(rot).scaled(scl), pos);
            }
        }

        static void decompose_transform(const godot::Variant &var, flecs::entity entity) {
            if constexpr (IS_2D) {
                const godot::Transform2D transform = var;
                entity.set<Position>(Position(transform.get_origin()));
                entity.set<Rotation>(Rotation(transform.get_rotation()));
                entity.set<Scale>(Scale(transform.get_scale()));
                entity.set<Transform>(Transform(transform));
            } else {
                const godot::Transform3D transform = var;
                entity.set<Position>(Position(transform.origin));
                entity.set<Rotation>(Rotation(transform.basis.get_rotation_quaternion()));
                entity.set<Scale>(Scale(transform.basis.get_scale()));
                entity.set<Transform>(Transform(transform));
            }
        }

        static void write_linear_velocity(const godot::Variant &var, flecs::entity entity) {
            const GodotLinearVelocity linear_velocity = static_cast<GodotLinearVelocity>(var);
            entity.set<Velocity>(Velocity(linear_velocity));
        }

        static void write_angular_velocity(const godot::Variant &var, flecs::entity entity) {
            const GodotAngularVelocity angular_velocity = static_cast<GodotAngularVelocity>(var);
            entity.set<AngularVelocity>(AngularVelocity(angular_velocity));
        }

        static godot::Variant angular_velocity_to_variant(const AngularVelocity &angular_velocity) {
            if constexpr (IS_2D) {
                return angular_velocity.value;
            } else {
                return static_cast<const godot::Vector3 &>(angular_velocity);
            }
        }

        static godot::Variant linear_velocity_to_variant(const Velocity &velocity) { return static_cast<const typename Velocity::base_type &>(velocity); }

        static bool matches_body_dimension(PhysicsBodyType body_type) {
            if constexpr (IS_2D) {
                return is_2d_body_type(body_type);
            } else {
                return !is_2d_body_type(body_type);
            }
        }
    };

    template <typename ServerT> void feedback_transform(ServerT *server, const PhysicsBodyRID &rid, flecs::entity entity) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        godot::Variant transform_variant = server->body_get_state(rid, Traits::BODY_STATE_TRANSFORM);
        Traits::decompose_transform(transform_variant, entity);
    }

    template <typename ServerT> void feedback_linear_velocity(ServerT *server, const PhysicsBodyRID &rid, flecs::entity entity) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        godot::Variant velocity_variant = server->body_get_state(rid, Traits::BODY_STATE_LINEAR_VELOCITY);
        Traits::write_linear_velocity(velocity_variant, entity);
    }

    template <typename ServerT> void feedback_angular_velocity(ServerT *server, const PhysicsBodyRID &rid, flecs::entity entity) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        godot::Variant velocity_variant = server->body_get_state(rid, Traits::BODY_STATE_ANGULAR_VELOCITY);
        Traits::write_angular_velocity(velocity_variant, entity);
    }

    template <typename ServerT, typename RequiredComponent, void (*WriteState)(ServerT *, const PhysicsBodyRID &, flecs::entity)>
    void register_feedback_system(flecs::world &world, const char *name) {
        // clang-format off
        world.system<const PhysicsBodyRID, const PhysicsBodyType>(name)
            .kind(stagehand::OnEarlyUpdate)
            .template with<PhysicsBodyInSpace>()
            .template with<RequiredComponent>()
            .run([](flecs::iter &it) {
                // clang-format on
                while (it.next()) {
                    ServerT *server = ServerT::get_singleton();
                    if (server == nullptr) {
                        return;
                    }

                    auto rid_field = it.field<const PhysicsBodyRID>(0);
                    auto type_field = it.field<const PhysicsBodyType>(1);

                    for (auto i : it) {
                        if (!is_dynamic_body_type(type_field[i])) {
                            continue;
                        }
                        if (!rid_field[i].is_valid()) {
                            continue;
                        }

                        WriteState(server, rid_field[i], it.entity(i));
                    }
                }
            });
    }

    template <typename ServerT, typename ValueComponent, typename ChangedTag, auto BodyState, godot::Variant (*ToVariant)(const ValueComponent &)>
    void register_sync_state_system(flecs::world &world, const char *name) {
        world.system<const PhysicsBodyRID, const ValueComponent>(name)
            .kind(stagehand::OnLateUpdate)
            .template with<PhysicsBodyInSpace>()
            .template with<ChangedTag>()
            .each([](const PhysicsBodyRID &rid, const ValueComponent &value) {
                if (!rid.is_valid()) {
                    return;
                }
                ServerT *server = ServerT::get_singleton();
                if (server == nullptr) {
                    return;
                }
                server->body_set_state(rid, BodyState, ToVariant(value));
            });
    }

    template <typename ServerT>
    void write_transform_state(ServerT *server,
                               const PhysicsBodyRID &rid,
                               const typename PhysicsDimensionTraits<ServerT>::Position &position,
                               const typename PhysicsDimensionTraits<ServerT>::Rotation &rotation,
                               const typename PhysicsDimensionTraits<ServerT>::Scale &scale) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        godot::Variant transform = Traits::compose_transform(position, rotation, scale);
        server->body_set_state(rid, Traits::BODY_STATE_TRANSFORM, transform);
    }

    template <typename ServerT> void register_sync_transform_system(flecs::world &world, const char *name) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        using Position = typename Traits::Position;
        using Rotation = typename Traits::Rotation;
        using Scale = typename Traits::Scale;

        // clang-format off
        world.system<const PhysicsBodyRID, const Position, const Rotation, const Scale>(name)
            .kind(stagehand::OnLateUpdate)
            .template with<PhysicsBodyInSpace>()
            .template with<typename Traits::HasChangedPosition>()
                .or_()
            .template with<typename Traits::HasChangedRotation>()
                .or_()
            .template with<typename Traits::HasChangedScale>()
            .each([](
                const PhysicsBodyRID &rid,
                const Position &position,
                const Rotation &rotation,
                const Scale &scale) {
                // clang-format on
                if (!rid.is_valid()) {
                    return;
                }
                ServerT *server = ServerT::get_singleton();
                if (server == nullptr) {
                    return;
                }
                write_transform_state<ServerT>(server, rid, position, rotation, scale);
            });
    }

    template <typename ServerT> void register_sync_collision_system(flecs::world &world, const char *name) {
        using Traits = PhysicsDimensionTraits<ServerT>;

        // clang-format off
        world.system<const PhysicsBodyRID, const PhysicsBodyType, const CollisionLayer, const CollisionMask>(name)
            .kind(stagehand::OnLateUpdate)
            .template with<PhysicsBodyInSpace>()
            .template with<HasChangedCollisionLayer>()
                .or_()
            .template with<HasChangedCollisionMask>()
            .each([](
                const PhysicsBodyRID &rid,
                const PhysicsBodyType &body_type,
                const CollisionLayer &layer,
                const CollisionMask &mask) {
                // clang-format on
                if (!Traits::matches_body_dimension(body_type)) {
                    return;
                }
                if (!rid.is_valid()) {
                    return;
                }
                ServerT *server = ServerT::get_singleton();
                if (server == nullptr) {
                    return;
                }
                server->body_set_collision_layer(rid, layer);
                server->body_set_collision_mask(rid, mask);
            });
    }

    template <typename ServerT, typename SpaceTag> void free_owned_physics_space(flecs::world world) {
        const PhysicsSpaceRID *space_rid = world.try_get<PhysicsSpaceRID, SpaceTag>();
        if (space_rid == nullptr || !space_rid->is_valid()) {
            return;
        }
        if (!world.has<OwnedPhysicsSpace, SpaceTag>()) {
            return;
        }

        ServerT *server = ServerT::get_singleton();
        if (server != nullptr) {
            server->free_rid(*space_rid);
        }
    }

    // --- Space Management ------------------------------------------------
    //
    // Physics spaces are stored as pair singletons on the world entity:   (PhysicsSpaceRID, PhysicsSpace2D) and (PhysicsSpaceRID, PhysicsSpace3D)
    // This uses a single PhysicsSpaceRID component type, differentiated by the dimension tag in the pair relationship.
    // Ownership is tracked by the (OwnedPhysicsSpace, SpaceTag) pair.

    /// Ensures a physics space exists for the given dimension and returns its RID.  Returns an invalid RID on failure.
    // The RID is returned directly so it is available even when the set is deferred (inside a system callback).
    template <typename ServerT> godot::RID ensure_physics_space(flecs::world world) {
        using SpaceTag = typename PhysicsDimensionTraits<ServerT>::SpaceDimensionTag;

        const PhysicsSpaceRID *existing = world.try_get<PhysicsSpaceRID, SpaceTag>();
        if (existing != nullptr && existing->is_valid()) {
            return static_cast<const godot::RID &>(*existing);
        }

        if (godot::gdextension_interface::library == nullptr) {
            return godot::RID();
        }

        ServerT *server = ServerT::get_singleton();
        if (server == nullptr) {
            return godot::RID();
        }

        godot::RID space_rid = server->space_create();
        server->space_set_active(space_rid, true);

        world.set<PhysicsSpaceRID, SpaceTag>(PhysicsSpaceRID(space_rid));
        world.add<OwnedPhysicsSpace, SpaceTag>();
        return space_rid;
    }

    // --- Templated System Registration -----------------------------------

    template <typename ServerT> void register_physics_systems(flecs::world &world) {
        using Traits = PhysicsDimensionTraits<ServerT>;
        using Position = typename Traits::Position;
        using Rotation = typename Traits::Rotation;
        using Scale = typename Traits::Scale;
        using Velocity = typename Traits::Velocity;
        using AngularVelocity = typename Traits::AngularVelocity;

        // -- Body Space Assignment ----------------------------------------
        {
            const char *name = PhysicsSystemNames<ServerT>::BODY_SPACE_ASSIGNMENT;

            // clang-format off
            world.system<
                    const PhysicsBodyRID,
                    const PhysicsBodyType,
                    const Position,
                    const Rotation,
                    const Scale
                >(name)
                .kind(stagehand::OnLateUpdate)
                .template without<PhysicsBodyInSpace>()
                .each([](
                    flecs::entity e,
                    const PhysicsBodyRID &rid,
                    const PhysicsBodyType &body_type,
                    const Position &pos,
                    const Rotation &rot,
                    const Scale &scl
                ){
                    // clang-format on
                    if (!Traits::matches_body_dimension(body_type)) {
                        return;
                    }
                    if (!rid.is_valid()) {
                        return;
                    }

                    godot::RID space_rid = ensure_physics_space<ServerT>(e.world());
                    if (!space_rid.is_valid()) {
                        return;
                    }

                    ServerT *server = ServerT::get_singleton();
                    if (server == nullptr) {
                        return;
                    }

                    server->body_set_space(rid, space_rid);

                    const CollisionLayer *layer = e.try_get<CollisionLayer>();
                    const CollisionMask *mask = e.try_get<CollisionMask>();
                    if (layer != nullptr) {
                        server->body_set_collision_layer(rid, layer->value);
                    }
                    if (mask != nullptr) {
                        server->body_set_collision_mask(rid, mask->value);
                    }

                    auto transform = Traits::compose_transform(pos, rot, scl);
                    server->body_set_state(rid, Traits::BODY_STATE_TRANSFORM, transform);

                    e.add<PhysicsBodyInSpace>();
                });
        }

        // -- Physics Feedback (Transform) ---------------------------------
        {
            register_feedback_system<ServerT, Position, &feedback_transform<ServerT>>(world, PhysicsSystemNames<ServerT>::FEEDBACK_TRANSFORM);
        }

        // -- Physics Feedback (Linear Velocity) ---------------------------
        {
            register_feedback_system<ServerT, Velocity, &feedback_linear_velocity<ServerT>>(world, PhysicsSystemNames<ServerT>::FEEDBACK_VELOCITY);
        }

        // -- Physics Feedback (Angular Velocity) --------------------------
        {
            register_feedback_system<ServerT, AngularVelocity, &feedback_angular_velocity<ServerT>>(world,
                                                                                                    PhysicsSystemNames<ServerT>::FEEDBACK_ANGULAR_VELOCITY);
        }

        // -- Transform to Physics Sync ------------------------------------
        {
            register_sync_transform_system<ServerT>(world, PhysicsSystemNames<ServerT>::SYNC_TRANSFORM);
        }

        // -- Linear Velocity to Physics Sync ------------------------------
        {
            register_sync_state_system<ServerT, Velocity, typename Traits::HasChangedVelocity, Traits::BODY_STATE_LINEAR_VELOCITY,
                                       &Traits::linear_velocity_to_variant>(world, PhysicsSystemNames<ServerT>::SYNC_VELOCITY);
        }

        // -- Angular Velocity to Physics Sync -----------------------------
        {
            register_sync_state_system<ServerT, AngularVelocity, typename Traits::HasChangedAngularVelocity, Traits::BODY_STATE_ANGULAR_VELOCITY,
                                       &Traits::angular_velocity_to_variant>(world, PhysicsSystemNames<ServerT>::SYNC_ANGULAR_VELOCITY);
        }

        // -- Collision Layer / Mask to Physics Sync -----------------------
        {
            register_sync_collision_system<ServerT>(world, PhysicsSystemNames<ServerT>::SYNC_COLLISION);
        }

        // -- Cleanup Observer ---------------------------------------------
        {
            const char *name = PhysicsSystemNames<ServerT>::BODY_SPACE_CLEANUP;

            // clang-format off
            world.observer<const PhysicsBodyRID, const PhysicsBodyType>(name)
                .event(flecs::OnRemove)
                .template with<PhysicsBodyInSpace>()
                .each([](const PhysicsBodyRID &rid, const PhysicsBodyType &body_type) {
                    // clang-format on
                    if (!Traits::matches_body_dimension(body_type)) {
                        return;
                    }
                    if (!rid.is_valid()) {
                        return;
                    }
                    if (godot::gdextension_interface::library == nullptr) {
                        return;
                    }
                    ServerT *server = ServerT::get_singleton();
                    if (server == nullptr) {
                        return;
                    }
                    server->body_set_space(rid, godot::RID());
                });
        }
    }

    // --- Registration ----------------------------------------------------

    REGISTER([](flecs::world &world) {
        register_physics_systems<godot::PhysicsServer2D>(world);
        register_physics_systems<godot::PhysicsServer3D>(world);

        ecs_atfini(
            world.c_ptr(),
            [](ecs_world_t *raw, void *) {
                flecs::world w(raw);
                // Free physics bodies.
                w.each([](flecs::entity, const PhysicsBodyType &bt, const PhysicsBodyRID &rid) { free_physics_body(bt, rid); });
                // Free owned physics spaces stored as pair singletons.
                if (godot::gdextension_interface::library != nullptr) {
                    free_owned_physics_space<godot::PhysicsServer2D, PhysicsSpace2D>(w);
                    free_owned_physics_space<godot::PhysicsServer3D, PhysicsSpace3D>(w);
                }
            },
            nullptr);
    });

} // namespace stagehand::physics
