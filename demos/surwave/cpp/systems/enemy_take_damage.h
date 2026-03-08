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
        Position2D *position;
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

    inline godot::real_t get_shockwave_radius(const ShockwaveData *shockwave_data) {
        if (shockwave_data == nullptr || shockwave_data->is_empty()) {
            return godot::real_t(0.0);
        }

        static const godot::String radius_key("radius");
        if (!shockwave_data->has(radius_key)) {
            return godot::real_t(0.0);
        }

        const godot::Variant radius_variant = (*shockwave_data)[radius_key];
        if (radius_variant.get_type() == godot::Variant::FLOAT || radius_variant.get_type() == godot::Variant::INT) {
            const godot::real_t parsed_radius = radius_variant;
            return godot::Math::max(parsed_radius, godot::real_t(0.0));
        }

        return godot::real_t(0.0);
    }

} // namespace enemy_take_damage

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world.system<Position2D, HitPoints, const HitRadius, ProjectileHitTimeout, ShockwaveHitTimeout, HitReactionTimer>("Enemy Take Damage")
        .with(flecs::IsA, EnemyPrefab)
        .run([](flecs::iter &it) {
            // clang-format on
            flecs::world stage_world = it.world();
            const ProjectileData *projectile_data = stage_world.try_get<ProjectileData>();
            const EnemyBoidMovementSettings *movement_settings = stage_world.try_get<EnemyBoidMovementSettings>();
            const EnemyTakeDamageSettings *take_damage_settings = stage_world.try_get<EnemyTakeDamageSettings>();
            const EnemyAnimationSettings *animation_settings = stage_world.try_get<EnemyAnimationSettings>();
            const ShockwaveData *shockwave_data = stage_world.try_get<ShockwaveData>();
            const PlayerPosition *player_position = stage_world.try_get<PlayerPosition>();
            if (take_damage_settings == nullptr) {
                return;
            }

            const godot::real_t projectile_cooldown = godot::Math::max(take_damage_settings->projectile_hit_cooldown, godot::real_t(0.0));
            const godot::real_t shockwave_cooldown = godot::Math::max(take_damage_settings->shockwave_hit_cooldown, godot::real_t(0.0));
            const godot::real_t projectile_damage_amount = godot::Math::max(take_damage_settings->projectile_damage, godot::real_t(0.0));
            const godot::real_t shockwave_damage_amount = godot::Math::max(take_damage_settings->shockwave_damage, godot::real_t(0.0));
            godot::real_t hit_reaction_duration = godot::real_t(0.0);
            if (animation_settings != nullptr) {
                const godot::real_t configured_duration = godot::Math::max(animation_settings->hit_reaction_duration, godot::real_t(0.0));
                if (configured_duration > godot::real_t(0.0)) {
                    hit_reaction_duration = configured_duration;
                } else {
                    hit_reaction_duration = godot::Math::max(animation_settings->animation_interval, godot::real_t(0.0)) * godot::real_t(0.5);
                }
            }

            const godot::Array projectile_positions = enemy_take_damage::get_projectile_positions(projectile_data);
            const std::int32_t projectile_count = static_cast<std::int32_t>(projectile_positions.size());
            const bool has_projectiles = projectile_count > 0;
            const bool can_process_projectiles = has_projectiles && movement_settings != nullptr;

            const godot::real_t shockwave_radius = enemy_take_damage::get_shockwave_radius(shockwave_data);
            const bool shockwave_active = shockwave_radius > godot::real_t(0.0) && player_position != nullptr;
            const godot::Vector2 shockwave_center = shockwave_active ? godot::Vector2(*player_position) : godot::Vector2();

            if (!can_process_projectiles && !shockwave_active) {
                return;
            }

            std::vector<enemy_take_damage::DamageTargetAccessor> targets;
            targets.reserve(128U);
            godot::real_t max_hit_radius = godot::real_t(0.0);

            while (it.next()) {
                flecs::field<Position2D> positions = it.field<Position2D>(0);
                flecs::field<HitPoints> hit_points = it.field<HitPoints>(1);
                flecs::field<const HitRadius> hit_radii = it.field<const HitRadius>(2);
                flecs::field<ProjectileHitTimeout> projectile_hit_timeouts = it.field<ProjectileHitTimeout>(3);
                flecs::field<ShockwaveHitTimeout> shockwave_hit_timeouts = it.field<ShockwaveHitTimeout>(4);
                flecs::field<HitReactionTimer> hit_reaction_timers = it.field<HitReactionTimer>(5);
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

            const std::int32_t enemy_count = static_cast<std::int32_t>(targets.size());
            if (enemy_count == 0) {
                return;
            }

            std::vector<godot::real_t> accumulated_damage(static_cast<std::size_t>(enemy_count), godot::real_t(0.0));

            if (can_process_projectiles) {
                const godot::real_t clamped_cell_size = godot::Math::max(movement_settings->grid_cell_size, godot::real_t(1.0));
                const godot::real_t max_query_radius = godot::Math::max(max_hit_radius, godot::real_t(1.0));
                const godot::real_t normalized_span = max_query_radius / clamped_cell_size;
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
                                const godot::real_t distance_squared = delta.length_squared();
                                const godot::real_t entity_hit_radius =
                                    godot::Math::max(targets[static_cast<std::size_t>(enemy_index)].hit_radius->value, godot::real_t(1.0));
                                const godot::real_t entity_hit_radius_sq = entity_hit_radius * entity_hit_radius;
                                if (distance_squared <= entity_hit_radius_sq) {
                                    ProjectileHitTimeout &projectile_timeout = *targets[static_cast<std::size_t>(enemy_index)].projectile_hit_timeout;
                                    const bool can_take_projectile_hit =
                                        projectile_cooldown <= godot::real_t(0.0) || projectile_timeout.value >= projectile_cooldown;
                                    if (!can_take_projectile_hit) {
                                        continue;
                                    }

                                    if (projectile_damage_amount > godot::real_t(0.0)) {
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
                const godot::real_t projectile_damage = accumulated_damage[target_index];
                godot::real_t total_damage = godot::real_t(0.0);

                if (projectile_damage > godot::real_t(0.0)) {
                    total_damage += projectile_damage;
                    targets[target_index].projectile_hit_timeout->value = godot::real_t(0.0);
                }

                if (shockwave_active) {
                    ShockwaveHitTimeout &shockwave_timeout = *targets[target_index].shockwave_hit_timeout;
                    const bool can_take_shockwave_hit = shockwave_cooldown <= godot::real_t(0.0) || shockwave_timeout.value >= shockwave_cooldown;
                    if (can_take_shockwave_hit) {
                        const godot::Vector2 enemy_position = *targets[target_index].position;
                        const godot::real_t enemy_radius = godot::Math::max(targets[target_index].hit_radius->value, godot::real_t(1.0));
                        const godot::real_t combined_radius = shockwave_radius + enemy_radius;
                        const godot::Vector2 offset = enemy_position - shockwave_center;
                        const godot::real_t distance_squared = offset.length_squared();
                        if (distance_squared <= combined_radius * combined_radius && shockwave_damage_amount > godot::real_t(0.0)) {
                            total_damage += shockwave_damage_amount;
                            shockwave_timeout.value = godot::real_t(0.0);
                        }
                    }
                }

                if (total_damage <= godot::real_t(0.0)) {
                    continue;
                }

                targets[target_index].hit_points->value -= total_damage;
                if (hit_reaction_duration > godot::real_t(0.0)) {
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
