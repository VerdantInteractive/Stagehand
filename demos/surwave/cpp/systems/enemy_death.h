#pragma once

#include <cstddef>
#include <cstdint>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "stagehand/ecs/components/event_payload.h"
#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"
#include "stagehand/utilities/emit_signal.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"
#include "demos/surwave/cpp/utilities/enemy_constants.h"
#include "demos/surwave/cpp/utilities/enemy_type.h"

using namespace stagehand_demos::surwave;
using Position2D = stagehand::transform::Position2D;
using Velocity2D = stagehand::physics::Velocity2D;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world.system<const Position2D, HitPoints, DeathTimer, MeleeDamage, MovementSpeed, Velocity2D>("Enemy Death")
        .with(flecs::IsA, EnemyPrefab)
        .kind(flecs::OnValidate)
        .run([](flecs::iter &it) {
            // clang-format on
            const EnemyAnimationSettings *animation_settings = it.world().try_get<EnemyAnimationSettings>();
            if (animation_settings == nullptr) {
                return;
            }

            const godot::real_t death_animation_duration = animation_settings->animation_interval * animation_settings->death_animation_frame_count;
            const godot::real_t invulnerable_hit_points = kEnemyDeathInvulnerableHitPoints;

            while (it.next()) {
                flecs::field<const Position2D> positions = it.field<const Position2D>(0);
                flecs::field<HitPoints> hit_points = it.field<HitPoints>(1);
                flecs::field<DeathTimer> death_timer = it.field<DeathTimer>(2);
                flecs::field<MeleeDamage> melee_damage = it.field<MeleeDamage>(3);
                flecs::field<MovementSpeed> movement_speed = it.field<MovementSpeed>(4);
                flecs::field<Velocity2D> velocities = it.field<Velocity2D>(5);

                for (auto entity_index : it) {
                    if (hit_points[entity_index].value > godot::real_t(0.0)) {
                        continue;
                    }

                    flecs::entity entity = it.entity(entity_index);
                    godot::Dictionary signal_data;
                    signal_data["enemy_type"] = enemy_type::from_entity_prefab(entity);
                    signal_data["enemy_position"] = positions[entity_index];

                    // Emit signal using Stagehand's emit_signal helper
                    stagehand::EventPayload payload;
                    payload.name = "enemy_died";
                    payload.data = signal_data;
                    stagehand::emit_signal(it, entity_index, payload);

                    hit_points[entity_index].value = invulnerable_hit_points;
                    death_timer[entity_index].value = death_animation_duration;
                    melee_damage[entity_index].value = godot::real_t(0.0);
                    movement_speed[entity_index].value = godot::real_t(0.0);
                    velocities[entity_index] = godot::Vector2(0.0f, 0.0f);
                }
            }
        });
});
