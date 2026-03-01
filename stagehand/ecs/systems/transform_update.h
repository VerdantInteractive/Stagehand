#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/entity.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand::transform {
    REGISTER([](flecs::world &world) {
        // clang-format off
        world.system<
                Transform2D,
                const Position2D,
                const Rotation2D,
                const Scale2D
            >(stagehand::names::systems::TRANSFORM_UPDATE_2D)
            .kind(stagehand::PreRender)
            .with<const HasChangedPosition2D>().or_().with<const HasChangedRotation2D>().or_().with<const HasChangedScale2D>()
            .term_at<Transform2D>().out()
            .write<HasChangedTransform2D>()
            .multi_threaded()
            .each([](
                stagehand::entity e,
                Transform2D &transform,
                const Position2D &position,
                const Rotation2D &rotation,
                const Scale2D &scale
            ){
                e.modify(transform, [&](Transform2D &t) {
                    t.set_origin(position);
                    t.set_rotation_and_scale(rotation, scale);
                });
            });

        world.system<
                Transform3D,
                const Position3D,
                const Rotation3D,
                const Scale3D
            >(stagehand::names::systems::TRANSFORM_UPDATE_3D)
            .kind(stagehand::PreRender)
            .with<const HasChangedPosition3D>().or_().with<const HasChangedRotation3D>().or_().with<const HasChangedScale3D>()
            .term_at<Transform3D>().out()
            .write<HasChangedTransform3D>()
            .multi_threaded()
            .each([](
                stagehand::entity e,
                Transform3D &transform,
                const Position3D &position,
                const Rotation3D &rotation,
                const Scale3D &scale
            ){
                e.modify(transform, [&](Transform3D &t) {
                    t = Transform3D(Basis(rotation).scaled(scale), position);
                });
            });
        // clang-format on
    });
} // namespace stagehand::transform
