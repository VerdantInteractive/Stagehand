#pragma once

#include <flecs.h>

#include "stagehand/ecs/components/traits.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/registry.h"

namespace stagehand {
    REGISTER([](flecs::world &world) {
        // Disables all change detection tags. Uses a query with variables to match all entities
        // with any component/tag marked with the IsChangeDetectionTag trait, then disables those tags.
        // clang-format off
        world.system(names::systems::TAG_RESET_CHANGE_DETECTION)
            .kind(PostRender)
            .with<IsChangeDetectionTag>().src("$change_detection_tag_component")
            .term().first("$change_detection_tag_component")
            .run([](flecs::iter &it) {
                // clang-format on
                while (it.next()) {
                    flecs::id_t component_id = it.id(1);
                    for (auto i : it) {
                        it.entity(i).disable(component_id);
                    }
                }
            });
    });
} // namespace stagehand
