#pragma once

#include "stagehand/ecs/components/transform.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand {
    REGISTER([](flecs::world &world) {
        world.prefab(names::prefabs::ENTITY_2D)
            .add<transform::Position2D>()
            .add<transform::Rotation2D>()
            .add<transform::Scale2D>()
            .add<transform::Transform2D>();

        world.prefab(names::prefabs::ENTITY_3D)
            .add<transform::Position3D>()
            .add<transform::Rotation3D>()
            .add<transform::Scale3D>()
            .add<transform::Transform3D>();
    });
} // namespace stagehand
