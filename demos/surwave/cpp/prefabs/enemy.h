#pragma once

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/prefabs/character2d.h"

namespace stagehand_demos::surwave {
    inline flecs::entity EnemyPrefab;
}

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    EnemyPrefab = world.prefab("Enemy")
                      .is_a(Character2DPrefab)
                      .set_auto_override<HitPoints>({godot::real_t(10.0)})
                      .set_auto_override<HitRadius>({godot::real_t(14.0)})
                      .set_auto_override<MeleeDamage>({godot::real_t(10.0)})
                      .set_auto_override<MovementSpeed>({godot::real_t(9.0)})
                      .set<AnimationFrameOffset>({godot::real_t(0.0)})
                      .set_auto_override<DeathTimer>({godot::real_t(0.0)})
                      .set_auto_override<HitReactionTimer>({godot::real_t(0.0)})
                      .set_auto_override<HFlipTimer>({godot::real_t(0.5)})
                      .set_auto_override<VFlipTimer>({godot::real_t(0.5)})
                      .set_auto_override<ProjectileHitTimeout>({godot::real_t(0.0)})
                      .set_auto_override<ShockwaveHitTimeout>({godot::real_t(0.0)});
});
