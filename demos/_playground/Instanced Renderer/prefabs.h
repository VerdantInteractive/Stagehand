#pragma once

#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"

#include "components.h"
#include "names.h"

namespace instanced_renderer {
    inline flecs::entity TestInstancePrefab;

    inline stagehand::Registry register_test_instance_prefab([](flecs::world &world) {
        TestInstancePrefab = world.prefab(names::prefabs::TEST_INSTANCE)
                                 .add<stagehand::transform::Position3D>()
                                 .add<stagehand::transform::Rotation3D>()
                                 .set<stagehand::transform::Scale3D>({1.0f, 1.0f, 1.0f})
                                 .add<stagehand::transform::Transform3D>()
                                 .add<InstanceColour>();
    });
} // namespace instanced_renderer
