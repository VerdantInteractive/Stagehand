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

    struct EnemyBoidMovementSettings {
        float player_attraction_weight;
        float player_engage_radius;
        float neighbor_radius;
        float separation_radius;
        float max_speed_multiplier;
        float max_force;
        float grid_cell_size;
        float separation_weight;
        float kd_tree_rebuild_distance;
        float kd_tree_max_stale_frames;
        float max_neighbor_sample_count;
        float separation_noise_intensity;
    };

    struct EnemyAnimationSettings {
        float animation_interval;
        float walk_animation_range;
        float death_animation_frame_count;
        float up_direction_frame_offset;
        float horizontal_flip_cooldown;
        float vertical_flip_cooldown;
        float nominal_movement_speed;
        float animation_offset_fraction_range;
        float hit_reaction_duration;
    };

    struct EnemyTakeDamageSettings {
        float projectile_hit_cooldown;
        float shockwave_hit_cooldown;
        float projectile_damage;
        float shockwave_damage;
    };

    struct PlayerTakeDamageSettings {
        float damage_cooldown;
        float player_hit_radius;
    };

    inline auto register_game_singletons = stagehand::Registry("stagehand_demos::surwave", [](flecs::world &world) {
        world.component<EnemyBoidMovementSettings>()
            .member<float>("player_attraction_weight")
            .member<float>("player_engage_distance")
            .member<float>("neighbor_radius")
            .member<float>("separation_radius")
            .member<float>("max_speed_multiplier")
            .member<float>("max_force")
            .member<float>("grid_cell_size")
            .member<float>("separation_weight")
            .member<float>("kd_tree_rebuild_distance")
            .member<float>("kd_tree_max_stale_frames")
            .member<float>("max_neighbor_sample_count")
            .member<float>("separation_noise_intensity")
            .add(flecs::Singleton)
            .set<EnemyBoidMovementSettings>({
                1.0f,   // player_attraction_weight
                28.0f,  // player_engage_distance
                110.0f, // neighbor_radius
                30.0f,  // separation_radius
                1.1f,   // max_speed_multiplier
                220.0f, // max_force
                96.0f,  // grid_cell_size
                1.0f,   // separation_weight
                35.0f,  // kd_tree_rebuild_distance
                8.0f,   // kd_tree_max_stale_frames
                48.0f,  // max_neighbor_sample_count
                0.05f   // separation_noise_intensity
            });

        world.component<EnemyAnimationSettings>()
            .member<float>("animation_interval")
            .member<float>("walk_animation_range")
            .member<float>("death_animation_frame_count")
            .member<float>("up_direction_frame_offset")
            .member<float>("horizontal_flip_cooldown")
            .member<float>("vertical_flip_cooldown")
            .member<float>("nominal_movement_speed")
            .member<float>("animation_offset_fraction_range")
            .member<float>("hit_reaction_duration")
            .add(flecs::Singleton)
            .set<EnemyAnimationSettings>({
                0.25f, // animation_interval
                5.0f,  // walk_animation_range
                4.0f,  // death_animation_frame_count
                6.0f,  // up_direction_frame_offset
                0.5f,  // horizontal_flip_cooldown
                0.5f,  // vertical_flip_cooldown
                9.0f,  // nominal_movement_speed
                0.3f,  // animation_offset_fraction_range
                0.1f   // hit_reaction_duration
            });

        world.component<EnemyTakeDamageSettings>()
            .member<float>("projectile_hit_cooldown")
            .member<float>("shockwave_hit_cooldown")
            .member<float>("projectile_damage")
            .member<float>("shockwave_damage")
            .add(flecs::Singleton)
            .set<EnemyTakeDamageSettings>({
                0.2f, // projectile_hit_cooldown
                1.0f, // shockwave_hit_cooldown
                1.0f, // projectile_damage
                1.0f  // shockwave_damage
            });

        world.component<PlayerTakeDamageSettings>()
            .member<float>("damage_cooldown")
            .member<float>("player_hit_radius")
            .add(flecs::Singleton)
            .set<PlayerTakeDamageSettings>({0.3f, 9.0f});
    });

} // namespace stagehand_demos::surwave
