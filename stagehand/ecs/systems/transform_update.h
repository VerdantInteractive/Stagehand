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
    stagehand::transform::TransformUpdate2D =
        world
            .system<Transform2D, const stagehand::transform::Position2D, const stagehand::transform::Rotation2D, const stagehand::transform::Scale2D>(
                stagehand::names::systems::TRANSFORM_UPDATE_2D)
            .kind(stagehand::PreRender)
            .multi_threaded()
            .term_at<Transform2D>()
            .out()
            .each([](Transform2D &transform, const stagehand::transform::Position2D &position, const stagehand::transform::Rotation2D &rotation,
                     const stagehand::transform::Scale2D &scale) {
                transform.set_origin(position);
                transform.set_rotation_and_scale(rotation, scale);
            });

    stagehand::transform::TransformUpdate3D =
        world
            .system<Transform3D, const stagehand::transform::Position3D, const stagehand::transform::Rotation3D, const stagehand::transform::Scale3D>(
                stagehand::names::systems::TRANSFORM_UPDATE_3D)
            .kind(stagehand::PreRender)
            .multi_threaded()
            .term_at<Transform3D>()
            .out()
            .each([](Transform3D &transform, const stagehand::transform::Position3D &position, const stagehand::transform::Rotation3D &rotation,
                     const stagehand::transform::Scale3D &scale) { transform = Transform3D(Basis(rotation).scaled(scale), position); });
});
