#pragma once

#include <flecs.h>

#include <godot_cpp/variant/string.hpp>

namespace stagehand_demos::surwave::enemy_type {

    inline godot::String from_entity_prefab(const flecs::entity &entity) {
        const flecs::entity prefab_entity = entity.target(flecs::IsA);
        if (!prefab_entity.is_valid()) {
            return godot::String();
        }

        const char *prefab_name = prefab_entity.name();
        if (prefab_name == nullptr) {
            return godot::String();
        }

        return godot::String(prefab_name);
    }

} // namespace stagehand_demos::surwave::enemy_type
