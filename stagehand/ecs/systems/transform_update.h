#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

using namespace stagehand::transform;

namespace stagehand::transform {
    inline flecs::system TransformUpdate2D;
    inline flecs::system TransformUpdate3D;
} // namespace stagehand::transform

REGISTER([](flecs::world &world) {
    // clang-format off
    stagehand::transform::TransformUpdate2D =
        world.system<
                stagehand::transform::Transform2D,
                const stagehand::transform::Position2D,
                const stagehand::transform::Rotation2D,
                const stagehand::transform::Scale2D
            >(stagehand::names::systems::TRANSFORM_UPDATE_2D)
            .kind(stagehand::PreRender)
            .with<const stagehand::transform::ChangedPosition2D>().or_().with<const stagehand::transform::ChangedRotation2D>().or_().with<const stagehand::transform::ChangedScale2D>()
            .term_at<stagehand::transform::Transform2D>().out()
            .write<stagehand::transform::ChangedTransform2D>()
            .multi_threaded()
            .each([](
                flecs::entity e,
                stagehand::transform::Transform2D &transform,
                const stagehand::transform::Position2D &position,
                const stagehand::transform::Rotation2D &rotation,
                const stagehand::transform::Scale2D &scale
            ){
                transform.set_origin(position);
                transform.set_rotation_and_scale(rotation, scale);
                e.add<stagehand::transform::ChangedTransform2D>();
            });

    stagehand::transform::TransformUpdate3D =
        world.system<
                stagehand::transform::Transform3D,
                const stagehand::transform::Position3D,
                const stagehand::transform::Rotation3D,
                const stagehand::transform::Scale3D
            >(stagehand::names::systems::TRANSFORM_UPDATE_3D)
            .kind(stagehand::PreRender)
            .with<const stagehand::transform::ChangedPosition3D>().or_().with<const stagehand::transform::ChangedRotation3D>().or_().with<const stagehand::transform::ChangedScale3D>()
            .term_at<stagehand::transform::Transform3D>().out()
            .write<stagehand::transform::ChangedTransform3D>()
            .multi_threaded()
            .each([](
                flecs::entity e,
                stagehand::transform::Transform3D &transform,
                const stagehand::transform::Position3D &position,
                const stagehand::transform::Rotation3D &rotation,
                const stagehand::transform::Scale3D &scale
            ){
                transform = stagehand::transform::Transform3D(Basis(rotation).scaled(scale), position);
                e.add<stagehand::transform::ChangedTransform3D>();
            });
    // clang-format on
});
