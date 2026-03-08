#pragma once

#include <cstdint>

#include <godot_cpp/core/math.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/rendering.h"
#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"
#include "demos/surwave/cpp/utilities/enemy_constants.h"

using namespace stagehand_demos::surwave;
using Velocity2D = stagehand::physics::Velocity2D;
using RenderingCustomData = stagehand::rendering::CustomData;

namespace enemy_animation {

    inline bool try_get_previous_walk_orientation(const RenderingCustomData &custom_data,
                                                  godot::real_t base_offset,
                                                  godot::real_t walk_animation_range,
                                                  godot::real_t up_direction_frame_offset,
                                                  bool &is_facing_up) {
        const godot::real_t stored_walk_animation_range = godot::Math::floor(godot::Math::abs(custom_data.y));
        if (!godot::Math::is_equal_approx(stored_walk_animation_range, walk_animation_range)) {
            return false;
        }

        if (up_direction_frame_offset <= godot::real_t(0.0)) {
            is_facing_up = false;
            return true;
        }

        const godot::real_t stored_walk_offset = static_cast<godot::real_t>(custom_data.x) - base_offset - godot::real_t(12.0);
        const godot::real_t threshold = up_direction_frame_offset * godot::real_t(0.5);
        is_facing_up = stored_walk_offset > threshold;
        return true;
    }

    inline godot::real_t compute_entity_animation_offset_fraction(flecs::entity entity_handle, godot::real_t offset_range) {
        if (offset_range <= godot::real_t(0.0)) {
            return godot::real_t(0.0);
        }

        const std::uint64_t entity_identifier = entity_handle.id();
        std::uint32_t seeded_value = static_cast<std::uint32_t>(entity_identifier) ^ static_cast<std::uint32_t>(entity_identifier >> 32U);
        seeded_value = seeded_value * 1664525U + 1013904223U;
        const godot::real_t normalized_value = static_cast<godot::real_t>(seeded_value & 0x00FFFFFFU) / static_cast<godot::real_t>(16777215.0);
        return (normalized_value * 2.0f - 1.0f) * offset_range;
    }

} // namespace enemy_animation

// The spritesheet contains 6 columns and 12 rows, with the death animations consisting of only the
// leftmost 4 frames and walk animations using all 6 columns in their rows. Row by row layout:
// BugSmall death(down)
// BugSmall death(up)
// BugSmall walk(down)
// BugSmall walk(up)
// BugHumanoid death(down)
// BugHumanoid death(up)
// BugHumanoid walk(down)
// BugHumanoid walk(up)
// BugLarge death(down)
// BugLarge death(up)
// BugLarge walk(down)
// BugLarge walk(up)

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world
        .system<const HitPoints, const Velocity2D, const MovementSpeed, const AnimationFrameOffset, const DeathTimer, const HitReactionTimer, HFlipTimer,
                VFlipTimer, RenderingCustomData>("Enemy Animation")
        .with(flecs::IsA, EnemyPrefab)
        .kind(flecs::PostUpdate)
        .run([](flecs::iter &it) {
            // clang-format on
            const EnemyAnimationSettings *animation_settings = it.world().try_get<EnemyAnimationSettings>();
            if (animation_settings == nullptr) {
                return;
            }

            const godot::real_t animation_interval = animation_settings->animation_interval;
            const godot::real_t animation_speed = animation_interval > 0.0 ? godot::real_t(1.0) / animation_interval : godot::real_t(0.0);
            const godot::real_t animation_range = animation_settings->walk_animation_range;
            const godot::real_t death_animation_frame_count = animation_settings->death_animation_frame_count;
            const godot::real_t death_animation_range = death_animation_frame_count - godot::real_t(1.0);
            const godot::real_t frame_interval = animation_interval > 0.0 ? animation_interval : godot::real_t(0.001);
            const godot::real_t up_direction_frame_offset = animation_settings->up_direction_frame_offset;
            const godot::real_t horizontal_flip_cooldown = godot::Math::max(animation_settings->horizontal_flip_cooldown, godot::real_t(0.0));
            const godot::real_t vertical_flip_cooldown = godot::Math::max(animation_settings->vertical_flip_cooldown, godot::real_t(0.0));
            const godot::real_t nominal_movement_speed = animation_settings->nominal_movement_speed;
            const godot::real_t animation_offset_fraction_range = godot::Math::max(animation_settings->animation_offset_fraction_range, godot::real_t(0.0));

            while (it.next()) {
                flecs::field<const HitPoints> hit_points = it.field<const HitPoints>(0);
                flecs::field<const Velocity2D> velocities = it.field<const Velocity2D>(1);
                flecs::field<const MovementSpeed> movement_speeds = it.field<const MovementSpeed>(2);
                flecs::field<const AnimationFrameOffset> frame_offsets = it.field<const AnimationFrameOffset>(3);
                flecs::field<const DeathTimer> death_timer = it.field<const DeathTimer>(4);
                flecs::field<const HitReactionTimer> hit_reaction_timers = it.field<const HitReactionTimer>(5);
                flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(6);
                flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(7);
                flecs::field<RenderingCustomData> custom_data_field = it.field<RenderingCustomData>(8);

                const size_t count = it.count();
                for (size_t i = 0; i < count; ++i) {
                    const godot::Vector2 velocity_value = velocities[i];
                    const bool wants_vertical_up = velocity_value.y < godot::real_t(0.0);
                    const bool wants_horizontal_flip = velocity_value.x < godot::real_t(0.0);
                    const godot::real_t movement_speed_value = movement_speeds[i].value;
                    const godot::real_t speed_scale = nominal_movement_speed > 0.0 ? movement_speed_value / nominal_movement_speed : godot::real_t(0.0);
                    const bool has_positive_speed_scale = speed_scale > godot::real_t(0.0);
                    const godot::real_t scaled_animation_speed = has_positive_speed_scale ? animation_speed * speed_scale : godot::real_t(0.0);
                    const godot::real_t scaled_frame_interval = has_positive_speed_scale ? frame_interval / speed_scale : frame_interval;

                    const godot::real_t death_timer_value = death_timer[i].value;
                    const godot::real_t hit_points_value = hit_points[i].value;
                    const godot::real_t base_offset = frame_offsets[i].value;
                    const bool has_invulnerable_hit_points = hit_points_value >= kEnemyDeathInvulnerableHitPoints;
                    const bool is_dying_state = death_timer_value > godot::real_t(0.0) || has_invulnerable_hit_points;
                    const godot::real_t hit_reaction_time_remaining = hit_reaction_timers[i].value;
                    const bool is_hit_reacting = !is_dying_state && hit_reaction_time_remaining > godot::real_t(0.0);

                    RenderingCustomData &custom_data = custom_data_field[i];
                    HFlipTimer &horizontal_timer = horizontal_flip_timers[i];
                    VFlipTimer &vertical_timer = vertical_flip_timers[i];

                    uint32_t animation_flags = static_cast<uint32_t>(custom_data.w);
                    bool current_horizontal_flip = (animation_flags & 1U) != 0U;

                    if (horizontal_flip_cooldown <= godot::real_t(0.0)) {
                        current_horizontal_flip = wants_horizontal_flip;
                        horizontal_timer.value = godot::real_t(0.0);
                    } else if (wants_horizontal_flip != current_horizontal_flip && horizontal_timer.value >= horizontal_flip_cooldown) {
                        current_horizontal_flip = wants_horizontal_flip;
                        horizontal_timer.value = godot::real_t(0.0);
                    }

                    bool resolved_vertical_up = wants_vertical_up;
                    if (!is_dying_state) {
                        bool previous_vertical_up = wants_vertical_up;
                        if (!enemy_animation::try_get_previous_walk_orientation(custom_data, base_offset, animation_range, up_direction_frame_offset,
                                                                                previous_vertical_up)) {
                            previous_vertical_up = wants_vertical_up;
                        }

                        resolved_vertical_up = previous_vertical_up;
                        if (vertical_flip_cooldown <= godot::real_t(0.0)) {
                            resolved_vertical_up = wants_vertical_up;
                            vertical_timer.value = godot::real_t(0.0);
                        } else if (wants_vertical_up != previous_vertical_up && vertical_timer.value >= vertical_flip_cooldown) {
                            resolved_vertical_up = wants_vertical_up;
                            vertical_timer.value = godot::real_t(0.0);
                        }
                    }

                    const godot::real_t walk_directional_offset =
                        (resolved_vertical_up ? godot::real_t(1.0) : godot::real_t(0.0)) * up_direction_frame_offset + godot::real_t(12.0);
                    const godot::real_t death_directional_offset = (resolved_vertical_up ? godot::real_t(1.0) : godot::real_t(0.0)) * up_direction_frame_offset;

                    if (is_hit_reacting) {
                        custom_data.x = static_cast<float>(base_offset + death_directional_offset);
                        custom_data.y = 0.0f;
                        custom_data.z = 0.0f;
                    } else if (is_dying_state) {
                        const godot::real_t timer_for_frame_selection = godot::Math::max(death_timer_value, godot::real_t(0.0));
                        const godot::real_t frames_remaining = godot::Math::ceil(timer_for_frame_selection / scaled_frame_interval);
                        const godot::real_t frames_elapsed = death_animation_frame_count - frames_remaining;
                        const godot::real_t clamped_frames_elapsed = godot::Math::clamp(frames_elapsed, godot::real_t(0.0), death_animation_range);
                        const godot::real_t clamped_frame_offset = godot::Math::floor(clamped_frames_elapsed);
                        custom_data.x = static_cast<float>(base_offset + death_directional_offset + clamped_frame_offset);
                        custom_data.y = 0.0f;
                        custom_data.z = 0.0f;
                    } else {
                        godot::real_t animation_time_offset_fraction = godot::real_t(0.0);
                        if (animation_offset_fraction_range > godot::real_t(0.0)) {
                            const flecs::entity entity_handle = it.entity(static_cast<std::int32_t>(i));
                            animation_time_offset_fraction =
                                enemy_animation::compute_entity_animation_offset_fraction(entity_handle, animation_offset_fraction_range);
                        }
                        const godot::real_t encoded_animation_range = animation_time_offset_fraction < godot::real_t(0.0) ? -animation_range : animation_range;
                        custom_data.x = static_cast<float>(base_offset + walk_directional_offset);
                        custom_data.y = static_cast<float>(encoded_animation_range + animation_time_offset_fraction);
                        custom_data.z = static_cast<float>(scaled_animation_speed);
                    }

                    animation_flags = current_horizontal_flip ? 1U : 0U;
                    custom_data.w = static_cast<float>(animation_flags);
                }
            }
        });
});
