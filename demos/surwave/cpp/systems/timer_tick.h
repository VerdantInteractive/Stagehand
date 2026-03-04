#pragma once

#include <cstddef>

#include <godot_cpp/core/math.hpp>

#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    world.system<ProjectileHitTimeout, ShockwaveHitTimeout, DeathTimer, HitReactionTimer, HFlipTimer, VFlipTimer>("Enemy Timer Tick")
        .with(flecs::IsA, EnemyPrefab)
        .kind(flecs::PreUpdate)
        .run([](flecs::iter &it) {
            const EnemyTakeDamageSettings *take_damage_settings = it.world().try_get<EnemyTakeDamageSettings>();
            const godot::real_t projectile_cooldown =
                take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->projectile_hit_cooldown, godot::real_t(0.0)) : godot::real_t(0.0);
            const godot::real_t shockwave_cooldown =
                take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->shockwave_hit_cooldown, godot::real_t(0.0)) : godot::real_t(0.0);

            while (it.next()) {
                const godot::real_t delta_time = godot::Math::max(static_cast<godot::real_t>(it.delta_time()), godot::real_t(0.0));
                if (delta_time <= godot::real_t(0.0)) {
                    continue;
                }

                flecs::field<ProjectileHitTimeout> projectile_timeouts = it.field<ProjectileHitTimeout>(0);
                flecs::field<ShockwaveHitTimeout> shockwave_timeouts = it.field<ShockwaveHitTimeout>(1);
                flecs::field<DeathTimer> death_timers = it.field<DeathTimer>(2);
                flecs::field<HitReactionTimer> hit_reaction_timers = it.field<HitReactionTimer>(3);
                flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(4);
                flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(5);
                const std::size_t entity_count = it.count();
                for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                    ProjectileHitTimeout &projectile_timeout = projectile_timeouts[entity_index];
                    ShockwaveHitTimeout &shockwave_timeout = shockwave_timeouts[entity_index];
                    DeathTimer &death_timer = death_timers[entity_index];
                    HitReactionTimer &hit_reaction_timer = hit_reaction_timers[entity_index];
                    HFlipTimer &horizontal_timer = horizontal_flip_timers[entity_index];
                    VFlipTimer &vertical_timer = vertical_flip_timers[entity_index];

                    projectile_timeout.value = godot::Math::min(projectile_timeout.value + delta_time, projectile_cooldown);
                    shockwave_timeout.value = godot::Math::min(shockwave_timeout.value + delta_time, shockwave_cooldown);

                    if (death_timer.value > godot::real_t(0.0)) {
                        death_timer.value -= delta_time;
                        if (death_timer.value <= godot::real_t(0.0)) {
                            death_timer.value = godot::real_t(0.0);

                            flecs::entity entity = it.entity(static_cast<std::int32_t>(entity_index));

                            entity.destruct();
                            continue;
                        }
                    }

                    hit_reaction_timer.value = godot::Math::max(hit_reaction_timer.value - delta_time, godot::real_t(0.0));

                    horizontal_timer.value = godot::Math::max(horizontal_timer.value + delta_time, godot::real_t(0.0));
                    vertical_timer.value = godot::Math::max(vertical_timer.value + delta_time, godot::real_t(0.0));
                }
            }
        });

    world.system<>("Player Damage Timer Tick").kind(flecs::PreUpdate).run([](flecs::iter &it) {
        flecs::world stage_world = it.world();
        const PlayerTakeDamageSettings *player_damage_settings = stage_world.try_get<PlayerTakeDamageSettings>();
        PlayerDamageCooldown *player_damage_cooldown = stage_world.try_get_mut<PlayerDamageCooldown>();
        if (player_damage_settings == nullptr || player_damage_cooldown == nullptr) {
            return;
        }

        const godot::real_t cooldown = godot::Math::max(player_damage_settings->damage_cooldown, godot::real_t(0.0));
        if (cooldown <= godot::real_t(0.0)) {
            player_damage_cooldown->value = cooldown;
            return;
        }

        const godot::real_t delta_time = godot::Math::max(static_cast<godot::real_t>(it.delta_time()), godot::real_t(0.0));
        if (delta_time <= godot::real_t(0.0)) {
            return;
        }

        player_damage_cooldown->value = godot::Math::min(player_damage_cooldown->value + delta_time, cooldown);
    });
});
