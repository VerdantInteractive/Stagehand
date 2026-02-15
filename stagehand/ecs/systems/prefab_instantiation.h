#pragma once

#include <string>

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "stagehand/names.h"
#include "stagehand/registry.h"

inline stagehand::Registry register_prefab_instantiation_systems([](flecs::world &world) {
    // System to enqueue prefab instantiation requests. Only meant to be called from GDScript.
    world.system<>(stagehand::names::systems::PREFAB_INSTANTIATION)
        .kind(0) // On-demand
        .run([&](flecs::iter &it) {
            const godot::Dictionary *parameters = static_cast<const godot::Dictionary *>(it.param());
            if (!parameters || parameters->is_empty()) {
                godot::UtilityFunctions::push_warning(godot::String(stagehand::names::systems::PREFAB_INSTANTIATION) +
                                                      ": system called without parameters. At least 'prefab' needs to be specified.");
                return;
            }

            if (!parameters->has("prefab")) {
                godot::UtilityFunctions::push_warning(godot::String(stagehand::names::systems::PREFAB_INSTANTIATION) +
                                                      ": 'prefab' parameter needs to be given with the name of the prefab to instantiate.");
                return;
            }
            if ((*parameters)["prefab"].get_type() != godot::Variant::STRING) {
                godot::UtilityFunctions::push_warning(godot::String(stagehand::names::systems::PREFAB_INSTANTIATION) +
                                                      ": 'prefab' parameter must be a String.");
                return;
            }

            godot::String prefab_name = (*parameters)["prefab"];
            std::string prefab_name_str = prefab_name.utf8().get_data();

            flecs::world world = it.world();
            flecs::entity prefab = world.lookup(prefab_name_str.c_str());
            if (!prefab.is_valid()) {
                godot::UtilityFunctions::push_warning(godot::String("Prefab Instantiation: prefab '") + prefab_name + "' not found in Flecs world");
                return;
            }

            flecs::entity instance = world.entity().is_a(prefab);

            if (parameters->has("components")) {
                godot::Variant components_as_godot_variant = (*parameters)["components"];
                if (components_as_godot_variant.get_type() != godot::Variant::DICTIONARY) {
                    godot::UtilityFunctions::push_warning(godot::String(stagehand::names::systems::PREFAB_INSTANTIATION) +
                                                          ": 'components' parameter must be a Dictionary.");
                    return;
                }
                godot::Dictionary components = components_as_godot_variant;
                auto &component_setters = stagehand::get_component_setters();
                godot::Array keys = components.keys();
                for (int j = 0; j < keys.size(); ++j) {
                    godot::String key = keys[j];
                    std::string key_std = key.utf8().get_data();
                    auto it_setter = component_setters.find(key_std);
                    if (it_setter != component_setters.end()) {
                        it_setter->second(world, instance, components[key]);
                    } else {
                        godot::UtilityFunctions::push_warning(godot::String("Prefab Instantiation: No setter found for component '") + key + "'.");
                    }
                }
            }
        });
});
