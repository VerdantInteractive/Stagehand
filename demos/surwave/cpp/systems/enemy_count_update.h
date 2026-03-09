#pragma once

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    const flecs::query<> enemy_instance_query = world.query_builder<>().with(flecs::IsA, EnemyPrefab).build();

    // clang-format off
    world.system<>("Enemy Count Update")
        .with<EnemyCount>().inout()
        .kind(flecs::PostUpdate)
        .run([enemy_instance_query](flecs::iter &it) {
        // clang-format on
        while (it.next()) {
            const uint32_t current_enemy_count = static_cast<uint32_t>(enemy_instance_query.iter(it).count());
            flecs::field<EnemyCount> enemy_count_field = it.field<EnemyCount>(0);
            EnemyCount &enemy_count = enemy_count_field[0];

            if (enemy_count.value != current_enemy_count) {
                enemy_count.value = current_enemy_count;
            }
        }
    });
});
