#pragma once

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    const flecs::query<> enemy_instance_query = world.query_builder<>().with(flecs::IsA, EnemyPrefab).build();

    world.system<>("Enemy Count Update").kind(flecs::PostUpdate).run([enemy_instance_query](flecs::iter &it) {
        flecs::world stage_world = it.world();
        const uint32_t current_enemy_count = static_cast<uint32_t>(enemy_instance_query.iter(stage_world.c_ptr()).count());

        const EnemyCount *singleton_component = stage_world.try_get<EnemyCount>();
        if (singleton_component == nullptr || singleton_component->value != current_enemy_count) {
            stage_world.set<EnemyCount>({current_enemy_count});
        }
    });
});
