#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

using namespace stagehand;

inline stagehand::Registry register_entity_prefabs([](flecs::world &world) {
    world.prefab(names::prefabs::ENTITY_2D)
        .add<stagehand::transform::Position2D>()
        .add<stagehand::transform::Rotation2D>()
        .add<stagehand::transform::Scale2D>()
        .add<Transform2D>();

    world.prefab(names::prefabs::ENTITY_3D)
        .add<stagehand::transform::Position3D>()
        .add<stagehand::transform::Rotation3D>()
        .add<stagehand::transform::Scale3D>()
        .add<Transform3D>();
});
