#pragma once

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    const flecs::query<> enemy_instance_query = world.query_builder<>().with(flecs::IsA, EnemyPrefab).build();

    // clang-format off
    world.system<>("Enemy Count Update")
        .kind(flecs::PostUpdate)
        .run([enemy_instance_query](flecs::iter &it) {
        // clang-format on
        const uint32_t current_enemy_count = static_cast<uint32_t>(enemy_instance_query.iter(it).count());

        EnemyCount *singleton_component = it.world().try_get_mut<EnemyCount>();
        if (singleton_component == nullptr) {
            it.world().set<EnemyCount>({current_enemy_count});
            return;
        }

        if (singleton_component->value != current_enemy_count) {
            singleton_component->value = current_enemy_count;
            it.world().modified<EnemyCount>();
        }
    });
});
