#pragma once

#include "stagehand/registry.h"
#include "stagehand/names.h"
#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"

using namespace stagehand;

inline stagehand::Registry register_entity_prefabs([](flecs::world& world) {
    world.prefab(names::prefabs::ENTITY_2D)
        .add<Position2D>()
        .add<Rotation2D>()
        .add<Scale2D>()
        .add<Transform2D>()
        ;

    world.prefab(names::prefabs::ENTITY_3D)
        .add<Position3D>()
        .add<Rotation3D>()
        .add<Scale3D>()
        .add<Transform3D>()
        ;
});
