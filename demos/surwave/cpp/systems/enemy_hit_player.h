#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "stagehand/ecs/components/event_payload.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"
#include "stagehand/utilities/emit_signal.h"

#include "demos/surwave/cpp/components/enemy.h"
#include "demos/surwave/cpp/components/singletons.h"
#include "demos/surwave/cpp/prefabs/enemy.h"

using namespace stagehand_demos::surwave;
using Position2D = stagehand::transform::Position2D;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    world.system<const PlayerDamageCooldown, const PlayerPosition, const PlayerTakeDamageSettings, const Position2D, const MeleeDamage>("Enemy Hit Player")
        .with(flecs::IsA, EnemyPrefab)
        .run([](flecs::iter &it) {
            // clang-format on
            while (it.next()) {
                flecs::field<const PlayerDamageCooldown> player_damage_cooldown_field = it.field<const PlayerDamageCooldown>(0);
                flecs::field<const PlayerPosition> player_position_field = it.field<const PlayerPosition>(1);
                flecs::field<const PlayerTakeDamageSettings> damage_settings_field = it.field<const PlayerTakeDamageSettings>(2);
                flecs::field<const Position2D> positions = it.field<const Position2D>(3);
                flecs::field<const MeleeDamage> melee_damages = it.field<const MeleeDamage>(4);

                PlayerDamageCooldown *player_damage_cooldown = it.world().try_get_mut<PlayerDamageCooldown>();
                if (player_damage_cooldown == nullptr) {
                    return;
                }
                const PlayerPosition *player_position = &player_position_field[0];
                const PlayerTakeDamageSettings *damage_settings = &damage_settings_field[0];

                const float cooldown = godot::Math::max(damage_settings->damage_cooldown, 0.0f);
                const bool can_take_damage = cooldown <= 0.0f || player_damage_cooldown->value >= cooldown;
                if (!can_take_damage) {
                    return;
                }

                const float player_hit_radius = godot::Math::max(damage_settings->player_hit_radius, 1.0f);
                const godot::Vector2 player_position_value = *player_position;
                for (auto entity_index : it) {
                    const godot::Vector2 enemy_position = positions[entity_index];
                    const godot::Vector2 delta = player_position_value - enemy_position;
                    const float distance_squared = delta.length_squared();
                    const float contact_radius = player_hit_radius;
                    const float contact_radius_squared = contact_radius * contact_radius;
                    if (distance_squared > contact_radius_squared) {
                        continue;
                    }

                    const float damage_amount = godot::Math::max(melee_damages[entity_index].value, 0.0f);
                    if (damage_amount <= 0.0f) {
                        continue;
                    }

                    flecs::entity damaging_enemy = it.entity(entity_index);
                    godot::Dictionary signal_data;
                    signal_data["damage_amount"] = damage_amount;

                    // Emit signal using Stagehand's emit_signal helper
                    stagehand::EventPayload payload;
                    payload.name = "enemy_hit_player";
                    payload.data = signal_data;
                    stagehand::emit_signal(it, entity_index, payload);

                    player_damage_cooldown->value = 0.0f;
                    return;
                }
            }
        });
});
