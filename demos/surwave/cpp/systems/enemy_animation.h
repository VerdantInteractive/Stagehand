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

    inline bool try_get_previous_walk_orientation(
        const RenderingCustomData &custom_data, float base_offset, float walk_animation_range, float up_direction_frame_offset, bool &is_facing_up) {
        const float stored_walk_animation_range = godot::Math::floor(godot::Math::abs(custom_data.y));
        if (!godot::Math::is_equal_approx(stored_walk_animation_range, walk_animation_range)) {
            return false;
        }

        if (up_direction_frame_offset <= 0.0f) {
            is_facing_up = false;
            return true;
        }

        const float stored_walk_offset = static_cast<float>(custom_data.x) - base_offset - 12.0f;
        const float threshold = up_direction_frame_offset * 0.5f;
        is_facing_up = stored_walk_offset > threshold;
        return true;
    }

    inline float compute_entity_animation_offset_fraction(flecs::entity entity_handle, float offset_range) {
        if (offset_range <= 0.0f) {
            return 0.0f;
        }

        const std::uint64_t entity_identifier = entity_handle.id();
        std::uint32_t seeded_value = static_cast<std::uint32_t>(entity_identifier) ^ static_cast<std::uint32_t>(entity_identifier >> 32U);
        seeded_value = seeded_value * 1664525U + 1013904223U;
        const float normalized_value = static_cast<float>(seeded_value & 0x00FFFFFFU) / static_cast<float>(16777215.0);
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
        .system<HFlipTimer, VFlipTimer, RenderingCustomData, const EnemyAnimationSettings, const HitPoints, const Velocity2D, const MovementSpeed, const AnimationFrameOffset, const DeathTimer, const HitReactionTimer>("Enemy Animation")
        .with(flecs::IsA, EnemyPrefab)
        .kind(flecs::PostUpdate)
        .run([](flecs::iter &it) {
            // clang-format on
            while (it.next()) {
                flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(0);
                flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(1);
                flecs::field<RenderingCustomData> custom_data_field = it.field<RenderingCustomData>(2);
                flecs::field<const EnemyAnimationSettings> animation_settings_field = it.field<const EnemyAnimationSettings>(3);
                flecs::field<const HitPoints> hit_points = it.field<const HitPoints>(4);
                flecs::field<const Velocity2D> velocities = it.field<const Velocity2D>(5);
                flecs::field<const MovementSpeed> movement_speeds = it.field<const MovementSpeed>(6);
                flecs::field<const AnimationFrameOffset> frame_offsets = it.field<const AnimationFrameOffset>(7);
                flecs::field<const DeathTimer> death_timer = it.field<const DeathTimer>(8);
                flecs::field<const HitReactionTimer> hit_reaction_timers = it.field<const HitReactionTimer>(9);

                const EnemyAnimationSettings *animation_settings = &animation_settings_field[0];
                const float animation_interval = animation_settings->animation_interval;
                const float animation_speed = animation_interval > 0.0f ? 1.0f / animation_interval : 0.0f;
                const float animation_range = animation_settings->walk_animation_range;
                const float death_animation_frame_count = animation_settings->death_animation_frame_count;
                const float death_animation_range = death_animation_frame_count - 1.0f;
                const float frame_interval = animation_interval > 0.0f ? animation_interval : 0.001f;
                const float up_direction_frame_offset = animation_settings->up_direction_frame_offset;
                const float horizontal_flip_cooldown = godot::Math::max(animation_settings->horizontal_flip_cooldown, 0.0f);
                const float vertical_flip_cooldown = godot::Math::max(animation_settings->vertical_flip_cooldown, 0.0f);
                const float nominal_movement_speed = animation_settings->nominal_movement_speed;
                const float animation_offset_fraction_range = godot::Math::max(animation_settings->animation_offset_fraction_range, 0.0f);

                const size_t count = it.count();
                for (size_t i = 0; i < count; ++i) {
                    const godot::Vector2 velocity_value = velocities[i];
                    const bool wants_vertical_up = velocity_value.y < 0.0f;
                    const bool wants_horizontal_flip = velocity_value.x < 0.0f;
                    const float movement_speed_value = movement_speeds[i].value;
                    const float speed_scale = nominal_movement_speed > 0.0f ? movement_speed_value / nominal_movement_speed : 0.0f;
                    const bool has_positive_speed_scale = speed_scale > 0.0f;
                    const float scaled_animation_speed = has_positive_speed_scale ? animation_speed * speed_scale : 0.0f;
                    const float scaled_frame_interval = has_positive_speed_scale ? frame_interval / speed_scale : frame_interval;

                    const float death_timer_value = death_timer[i].value;
                    const float hit_points_value = hit_points[i].value;
                    const float base_offset = frame_offsets[i].value;
                    const bool has_invulnerable_hit_points = hit_points_value >= kEnemyDeathInvulnerableHitPoints;
                    const bool is_dying_state = death_timer_value > 0.0f || has_invulnerable_hit_points;
                    const float hit_reaction_time_remaining = hit_reaction_timers[i].value;
                    const bool is_hit_reacting = !is_dying_state && hit_reaction_time_remaining > 0.0f;

                    RenderingCustomData &custom_data = custom_data_field[i];
                    HFlipTimer &horizontal_timer = horizontal_flip_timers[i];
                    VFlipTimer &vertical_timer = vertical_flip_timers[i];

                    uint32_t animation_flags = static_cast<uint32_t>(custom_data.w);
                    bool current_horizontal_flip = (animation_flags & 1U) != 0U;

                    if (horizontal_flip_cooldown <= 0.0f) {
                        current_horizontal_flip = wants_horizontal_flip;
                        horizontal_timer.value = 0.0f;
                    } else if (wants_horizontal_flip != current_horizontal_flip && horizontal_timer.value >= horizontal_flip_cooldown) {
                        current_horizontal_flip = wants_horizontal_flip;
                        horizontal_timer.value = 0.0f;
                    }

                    bool resolved_vertical_up = wants_vertical_up;
                    if (!is_dying_state) {
                        bool previous_vertical_up = wants_vertical_up;
                        if (!enemy_animation::try_get_previous_walk_orientation(custom_data, base_offset, animation_range, up_direction_frame_offset,
                                                                                previous_vertical_up)) {
                            previous_vertical_up = wants_vertical_up;
                        }

                        resolved_vertical_up = previous_vertical_up;
                        if (vertical_flip_cooldown <= 0.0f) {
                            resolved_vertical_up = wants_vertical_up;
                            vertical_timer.value = 0.0f;
                        } else if (wants_vertical_up != previous_vertical_up && vertical_timer.value >= vertical_flip_cooldown) {
                            resolved_vertical_up = wants_vertical_up;
                            vertical_timer.value = 0.0f;
                        }
                    }

                    const float walk_directional_offset = (resolved_vertical_up ? 1.0f : 0.0f) * up_direction_frame_offset + 12.0f;
                    const float death_directional_offset = (resolved_vertical_up ? 1.0f : 0.0f) * up_direction_frame_offset;

                    if (is_hit_reacting) {
                        custom_data.x = static_cast<float>(base_offset + death_directional_offset);
                        custom_data.y = 0.0f;
                        custom_data.z = 0.0f;
                    } else if (is_dying_state) {
                        const float timer_for_frame_selection = godot::Math::max(death_timer_value, 0.0f);
                        const float frames_remaining = godot::Math::ceil(timer_for_frame_selection / scaled_frame_interval);
                        const float frames_elapsed = death_animation_frame_count - frames_remaining;
                        const float clamped_frames_elapsed = godot::Math::clamp(frames_elapsed, 0.0f, death_animation_range);
                        const float clamped_frame_offset = godot::Math::floor(clamped_frames_elapsed);
                        custom_data.x = static_cast<float>(base_offset + death_directional_offset + clamped_frame_offset);
                        custom_data.y = 0.0f;
                        custom_data.z = 0.0f;
                    } else {
                        float animation_time_offset_fraction = 0.0f;
                        if (animation_offset_fraction_range > 0.0f) {
                            const flecs::entity entity_handle = it.entity(static_cast<std::int32_t>(i));
                            animation_time_offset_fraction =
                                enemy_animation::compute_entity_animation_offset_fraction(entity_handle, animation_offset_fraction_range);
                        }
                        const float encoded_animation_range = animation_time_offset_fraction < 0.0f ? -animation_range : animation_range;
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
