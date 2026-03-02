#pragma once

#include <cstdint>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    /// Component used to signal events to Godot.
    // TODO: Rename this to EventPayload or something
    struct Signal {
        godot::StringName name;
        godot::Dictionary data;
        uint64_t source_entity_id = 0;
    };

    REGISTER([](flecs::world &world) { world.component<Signal>("stagehand::Signal"); });
} // namespace stagehand
