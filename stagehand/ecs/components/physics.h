#pragma once

#include <array>
#include <cstdint>

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"

namespace stagehand::physics {

    // ─── Physics Body Type Enum ──────────────────────────────────────────

    enum class PhysicsBodyType : uint8_t {
        Static2D = 0,
        Kinematic2D = 1,
        Rigid2D = 2,
        RigidLinear2D = 3,
        Static3D = 4,
        Kinematic3D = 5,
        Rigid3D = 6,
        RigidLinear3D = 7,
    };

    static constexpr uint8_t PHYSICS_BODY_TYPE_COUNT = 8;

    // ─── Compile-time body type property tables ──────────────────────────

    static constexpr std::array<bool, PHYSICS_BODY_TYPE_COUNT> body_type_is_2d = {
        true, true, true, true, false, false, false, false,
    };

    static constexpr std::array<bool, PHYSICS_BODY_TYPE_COUNT> body_type_is_dynamic = {
        false, false, true, true, false, false, true, true,
    };

    constexpr bool is_2d_body_type(PhysicsBodyType type) {
        return body_type_is_2d[static_cast<uint8_t>(type)];
    }

    constexpr bool is_dynamic_body_type(PhysicsBodyType type) {
        return body_type_is_dynamic[static_cast<uint8_t>(type)];
    }

    // ─── Components ──────────────────────────────────────────────────────

    GODOT_VARIANT(PhysicsBodyRID, godot::RID);

    /// Physics space RID.  Used as a singleton with dimension tags
    /// (PhysicsSpace2D / PhysicsSpace3D) applied to the singleton entity.
    GODOT_VARIANT(PhysicsSpaceRID, godot::RID);

    GODOT_VARIANT(Velocity2D, godot::Vector2);
    FLOAT(AngularVelocity2D);
    GODOT_VARIANT(Velocity3D, godot::Vector3);
    GODOT_VARIANT(AngularVelocity3D, godot::Vector3);

    UINT32(CollisionLayer, 1);
    UINT32(CollisionMask, 1);

    /// Tag indicating that a physics body has been assigned to a physics space.
    TAG(PhysicsBodyInSpace);

    /// Tag applied to PhysicsSpaceRID singleton entities that the framework
    /// created and therefore must free on teardown.
    TAG(OwnedPhysicsSpace);

    /// Dimension tags applied to PhysicsSpaceRID singleton entities to
    /// distinguish 2D from 3D spaces.  Using tags lets the Flecs query
    /// engine filter by dimension without runtime branching.
    TAG(PhysicsSpace2D);
    TAG(PhysicsSpace3D);

    // ─── PhysicsServer Traits ────────────────────────────────────────────

    template <typename ServerType>
    struct PhysicsServerTraits {
        using BodyMode = typename ServerType::BodyMode;

        static ServerType *get_singleton() { return ServerType::get_singleton(); }

        static godot::RID create_body(ServerType *server) { return server->body_create(); }

        static void set_body_mode(ServerType *server, const godot::RID &rid, BodyMode mode) { server->body_set_mode(rid, mode); }

        static void free_rid(ServerType *server, const godot::RID &rid) { server->free_rid(rid); }
    };

    template <typename ServerType> godot::RID create_physics_body_rid(typename PhysicsServerTraits<ServerType>::BodyMode body_mode, const char *server_name) {
        using Traits = PhysicsServerTraits<ServerType>;

        if (godot::gdextension_interface::library == nullptr) {
            return godot::RID();
        }

        ServerType *server = Traits::get_singleton();
        if (server == nullptr) {
            godot::UtilityFunctions::push_warning(godot::String("PhysicsBodyType hook could not access ") + server_name + ". The body was not created.");
            return godot::RID();
        }

        const godot::RID rid = Traits::create_body(server);
        Traits::set_body_mode(server, rid, body_mode);
        return rid;
    }

    template <typename ServerType> void free_physics_body_rid(const godot::RID &rid) {
        using Traits = PhysicsServerTraits<ServerType>;

        if (godot::gdextension_interface::library == nullptr) {
            return;
        }

        ServerType *server = Traits::get_singleton();
        if (server == nullptr) {
            return;
        }
        Traits::free_rid(server, rid);
    }

    namespace internal {
        template <typename Server, typename PhysicsServerTraits<Server>::BodyMode Mode> godot::RID create_body_thunk() {
            if constexpr (std::is_same_v<Server, godot::PhysicsServer2D>) {
                return create_physics_body_rid<Server>(Mode, "PhysicsServer2D");
            } else {
                return create_physics_body_rid<Server>(Mode, "PhysicsServer3D");
            }
        }

        template <typename Server> void free_body_thunk(const godot::RID &rid) { free_physics_body_rid<Server>(rid); }

        struct PhysicsBodyHandlers {
            godot::RID (*create)();
            void (*free)(const godot::RID &);
        };

        inline const PhysicsBodyHandlers &get_physics_body_handlers(PhysicsBodyType type) {
            static constexpr PhysicsBodyHandlers handlers[] = {
                {create_body_thunk<godot::PhysicsServer2D, godot::PhysicsServer2D::BODY_MODE_STATIC>, free_body_thunk<godot::PhysicsServer2D>},
                {create_body_thunk<godot::PhysicsServer2D, godot::PhysicsServer2D::BODY_MODE_KINEMATIC>, free_body_thunk<godot::PhysicsServer2D>},
                {create_body_thunk<godot::PhysicsServer2D, godot::PhysicsServer2D::BODY_MODE_RIGID>, free_body_thunk<godot::PhysicsServer2D>},
                {create_body_thunk<godot::PhysicsServer2D, godot::PhysicsServer2D::BODY_MODE_RIGID_LINEAR>, free_body_thunk<godot::PhysicsServer2D>},
                {create_body_thunk<godot::PhysicsServer3D, godot::PhysicsServer3D::BODY_MODE_STATIC>, free_body_thunk<godot::PhysicsServer3D>},
                {create_body_thunk<godot::PhysicsServer3D, godot::PhysicsServer3D::BODY_MODE_KINEMATIC>, free_body_thunk<godot::PhysicsServer3D>},
                {create_body_thunk<godot::PhysicsServer3D, godot::PhysicsServer3D::BODY_MODE_RIGID>, free_body_thunk<godot::PhysicsServer3D>},
                {create_body_thunk<godot::PhysicsServer3D, godot::PhysicsServer3D::BODY_MODE_RIGID_LINEAR>, free_body_thunk<godot::PhysicsServer3D>},
            };

            size_t index = static_cast<size_t>(type);
            if (index < sizeof(handlers) / sizeof(handlers[0])) {
                return handlers[index];
            }

            static constexpr PhysicsBodyHandlers empty = {[]() { return godot::RID(); }, [](const godot::RID &) {}};
            return empty;
        }
    } // namespace internal

    inline godot::RID create_physics_body(PhysicsBodyType body_type) { return internal::get_physics_body_handlers(body_type).create(); }

    inline void free_physics_body(PhysicsBodyType body_type, const godot::RID &rid) {
        if (!rid.is_valid()) {
            return;
        }
        internal::get_physics_body_handlers(body_type).free(rid);
    }

    ENUM(PhysicsBodyType)
        .then([](auto c) {
            c.constant("Static2D", PhysicsBodyType::Static2D)
                .constant("Kinematic2D", PhysicsBodyType::Kinematic2D)
                .constant("Rigid2D", PhysicsBodyType::Rigid2D)
                .constant("RigidLinear2D", PhysicsBodyType::RigidLinear2D)
                .constant("Static3D", PhysicsBodyType::Static3D)
                .constant("Kinematic3D", PhysicsBodyType::Kinematic3D)
                .constant("Rigid3D", PhysicsBodyType::Rigid3D)
                .constant("RigidLinear3D", PhysicsBodyType::RigidLinear3D);
        })
        .then([](auto c) {
            c.on_add([](flecs::entity entity, PhysicsBodyType &physics_body_type) {
                // Skip body creation for prefab templates – they are
                // blueprints and should not own PhysicsServer resources.
                if (entity.has(flecs::Prefab)) {
                    return;
                }
                const godot::RID body_rid = create_physics_body(physics_body_type);
                if (body_rid.is_valid()) {
                    entity.set<PhysicsBodyRID>(body_rid);
                }
            });

            c.on_remove([](flecs::entity entity, PhysicsBodyType &physics_body_type) {
                // During world destruction, component registrations may already
                // be deleted so accessing PhysicsBodyRID would assert.
                // The ecs_atfini callback in systems/physics.h handles cleanup.
                if (ecs_is_fini(entity.world().c_ptr())) {
                    return;
                }

                // Prefab templates never created a body, so nothing to free.
                if (entity.has(flecs::Prefab)) {
                    return;
                }

                const PhysicsBodyRID *body_rid_component = entity.try_get<PhysicsBodyRID>();
                if (body_rid_component == nullptr) {
                    return;
                }

                free_physics_body(physics_body_type, *body_rid_component);
                entity.remove<PhysicsBodyRID>();
            });
        });

} // namespace stagehand::physics
