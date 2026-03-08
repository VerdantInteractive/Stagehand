#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "stagehand/ecs/components/event_payload.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"
#include "stagehand/utilities/emit_signal.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"
#include "demos/surwave/cpp/utilities/enemy_spatial_hash.h"
#include "demos/surwave/cpp/utilities/enemy_type.h"

using namespace stagehand_demos::surwave;
using Position2D = stagehand::transform::Position2D;

namespace enemy_take_damage {

    struct DamageTargetAccessor {
        const Position2D *position;
        HitPoints *hit_points;
        const HitRadius *hit_radius;
        ProjectileHitTimeout *projectile_hit_timeout;
        ShockwaveHitTimeout *shockwave_hit_timeout;
        HitReactionTimer *hit_reaction_timer;
        flecs::entity entity;
    };

    inline godot::Array get_projectile_positions(const ProjectileData *projectile_data) {
        if (projectile_data == nullptr || projectile_data->is_empty()) {
            return godot::Array();
        }

        const godot::String positions_key("projectile_positions");

        if (!projectile_data->has(positions_key)) {
            return godot::Array();
        }

        const godot::Variant positions_variant = (*projectile_data)[positions_key];
        if (positions_variant.get_type() != godot::Variant::ARRAY) {
            return godot::Array();
        }

        return positions_variant;
    }

    inline float get_shockwave_radius(const ShockwaveData *shockwave_data) {
        if (shockwave_data == nullptr || shockwave_data->is_empty()) {
            return 0.0f;
        }

        static const godot::String radius_key("radius");
        if (!shockwave_data->has(radius_key)) {
            return 0.0f;
        }

        const godot::Variant radius_variant = (*shockwave_data)[radius_key];
        if (radius_variant.get_type() == godot::Variant::FLOAT || radius_variant.get_type() == godot::Variant::INT) {
            const float parsed_radius = radius_variant;
            return godot::Math::max(parsed_radius, 0.0f);
        }

        return 0.0f;
    }

} // namespace enemy_take_damage

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world.system<HitPoints, ProjectileHitTimeout, ShockwaveHitTimeout, HitReactionTimer, const ProjectileData, const EnemyBoidMovementSettings, const EnemyTakeDamageSettings, const EnemyAnimationSettings, const ShockwaveData, const PlayerPosition, const Position2D, const HitRadius>("Enemy Take Damage")
        .with(flecs::IsA, EnemyPrefab)
        .run([](flecs::iter &it) {
            // clang-format on
            std::vector<enemy_take_damage::DamageTargetAccessor> targets;
            targets.reserve(128U);
            float max_hit_radius = 0.0f;

            const ProjectileData *projectile_data = nullptr;
            const EnemyBoidMovementSettings *movement_settings = nullptr;
            const EnemyTakeDamageSettings *take_damage_settings = nullptr;
            const EnemyAnimationSettings *animation_settings = nullptr;
            const ShockwaveData *shockwave_data = nullptr;
            const PlayerPosition *player_position = nullptr;

            while (it.next()) {
                flecs::field<HitPoints> hit_points = it.field<HitPoints>(0);
                flecs::field<ProjectileHitTimeout> projectile_hit_timeouts = it.field<ProjectileHitTimeout>(1);
                flecs::field<ShockwaveHitTimeout> shockwave_hit_timeouts = it.field<ShockwaveHitTimeout>(2);
                flecs::field<HitReactionTimer> hit_reaction_timers = it.field<HitReactionTimer>(3);
                flecs::field<const ProjectileData> projectile_data_field = it.field<const ProjectileData>(4);
                flecs::field<const EnemyBoidMovementSettings> movement_settings_field = it.field<const EnemyBoidMovementSettings>(5);
                flecs::field<const EnemyTakeDamageSettings> take_damage_settings_field = it.field<const EnemyTakeDamageSettings>(6);
                flecs::field<const EnemyAnimationSettings> animation_settings_field = it.field<const EnemyAnimationSettings>(7);
                flecs::field<const ShockwaveData> shockwave_data_field = it.field<const ShockwaveData>(8);
                flecs::field<const PlayerPosition> player_position_field = it.field<const PlayerPosition>(9);
                flecs::field<const Position2D> positions = it.field<const Position2D>(10);
                flecs::field<const HitRadius> hit_radii = it.field<const HitRadius>(11);

                projectile_data = &projectile_data_field[0];
                movement_settings = &movement_settings_field[0];
                take_damage_settings = &take_damage_settings_field[0];
                animation_settings = &animation_settings_field[0];
                shockwave_data = &shockwave_data_field[0];
                player_position = &player_position_field[0];

                const std::int32_t row_count = static_cast<std::int32_t>(it.count());
                for (std::int32_t row_index = 0; row_index < row_count; ++row_index) {
                    enemy_take_damage::DamageTargetAccessor accessor{&positions[static_cast<std::size_t>(row_index)],
                                                                     &hit_points[static_cast<std::size_t>(row_index)],
                                                                     &hit_radii[static_cast<std::size_t>(row_index)],
                                                                     &projectile_hit_timeouts[static_cast<std::size_t>(row_index)],
                                                                     &shockwave_hit_timeouts[static_cast<std::size_t>(row_index)],
                                                                     &hit_reaction_timers[static_cast<std::size_t>(row_index)],
                                                                     it.entity(row_index)};
                    targets.push_back(accessor);
                    max_hit_radius = godot::Math::max(max_hit_radius, hit_radii[static_cast<std::size_t>(row_index)].value);
                }
            }

            if (take_damage_settings == nullptr) {
                return;
            }

            const float projectile_cooldown = godot::Math::max(take_damage_settings->projectile_hit_cooldown, 0.0f);
            const float shockwave_cooldown = godot::Math::max(take_damage_settings->shockwave_hit_cooldown, 0.0f);
            const float projectile_damage_amount = godot::Math::max(take_damage_settings->projectile_damage, 0.0f);
            const float shockwave_damage_amount = godot::Math::max(take_damage_settings->shockwave_damage, 0.0f);
            float hit_reaction_duration = 0.0f;
            if (animation_settings != nullptr) {
                const float configured_duration = godot::Math::max(animation_settings->hit_reaction_duration, 0.0f);
                if (configured_duration > 0.0f) {
                    hit_reaction_duration = configured_duration;
                } else {
                    hit_reaction_duration = godot::Math::max(animation_settings->animation_interval, 0.0f) * 0.5f;
                }
            }

            const godot::Array projectile_positions = enemy_take_damage::get_projectile_positions(projectile_data);
            const std::int32_t projectile_count = static_cast<std::int32_t>(projectile_positions.size());
            const bool has_projectiles = projectile_count > 0;
            const bool can_process_projectiles = has_projectiles && movement_settings != nullptr;

            const float shockwave_radius = enemy_take_damage::get_shockwave_radius(shockwave_data);
            const bool shockwave_active = shockwave_radius > 0.0f && player_position != nullptr;
            const godot::Vector2 shockwave_center = shockwave_active ? godot::Vector2(*player_position) : godot::Vector2();

            if (!can_process_projectiles && !shockwave_active) {
                return;
            }

            const std::int32_t enemy_count = static_cast<std::int32_t>(targets.size());
            if (enemy_count == 0) {
                return;
            }

            std::vector<float> accumulated_damage(static_cast<std::size_t>(enemy_count), 0.0f);

            if (can_process_projectiles) {
                const float clamped_cell_size = godot::Math::max(movement_settings->grid_cell_size, 1.0f);
                const float max_query_radius = godot::Math::max(max_hit_radius, 1.0f);
                const float normalized_span = max_query_radius / clamped_cell_size;
                const std::int32_t cell_span = static_cast<std::int32_t>(godot::Math::ceil(normalized_span));

                std::vector<enemy_spatial_hash::GridCellKey> entity_cells;
                enemy_spatial_hash::SpatialHash spatial_hash;
                enemy_spatial_hash::populate_spatial_hash(
                    enemy_count, clamped_cell_size,
                    [&targets](std::int32_t entity_index) {
                        const std::size_t target_index = static_cast<std::size_t>(entity_index);
                        return *targets[target_index].position;
                    },
                    entity_cells, spatial_hash);

                for (std::int32_t projectile_index = 0; projectile_index < projectile_count; ++projectile_index) {
                    const godot::Variant projectile_variant = projectile_positions[projectile_index];
                    if (projectile_variant.get_type() != godot::Variant::VECTOR2) {
                        continue;
                    }

                    const godot::Vector2 projectile_position = projectile_variant;
                    const enemy_spatial_hash::GridCellKey projectile_cell = enemy_spatial_hash::make_key(projectile_position, clamped_cell_size);

                    for (std::int32_t offset_x = -cell_span; offset_x <= cell_span; ++offset_x) {
                        for (std::int32_t offset_y = -cell_span; offset_y <= cell_span; ++offset_y) {
                            const enemy_spatial_hash::GridCellKey neighbor_cell{projectile_cell.x + offset_x, projectile_cell.y + offset_y};
                            const enemy_spatial_hash::SpatialHash::const_iterator found = spatial_hash.find(neighbor_cell);
                            if (found == spatial_hash.end()) {
                                continue;
                            }

                            const enemy_spatial_hash::CellOccupants &occupants = found->second;
                            const std::size_t occupant_count = occupants.size();
                            for (std::size_t occupant_index = 0; occupant_index < occupant_count; ++occupant_index) {
                                const std::int32_t enemy_index = occupants[occupant_index];
                                const godot::Vector2 enemy_position = *targets[static_cast<std::size_t>(enemy_index)].position;
                                const godot::Vector2 delta = enemy_position - projectile_position;
                                const float distance_squared = delta.length_squared();
                                const float entity_hit_radius = godot::Math::max(targets[static_cast<std::size_t>(enemy_index)].hit_radius->value, 1.0f);
                                const float entity_hit_radius_sq = entity_hit_radius * entity_hit_radius;
                                if (distance_squared <= entity_hit_radius_sq) {
                                    ProjectileHitTimeout &projectile_timeout = *targets[static_cast<std::size_t>(enemy_index)].projectile_hit_timeout;
                                    const bool can_take_projectile_hit = projectile_cooldown <= 0.0f || projectile_timeout.value >= projectile_cooldown;
                                    if (!can_take_projectile_hit) {
                                        continue;
                                    }

                                    if (projectile_damage_amount > 0.0f) {
                                        accumulated_damage[static_cast<std::size_t>(enemy_index)] += projectile_damage_amount;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            for (std::int32_t enemy_index = 0; enemy_index < enemy_count; ++enemy_index) {
                const std::size_t target_index = static_cast<std::size_t>(enemy_index);
                const float projectile_damage = accumulated_damage[target_index];
                float total_damage = 0.0f;

                if (projectile_damage > 0.0f) {
                    total_damage += projectile_damage;
                    targets[target_index].projectile_hit_timeout->value = 0.0f;
                }

                if (shockwave_active) {
                    ShockwaveHitTimeout &shockwave_timeout = *targets[target_index].shockwave_hit_timeout;
                    const bool can_take_shockwave_hit = shockwave_cooldown <= 0.0f || shockwave_timeout.value >= shockwave_cooldown;
                    if (can_take_shockwave_hit) {
                        const godot::Vector2 enemy_position = *targets[target_index].position;
                        const float enemy_radius = godot::Math::max(targets[target_index].hit_radius->value, 1.0f);
                        const float combined_radius = shockwave_radius + enemy_radius;
                        const godot::Vector2 offset = enemy_position - shockwave_center;
                        const float distance_squared = offset.length_squared();
                        if (distance_squared <= combined_radius * combined_radius && shockwave_damage_amount > 0.0f) {
                            total_damage += shockwave_damage_amount;
                            shockwave_timeout.value = 0.0f;
                        }
                    }
                }

                if (total_damage <= 0.0f) {
                    continue;
                }

                targets[target_index].hit_points->value -= total_damage;
                if (hit_reaction_duration > 0.0f) {
                    HitReactionTimer &reaction_timer = *targets[target_index].hit_reaction_timer;
                    reaction_timer.value = godot::Math::max(reaction_timer.value, hit_reaction_duration);
                }

                flecs::entity damaged_entity = targets[target_index].entity;
                godot::Dictionary signal_data;
                signal_data["enemy_type"] = enemy_type::from_entity_prefab(damaged_entity);
                signal_data["enemy_position"] = *targets[target_index].position;

                // Emit signal using Stagehand's emit_signal helper
                stagehand::EventPayload payload;
                payload.name = "enemy_took_damage";
                payload.data = signal_data;
                stagehand::emit_signal(damaged_entity, payload);
            }
        });
});
