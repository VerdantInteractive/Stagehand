#pragma once

#include "stagehand/registry.h"

namespace stagehand {
    /// A trait that can be added to the Changed... tag components to indicate that they are used for tracking changes to components they are associated with.
    struct IsChangeDetectionTag {};
    inline auto register_change_detection_trait =
        stagehand::ComponentRegistrar<IsChangeDetectionTag>([](flecs::world &world) { world.component<IsChangeDetectionTag>().add(flecs::Trait); });
} // namespace stagehand
