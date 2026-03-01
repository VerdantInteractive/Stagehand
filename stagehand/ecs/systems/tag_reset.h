#pragma once

#include <flecs.h>

#include "stagehand/ecs/components/traits.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/registry.h"

namespace stagehand {
    REGISTER([](flecs::world &world) {
        // Generic system that disables all change detection tags. Uses a wildcard query
        // to match all entities with any component/tag marked with the IsChangeDetectionTag trait, then disables those tags.
        world.system(names::systems::TAG_RESET_CHANGE_DETECTION)
            .kind(PostRender)
            .with(flecs::Wildcard)
            .second<IsChangeDetectionTag>()
            .each([](flecs::iter &it, size_t i) {
                // Disable the component/tag matched by the wildcard (identified by id(0))
                it.entity(i).disable(it.id(0));
            });
    });
} // namespace stagehand
