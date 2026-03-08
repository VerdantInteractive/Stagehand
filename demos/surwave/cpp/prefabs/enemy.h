#pragma once

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/prefabs/character2d.h"

namespace stagehand_demos::surwave {
    inline flecs::entity EnemyPrefab;
}

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    EnemyPrefab = world.prefab("Enemy")
                      .is_a(Character2DPrefab)
                      .set<HitPoints>({godot::real_t(10.0)})
                      .set<HitRadius>({godot::real_t(14.0)})
                      .set<MeleeDamage>({godot::real_t(10.0)})
                      .set<MovementSpeed>({godot::real_t(9.0)})
                      .set<AnimationFrameOffset>({godot::real_t(0.0)})
                      .set<DeathTimer>({godot::real_t(0.0)})
                      .set<HitReactionTimer>({godot::real_t(0.0)})
                      .set<HFlipTimer>({godot::real_t(0.5)})
                      .set<VFlipTimer>({godot::real_t(0.5)})
                      .set<ProjectileHitTimeout>({godot::real_t(0.0)})
                      .set<ShockwaveHitTimeout>({godot::real_t(0.0)});
    // clang-format on
});
