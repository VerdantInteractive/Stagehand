#pragma once

#include "stagehand/registry.h"

#include "demos/ecs/surwave/components/enemy.h"
#include "demos/ecs/surwave/prefabs/character2d.h"

namespace stagehand_demos::surwave {
    inline flecs::entity EnemyPrefab;
}

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    EnemyPrefab = world.prefab("Enemy")
                      .is_a(Character2DPrefab)
                      .set<HitPoints>({10.0f})
                      .set<HitRadius>({14.0f})
                      .set<MeleeDamage>({10.0f})
                      .set<MovementSpeed>({9.0f})
                      .set<AnimationFrameOffset>({0.0f})
                      .set<DeathTimer>({0.0f})
                      .set<HitReactionTimer>({0.0f})
                      .set<HFlipTimer>({0.5f})
                      .set<VFlipTimer>({0.5f})
                      .set<ProjectileHitTimeout>({0.0f})
                      .set<ShockwaveHitTimeout>({0.0f});
    // clang-format on
});
