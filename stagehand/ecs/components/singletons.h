#pragma once

#include "stagehand/registry.h"
#include "stagehand/ecs/components/godot_variants.h"

namespace stagehand {

    GODOT_VARIANT(SceneChildren, godot::Dictionary)
        .then([](flecs::component<SceneChildren> c) { c.add(flecs::Singleton); });

    GODOT_VARIANT(WorldConfiguration, godot::Dictionary)
        .then([](flecs::component<WorldConfiguration> c) { c.add(flecs::Singleton); });

} // namespace stagehand
