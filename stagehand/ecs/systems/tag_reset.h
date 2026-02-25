#pragma once

#include <string>

#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/registry.h"

using namespace stagehand;

template <typename Tag> static void register_tag_reset(flecs::world &world, const char *tag_name) {
    std::string sys_name = std::string("stagehand::Tag Reset (") + tag_name + ")";
    // clang-format off
    world.system<>(sys_name.c_str())
        .kind(stagehand::PostRender)
        .with<Tag>().template write<Tag>()
        .each([](flecs::entity e) { e.remove<Tag>(); });
    // clang-format on
}

REGISTER([](flecs::world &world) {
    register_tag_reset<transform::ChangedPosition2D>(world, "ChangedPosition2D");
    register_tag_reset<transform::ChangedPosition3D>(world, "ChangedPosition3D");
    register_tag_reset<transform::ChangedRotation2D>(world, "ChangedRotation2D");
    register_tag_reset<transform::ChangedRotation3D>(world, "ChangedRotation3D");
    register_tag_reset<transform::ChangedScale2D>(world, "ChangedScale2D");
    register_tag_reset<transform::ChangedScale3D>(world, "ChangedScale3D");
    register_tag_reset<transform::ChangedTransform2D>(world, "ChangedTransform2D");
    register_tag_reset<transform::ChangedTransform3D>(world, "ChangedTransform3D");
});
