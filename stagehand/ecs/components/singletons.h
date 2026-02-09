#pragma once

#include <godot_cpp/variant/dictionary.hpp>

#include "stagehand/registry.h"
#include "stagehand/ecs/components/macros.h"

namespace stagehand {

    struct SceneChildren {
        godot::Dictionary nodes;
    };

} // namespace stagehand

inline stagehand::Registry register_singleton_components([](flecs::world& world) {
    world.component<stagehand::SceneChildren>()
        .add(flecs::Singleton);
});
