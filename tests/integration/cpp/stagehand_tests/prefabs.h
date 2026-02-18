#pragma once

#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

#include "components.h"
#include "names.h"

namespace stagehand_tests {

    REGISTER([](flecs::world &world) {
        // A 2D test prefab that inherits from the built-in Entity2D prefab.
        // It carries all the transform components (Position2D, Rotation2D, Scale2D, Transform2D)
        // plus a custom TestFloat component for testing.
        world.prefab(stagehand_tests::names::prefabs::TEST_ENTITY_2D).is_a(world.lookup(::stagehand::names::prefabs::ENTITY_2D)).add<EntityValue>();

        // A 3D test prefab that inherits from the built-in Entity3D prefab.
        world.prefab(stagehand_tests::names::prefabs::TEST_ENTITY_3D).is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D)).add<EntityValue>();

        // ── Rendering test prefabs ───────────────────────────────────────────
        // These prefabs are used by MultiMesh rendering integration tests.
        // They inherit from the built-in Entity2D/3D prefabs and add rendering-related components.

        // Basic rendered entity: just transform (no Color or CustomData)
        world.prefab(stagehand_tests::names::prefabs::RENDERED_ENTITY_2D).is_a(world.lookup(::stagehand::names::prefabs::ENTITY_2D)).add<EntityValue>();

        world.prefab(stagehand_tests::names::prefabs::RENDERED_ENTITY_3D).is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D)).add<EntityValue>();

        // Colored entity: transform + Color
        world.prefab(stagehand_tests::names::prefabs::COLORED_ENTITY_2D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_2D))
            .add<EntityValue>()
            .add<Color>();

        world.prefab(stagehand_tests::names::prefabs::COLORED_ENTITY_3D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D))
            .add<EntityValue>()
            .add<Color>();

        // CustomData entity: transform + CustomData
        world.prefab(stagehand_tests::names::prefabs::CUSTOM_DATA_ENTITY_2D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_2D))
            .add<EntityValue>()
            .add<stagehand::rendering::CustomData>();

        world.prefab(stagehand_tests::names::prefabs::CUSTOM_DATA_ENTITY_3D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D))
            .add<EntityValue>()
            .add<stagehand::rendering::CustomData>();

        // Full entity: transform + Color + CustomData
        world.prefab(stagehand_tests::names::prefabs::FULL_ENTITY_2D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_2D))
            .add<EntityValue>()
            .add<Color>()
            .add<stagehand::rendering::CustomData>();

        world.prefab(stagehand_tests::names::prefabs::FULL_ENTITY_3D)
            .is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D))
            .add<EntityValue>()
            .add<Color>()
            .add<stagehand::rendering::CustomData>();

        // ── Instanced rendering test prefab ──────────────────────────────────
        // A 3D entity prefab used by InstancedRenderer3D integration tests.
        world.prefab(stagehand_tests::names::prefabs::INSTANCED_ENTITY_3D).is_a(world.lookup(::stagehand::names::prefabs::ENTITY_3D)).add<EntityValue>();
    });

} // namespace stagehand_tests
