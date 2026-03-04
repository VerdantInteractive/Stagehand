#pragma once

#include <cstddef>

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"

namespace stagehand_demos::surwave {

    UINT32(EnemyCount).then([](auto component) { component.add(flecs::Singleton); });
    GODOT_VARIANT(ProjectileData, godot::Dictionary).then([](auto component) { component.add(flecs::Singleton); });
    GODOT_VARIANT(ShockwaveData, godot::Dictionary).then([](auto component) { component.add(flecs::Singleton); });

    GODOT_VARIANT(PlayerPosition, godot::Vector2).then([](auto component) { component.add(flecs::Singleton); });
    FLOAT(PlayerDamageCooldown, 0.3f).then([](auto component) { component.add(flecs::Singleton); });

    struct EnemyBoidMovementSettings {
        godot::real_t player_attraction_weight;
        godot::real_t player_engage_radius;
        godot::real_t neighbor_radius;
        godot::real_t separation_radius;
        godot::real_t max_speed_multiplier;
        godot::real_t max_force;
        godot::real_t grid_cell_size;
        godot::real_t separation_weight;
        godot::real_t kd_tree_rebuild_distance;
        godot::real_t kd_tree_max_stale_frames;
        godot::real_t max_neighbor_sample_count;
        godot::real_t separation_noise_intensity;
    };

    struct EnemyAnimationSettings {
        godot::real_t animation_interval;
        godot::real_t walk_animation_range;
        godot::real_t death_animation_frame_count;
        godot::real_t up_direction_frame_offset;
        godot::real_t horizontal_flip_cooldown;
        godot::real_t vertical_flip_cooldown;
        godot::real_t nominal_movement_speed;
        godot::real_t animation_offset_fraction_range;
        godot::real_t hit_reaction_duration;
    };

    struct EnemyTakeDamageSettings {
        godot::real_t projectile_hit_cooldown;
        godot::real_t shockwave_hit_cooldown;
        godot::real_t projectile_damage;
        godot::real_t shockwave_damage;
    };

    struct PlayerTakeDamageSettings {
        godot::real_t damage_cooldown;
        godot::real_t player_hit_radius;
    };

    inline auto register_game_singletons = stagehand::Registry("stagehand_demos::surwave", [](flecs::world &world) {
        world.component<EnemyBoidMovementSettings>("EnemyBoidMovementSettings")
            .member<godot::real_t>("player_attraction_weight")
            .member<godot::real_t>("player_engage_distance")
            .member<godot::real_t>("neighbor_radius")
            .member<godot::real_t>("separation_radius")
            .member<godot::real_t>("max_speed_multiplier")
            .member<godot::real_t>("max_force")
            .member<godot::real_t>("grid_cell_size")
            .member<godot::real_t>("separation_weight")
            .member<godot::real_t>("kd_tree_rebuild_distance")
            .member<godot::real_t>("kd_tree_max_stale_frames")
            .member<godot::real_t>("max_neighbor_sample_count")
            .member<godot::real_t>("separation_noise_intensity")
            .add(flecs::Singleton)
            .set<EnemyBoidMovementSettings>({
                godot::real_t(1.0),   // player_attraction_weight
                godot::real_t(28.0),  // player_engage_distance
                godot::real_t(110.0), // neighbor_radius
                godot::real_t(30.0),  // separation_radius
                godot::real_t(1.1),   // max_speed_multiplier
                godot::real_t(220.0), // max_force
                godot::real_t(96.0),  // grid_cell_size
                godot::real_t(1.0),   // separation_weight
                godot::real_t(35.0),  // kd_tree_rebuild_distance
                godot::real_t(8.0),   // kd_tree_max_stale_frames
                godot::real_t(48.0),  // max_neighbor_sample_count
                godot::real_t(0.05)   // separation_noise_intensity
            });

        world.component<EnemyAnimationSettings>("EnemyAnimationSettings")
            .member<godot::real_t>("animation_interval")
            .member<godot::real_t>("walk_animation_range")
            .member<godot::real_t>("death_animation_frame_count")
            .member<godot::real_t>("up_direction_frame_offset")
            .member<godot::real_t>("horizontal_flip_cooldown")
            .member<godot::real_t>("vertical_flip_cooldown")
            .member<godot::real_t>("nominal_movement_speed")
            .member<godot::real_t>("animation_offset_fraction_range")
            .member<godot::real_t>("hit_reaction_duration")
            .add(flecs::Singleton)
            .set<EnemyAnimationSettings>({
                godot::real_t(0.25), // animation_interval
                godot::real_t(5.0),  // walk_animation_range
                godot::real_t(4.0),  // death_animation_frame_count
                godot::real_t(6.0),  // up_direction_frame_offset
                godot::real_t(0.5),  // horizontal_flip_cooldown
                godot::real_t(0.5),  // vertical_flip_cooldown
                godot::real_t(9.0),  // nominal_movement_speed
                godot::real_t(0.3),  // animation_offset_fraction_range
                godot::real_t(0.1)   // hit_reaction_duration
            });

        world.component<EnemyTakeDamageSettings>("EnemyTakeDamageSettings")
            .member<godot::real_t>("projectile_hit_cooldown")
            .member<godot::real_t>("shockwave_hit_cooldown")
            .member<godot::real_t>("projectile_damage")
            .member<godot::real_t>("shockwave_damage")
            .add(flecs::Singleton)
            .set<EnemyTakeDamageSettings>({
                godot::real_t(0.2), // projectile_hit_cooldown
                godot::real_t(1.0), // shockwave_hit_cooldown
                godot::real_t(1.0), // projectile_damage
                godot::real_t(1.0)  // shockwave_damage
            });

        world.component<PlayerTakeDamageSettings>("PlayerTakeDamageSettings")
            .member<godot::real_t>("damage_cooldown")
            .member<godot::real_t>("player_hit_radius")
            .add(flecs::Singleton)
            .set<PlayerTakeDamageSettings>({godot::real_t(0.3), godot::real_t(9.0)});
    });

} // namespace stagehand_demos::surwave
