#pragma once

#include <cstddef>

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"

namespace stagehand_demos::surwave {

    UINT32_(EnemyCount).then([](auto component) { component.add(flecs::Singleton); });
    GODOT_VARIANT_(ProjectileData, godot::Dictionary).then([](auto component) { component.add(flecs::Singleton); });
    GODOT_VARIANT_(ShockwaveData, godot::Dictionary).then([](auto component) { component.add(flecs::Singleton); });

    GODOT_VARIANT_(PlayerPosition, godot::Vector2).then([](auto component) { component.add(flecs::Singleton); });
    FLOAT_(PlayerDamageCooldown, 0.3f).then([](auto component) { component.add(flecs::Singleton); });

    STRUCT_(EnemyBoidMovementSettings, {
        float player_attraction_weight = 1.0f;
        float player_engage_radius = 28.0f;
        float neighbor_radius = 110.0f;
        float separation_radius = 30.0f;
        float max_speed_multiplier = 1.1f;
        float max_force = 220.0f;
        float grid_cell_size = 96.0f;
        float separation_weight = 1.0f;
        float kd_tree_rebuild_distance = 35.0f;
        float kd_tree_max_stale_frames = 8.0f;
        float max_neighbor_sample_count = 48.0f;
        float separation_noise_intensity = 0.05f;
    }).then([](auto component) { component.add(flecs::Singleton); });

    STRUCT_(EnemyAnimationSettings, {
        float animation_interval = 0.25f;
        float walk_animation_range = 5.0f;
        float death_animation_frame_count = 4.0f;
        float up_direction_frame_offset = 6.0f;
        float horizontal_flip_cooldown = 0.5f;
        float vertical_flip_cooldown = 0.5f;
        float nominal_movement_speed = 9.0f;
        float animation_offset_fraction_range = 0.3f;
        float hit_reaction_duration = 0.1f;
    }).then([](auto component) { component.add(flecs::Singleton); });

    STRUCT_(EnemyTakeDamageSettings, {
        float projectile_hit_cooldown = 0.2f;
        float shockwave_hit_cooldown = 1.0f;
        float projectile_damage = 1.0f;
        float shockwave_damage = 1.0f;
    }).then([](auto component) { component.add(flecs::Singleton); });

    STRUCT_(PlayerTakeDamageSettings, {
        float damage_cooldown = 0.3f;
        float player_hit_radius = 9.0f;
    }).then([](auto component) { component.add(flecs::Singleton); });

} // namespace stagehand_demos::surwave
