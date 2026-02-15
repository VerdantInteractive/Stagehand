#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

inline stagehand::Registry register_transform_update_systems([](flecs::world &world) {
    world
        .system<Transform2D, const stagehand::Position2D, const stagehand::Rotation2D, const stagehand::Scale2D>(stagehand::names::systems::TRANSFORM_UPDATE_2D)
        .kind(stagehand::PreRender)
        .multi_threaded()
        .term_at<Transform2D>()
        .out()
        .each([](Transform2D &transform, const stagehand::Position2D &position, const stagehand::Rotation2D &rotation, const stagehand::Scale2D &scale) {
            transform.set_origin(position);
            transform.set_rotation_and_scale(rotation, scale);
        });

    world
        .system<Transform3D, const stagehand::Position3D, const stagehand::Rotation3D, const stagehand::Scale3D>(stagehand::names::systems::TRANSFORM_UPDATE_3D)
        .kind(stagehand::PreRender)
        .multi_threaded()
        .term_at<Transform3D>()
        .out()
        .each([](Transform3D &transform, const stagehand::Position3D &position, const stagehand::Rotation3D &rotation, const stagehand::Scale3D &scale) {
            transform = Transform3D(Basis(rotation).scaled(scale), position);
        });
});
