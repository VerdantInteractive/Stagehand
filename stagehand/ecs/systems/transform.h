#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
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
            .with<const ChangedPosition2D>().or_().with<const ChangedRotation2D>().or_().with<const ChangedScale2D>()
            .term_at<Transform2D>().out()
            .write<ChangedTransform2D>()
            .multi_threaded()
            .each([](
                flecs::entity e,
                Transform2D &transform,
                const Position2D &position,
                const Rotation2D &rotation,
                const Scale2D &scale
            ){
                transform.set_origin(position);
                transform.set_rotation_and_scale(rotation, scale);
                e.add<ChangedTransform2D>();
            });

        world.system<
                Transform3D,
                const Position3D,
                const Rotation3D,
                const Scale3D
            >(stagehand::names::systems::TRANSFORM_UPDATE_3D)
            .kind(stagehand::PreRender)
            .with<const ChangedPosition3D>().or_().with<const ChangedRotation3D>().or_().with<const ChangedScale3D>()
            .term_at<Transform3D>().out()
            .write<ChangedTransform3D>()
            .multi_threaded()
            .each([](
                flecs::entity e,
                Transform3D &transform,
                const Position3D &position,
                const Rotation3D &rotation,
                const Scale3D &scale
            ){
                transform = Transform3D(Basis(rotation).scaled(scale), position);
                e.add<ChangedTransform3D>();
            });
        // clang-format on
    });

    template <typename Tag> static void register_tag_reset(flecs::world &world, const char *tag_name) {
        std::string sys_name = std::string("stagehand::transform::Tag Reset (") + tag_name + ")";
        // clang-format off
    world.system<>(sys_name.c_str())
        .kind(stagehand::PostRender)
        .with<Tag>().template write<Tag>()
        .each([](flecs::entity e) { e.remove<Tag>(); });
        // clang-format on
    }

    REGISTER([](flecs::world &world) {
        register_tag_reset<ChangedPosition2D>(world, "ChangedPosition2D");
        register_tag_reset<ChangedPosition3D>(world, "ChangedPosition3D");
        register_tag_reset<ChangedRotation2D>(world, "ChangedRotation2D");
        register_tag_reset<ChangedRotation3D>(world, "ChangedRotation3D");
        register_tag_reset<ChangedScale2D>(world, "ChangedScale2D");
        register_tag_reset<ChangedScale3D>(world, "ChangedScale3D");
        register_tag_reset<ChangedTransform2D>(world, "ChangedTransform2D");
        register_tag_reset<ChangedTransform3D>(world, "ChangedTransform3D");
    });

} // namespace stagehand::transform
