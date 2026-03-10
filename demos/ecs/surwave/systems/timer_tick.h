#pragma once

#include <cstddef>

#include <godot_cpp/core/math.hpp>

#include "stagehand/registry.h"

#include "demos/ecs/surwave/components/enemy.h"
#include "demos/ecs/surwave/components/singletons.h"
#include "demos/ecs/surwave/prefabs/enemy.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world.system<
        ProjectileHitTimeout,
        ShockwaveHitTimeout,
        DeathTimer,
        HitReactionTimer,
        HFlipTimer,
        VFlipTimer,
        const EnemyTakeDamageSettings
    >("Enemy Timer Tick")
        .with(flecs::IsA, EnemyPrefab)
        .kind(flecs::PreUpdate)
        .run([](flecs::iter &it) {
            // clang-format on
            while (it.next()) {
                const float delta_time = godot::Math::max(static_cast<float>(it.delta_time()), 0.0f);
                if (delta_time <= 0.0f) {
                    continue;
                }

                flecs::field<ProjectileHitTimeout> projectile_timeouts = it.field<ProjectileHitTimeout>(0);
                flecs::field<ShockwaveHitTimeout> shockwave_timeouts = it.field<ShockwaveHitTimeout>(1);
                flecs::field<DeathTimer> death_timers = it.field<DeathTimer>(2);
                flecs::field<HitReactionTimer> hit_reaction_timers = it.field<HitReactionTimer>(3);
                flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(4);
                flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(5);
                flecs::field<const EnemyTakeDamageSettings> take_damage_settings_field = it.field<const EnemyTakeDamageSettings>(6);

                const EnemyTakeDamageSettings *take_damage_settings = &take_damage_settings_field[0];
                const float projectile_cooldown =
                    take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->projectile_hit_cooldown, 0.0f) : 0.0f;
                const float shockwave_cooldown = take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->shockwave_hit_cooldown, 0.0f) : 0.0f;
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

                    if (death_timer.value > 0.0f) {
                        death_timer.value -= delta_time;
                        if (death_timer.value <= 0.0f) {
                            death_timer.value = 0.0f;

                            flecs::entity entity = it.entity(static_cast<std::int32_t>(entity_index));

                            entity.destruct();
                            continue;
                        }
                    }

                    hit_reaction_timer.value = godot::Math::max(hit_reaction_timer.value - delta_time, 0.0f);

                    horizontal_timer.value = godot::Math::max(horizontal_timer.value + delta_time, 0.0f);
                    vertical_timer.value = godot::Math::max(vertical_timer.value + delta_time, 0.0f);
                }
            }
        });

    // clang-format off
        world.system<const PlayerTakeDamageSettings>("Player Damage Timer Tick")
            .with<PlayerDamageCooldown>().inout()
        .kind(flecs::PreUpdate)
        .run([](flecs::iter &it) {
        // clang-format on
        while (it.next()) {
            flecs::field<const PlayerTakeDamageSettings> player_damage_settings = it.field<const PlayerTakeDamageSettings>(0);
            flecs::field<PlayerDamageCooldown> player_damage_cooldown_field = it.field<PlayerDamageCooldown>(1);
            PlayerDamageCooldown &player_damage_cooldown = player_damage_cooldown_field[0];

            const float cooldown = godot::Math::max(player_damage_settings->damage_cooldown, 0.0f);
            if (cooldown <= 0.0f) {
                player_damage_cooldown.value = cooldown;
                return;
            }

            const float delta_time = godot::Math::max(static_cast<float>(it.delta_time()), 0.0f);
            if (delta_time <= 0.0f) {
                return;
            }

            player_damage_cooldown.value = godot::Math::min(player_damage_cooldown.value + delta_time, cooldown);
        }
    });
});
