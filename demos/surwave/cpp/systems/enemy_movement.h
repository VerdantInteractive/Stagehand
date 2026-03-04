#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"
#include "demos/surwave/cpp/utilities/enemy_kd_tree.h"

using namespace stagehand_demos::surwave;
using Position2D = stagehand::transform::Position2D;
using Velocity2D = stagehand::physics::Velocity2D;

namespace enemy_movement {

    struct BoidAccessor {
        flecs::entity_t entity_id;
        godot::Vector2 *position;
        godot::Vector2 *velocity;
        godot::real_t max_speed;
    };

    struct KdTreeCache {
        enemy_kd_tree::KdTree2D tree;
        std::vector<godot::Vector2> cached_positions;
        std::vector<flecs::entity_t> cached_entity_ids;
        std::size_t cached_count = 0;
        std::uint32_t frames_since_rebuild = 0;
    };

    inline KdTreeCache &get_kd_tree_cache() {
        static KdTreeCache cache;
        return cache;
    }

    inline godot::Vector2 steer_towards(const godot::Vector2 &desired_direction, const godot::Vector2 &current_velocity, godot::real_t max_speed) {
        const godot::real_t desired_length_sq = desired_direction.length_squared();
        if (desired_length_sq == 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        const godot::Vector2 desired_velocity = (desired_direction / godot::Math::sqrt(desired_length_sq)) * max_speed;
        return desired_velocity - current_velocity;
    }

    inline godot::Vector2 limit_vector_squared(const godot::Vector2 &value, godot::real_t max_length_sq) {
        if (max_length_sq <= 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        const godot::real_t current_length_sq = value.length_squared();
        if (current_length_sq <= max_length_sq) {
            return value;
        }

        if (current_length_sq == 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        return value * (godot::Math::sqrt(max_length_sq / current_length_sq));
    }

} // namespace enemy_movement

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    world.system<Position2D, Velocity2D, const MovementSpeed, const DeathTimer>("Enemy Movement").with(flecs::IsA, EnemyPrefab).run([](flecs::iter &it) {
        flecs::world stage_world = it.world();
        const PlayerPosition *player_position = stage_world.try_get<PlayerPosition>();
        const EnemyBoidMovementSettings *movement_settings = stage_world.try_get<EnemyBoidMovementSettings>();

        if (player_position == nullptr || movement_settings == nullptr) {
            return;
        }

        enemy_movement::KdTreeCache &kd_cache = enemy_movement::get_kd_tree_cache();

        std::vector<enemy_movement::BoidAccessor> boids;
        const std::size_t suggested_capacity = kd_cache.cached_count > 0 ? kd_cache.cached_count : static_cast<std::size_t>(256U);
        boids.reserve(suggested_capacity);

        godot::real_t delta_time = 0.0f;
        bool delta_time_initialized = false;

        while (it.next()) {
            if (!delta_time_initialized) {
                delta_time = it.delta_time();
                delta_time_initialized = true;
            }

            flecs::field<Position2D> positions = it.field<Position2D>(0);
            flecs::field<Velocity2D> velocities = it.field<Velocity2D>(1);
            flecs::field<const MovementSpeed> movement_speeds = it.field<const MovementSpeed>(2);
            flecs::field<const DeathTimer> death_timers = it.field<const DeathTimer>(3);

            const godot::real_t max_speed_multiplier = movement_settings->max_speed_multiplier;

            for (size_t row_index = 0; row_index < it.count(); ++row_index) {
                if (death_timers[row_index].value > 0.0f) {
                    continue;
                }
                const flecs::entity current_entity = it.entity(static_cast<int32_t>(row_index));
                enemy_movement::BoidAccessor accessor{current_entity.id(), &positions[row_index], &velocities[row_index],
                                                      godot::Math::max(movement_speeds[row_index].value * max_speed_multiplier, 1.0f)};
                boids.push_back(accessor);
            }
        }

        const size_t enemy_count = boids.size();
        if (enemy_count == 0) {
            return;
        }

        std::sort(boids.begin(), boids.end(),
                  [](const enemy_movement::BoidAccessor &lhs, const enemy_movement::BoidAccessor &rhs) { return lhs.entity_id < rhs.entity_id; });

        const godot::real_t player_engage_radius = movement_settings->player_engage_radius;
        const godot::real_t player_engage_radius_sq = player_engage_radius * player_engage_radius;
        const godot::real_t neighbor_radius_sq = movement_settings->neighbor_radius * movement_settings->neighbor_radius;
        const godot::real_t separation_radius_sq = movement_settings->separation_radius * movement_settings->separation_radius;
        const godot::real_t max_force = movement_settings->max_force;
        const std::int32_t neighbor_sample_limit =
            movement_settings->max_neighbor_sample_count <= godot::real_t(0.0) ? -1 : static_cast<std::int32_t>(movement_settings->max_neighbor_sample_count);

        struct BoidPositionAccessor {
            const std::vector<enemy_movement::BoidAccessor> *boid_array;

            godot::Vector2 operator()(std::int32_t entity_index) const {
                const std::size_t offset = static_cast<std::size_t>(entity_index);
                return *(*boid_array)[offset].position;
            }
        };

        BoidPositionAccessor position_accessor{&boids};

        const godot::real_t rebuild_distance = godot::Math::max(movement_settings->kd_tree_rebuild_distance, godot::real_t(0.0));
        const godot::real_t rebuild_distance_sq = rebuild_distance * rebuild_distance;
        const std::uint32_t stale_frame_limit =
            movement_settings->kd_tree_max_stale_frames <= godot::real_t(0.0) ? 0U : static_cast<std::uint32_t>(movement_settings->kd_tree_max_stale_frames);

        bool force_rebuild = kd_cache.tree.empty();
        force_rebuild = force_rebuild || kd_cache.cached_count != enemy_count;
        force_rebuild = force_rebuild || kd_cache.cached_positions.size() != enemy_count;
        force_rebuild = force_rebuild || kd_cache.cached_entity_ids.size() != enemy_count;

        if (!force_rebuild && enemy_count > 0) {
            for (size_t index = 0; index < enemy_count; ++index) {
                if (kd_cache.cached_entity_ids[index] != boids[index].entity_id) {
                    force_rebuild = true;
                    break;
                }
            }
        }

        if (!force_rebuild && rebuild_distance_sq > godot::real_t(0.0) && !kd_cache.cached_positions.empty()) {
            godot::real_t max_displacement_sq = godot::real_t(0.0);
            for (size_t index = 0; index < enemy_count; ++index) {
                const godot::Vector2 delta = *boids[index].position - kd_cache.cached_positions[index];
                max_displacement_sq = godot::Math::max(max_displacement_sq, delta.length_squared());
                if (max_displacement_sq >= rebuild_distance_sq) {
                    force_rebuild = true;
                    break;
                }
            }
        }

        if (!force_rebuild && stale_frame_limit > 0U && kd_cache.frames_since_rebuild >= stale_frame_limit) {
            force_rebuild = true;
        }

        if (force_rebuild) {
            kd_cache.tree.build(static_cast<std::int32_t>(enemy_count), position_accessor);
            kd_cache.frames_since_rebuild = 0;
        } else {
            kd_cache.tree.refresh_points(static_cast<std::int32_t>(enemy_count), position_accessor);
            kd_cache.frames_since_rebuild += 1;
        }

        kd_cache.cached_positions.resize(enemy_count);
        kd_cache.cached_entity_ids.resize(enemy_count);
        for (size_t index = 0; index < enemy_count; ++index) {
            kd_cache.cached_positions[index] = *boids[index].position;
            kd_cache.cached_entity_ids[index] = boids[index].entity_id;
        }
        kd_cache.cached_count = enemy_count;

        for (size_t entity_index = 0; entity_index < enemy_count; ++entity_index) {
            const godot::Vector2 position_value = *boids[entity_index].position;
            const godot::Vector2 current_velocity = *boids[entity_index].velocity;
            const godot::real_t max_speed = boids[entity_index].max_speed;

            godot::Vector2 separation_sum = godot::Vector2(0.0f, 0.0f);
            std::int32_t separation_count = 0;

            struct NeighborAccumulator {
                const std::vector<enemy_movement::BoidAccessor> *boid_array;
                const godot::Vector2 &origin;
                std::size_t self_index;
                godot::real_t separation_radius_squared;
                godot::Vector2 &separation_sum_ref;
                std::int32_t &separation_count_ref;

                void operator()(std::int32_t other_index, const godot::Vector2 &other_position, godot::real_t distance_squared) const {
                    const std::size_t other_offset = static_cast<std::size_t>(other_index);
                    if (other_offset == self_index || distance_squared == 0.0f) {
                        return;
                    }

                    if (distance_squared < separation_radius_squared) {
                        const godot::Vector2 offset = other_position - origin;
                        separation_sum_ref -= offset / distance_squared;
                        separation_count_ref += 1;
                    }
                }
            };

            NeighborAccumulator accumulator{&boids, position_value, entity_index, separation_radius_sq, separation_sum, separation_count};

            kd_cache.tree.radius_query(position_value, separation_radius_sq, accumulator, neighbor_sample_limit);

            godot::Vector2 separation_force = godot::Vector2(0.0f, 0.0f);
            if (separation_count > 0) {
                const godot::Vector2 average_push = separation_sum / static_cast<godot::real_t>(separation_count);

                // Add noise to break up rows/columns
                const double noise_intensity = static_cast<double>(movement_settings->separation_noise_intensity);
                const godot::Vector2 noise(static_cast<float>(godot::UtilityFunctions::randf_range(-noise_intensity, noise_intensity)),
                                           static_cast<float>(godot::UtilityFunctions::randf_range(-noise_intensity, noise_intensity)));

                separation_force = enemy_movement::steer_towards(average_push + noise, current_velocity, max_speed);
            }

            const godot::Vector2 player_offset = *player_position - position_value;
            godot::Vector2 player_force = enemy_movement::steer_towards(player_offset, current_velocity, max_speed);
            const godot::real_t distance_to_player_sq = player_offset.length_squared();
            if (distance_to_player_sq < player_engage_radius_sq && player_engage_radius_sq > 0.0f) {
                const godot::real_t distance_to_player = godot::Math::sqrt(distance_to_player_sq);
                const godot::real_t normalized_distance = distance_to_player / player_engage_radius;
                const godot::real_t slowdown_factor = godot::Math::clamp(normalized_distance, 0.2f, 1.0f);
                player_force *= slowdown_factor;
            }

            godot::Vector2 acceleration = godot::Vector2(0.0f, 0.0f);
            acceleration += separation_force * movement_settings->separation_weight;
            acceleration += player_force * movement_settings->player_attraction_weight;

            acceleration = enemy_movement::limit_vector_squared(acceleration, max_force * max_force);

            godot::Vector2 new_velocity = current_velocity + acceleration * delta_time;
            new_velocity = enemy_movement::limit_vector_squared(new_velocity, max_speed * max_speed);

            *boids[entity_index].velocity = new_velocity;
        }
    });
});
