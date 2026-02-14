#pragma once

#include <godot_cpp/variant/typed_dictionary.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    struct WorldConfiguration {
        godot::TypedDictionary<godot::String, godot::Variant> value;
    };

    inline stagehand::Registry register_world_configuration([](flecs::world& world) {
        world.component<WorldConfiguration>().add(flecs::Singleton);

        stagehand::get_component_getters()["WorldConfiguration"] = [](const flecs::world& world, flecs::entity_t entity_id) -> godot::Variant {
            if (entity_id != 0) {
                godot::UtilityFunctions::push_warning(
                    "WorldConfiguration is a singleton component and can only be accessed with entity_id 0. Got entity_id: " +
                    godot::String::num_uint64(entity_id)
                );
                return godot::Variant(godot::TypedDictionary<godot::String, godot::Variant>());
            }

            const WorldConfiguration* data = world.try_get<WorldConfiguration>();
            if (data) {
                return godot::Variant(data->value);
            }
            // Return empty dictionary if component doesn't exist
            return godot::Variant(godot::TypedDictionary<godot::String, godot::Variant>());
        };

        stagehand::get_component_setters()["WorldConfiguration"] = [](flecs::world& world, flecs::entity_t entity_id, const godot::Variant& v) {
            if (entity_id != 0) {
                godot::UtilityFunctions::push_warning(
                    "WorldConfiguration is a singleton component and can only be set with entity_id 0. Got entity_id: " +
                    godot::String::num_uint64(entity_id)
                );
                return;
            }

            if (v.get_type() != godot::Variant::DICTIONARY) {
                godot::UtilityFunctions::push_warning(
                    "Cannot set WorldConfiguration from non-Dictionary type '" +
                    godot::Variant::get_type_name(v.get_type()) + "'"
                );
                return;
            }

            const godot::Dictionary source_dictionary = static_cast<godot::Dictionary>(v);

            godot::TypedDictionary<godot::String, godot::Variant> typed_dictionary;
            typed_dictionary.assign(source_dictionary);

            world.set<WorldConfiguration>({ typed_dictionary });
        };
    });

} // namespace stagehand
