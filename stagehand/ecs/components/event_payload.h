#pragma once

#include <cstdint>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    /// Event payload component used for Stagehand event and signal bridging.
    struct EventPayload {
        godot::StringName name;
        godot::Dictionary data;
        uint64_t source_entity_id = 0;
    };

    REGISTER([](flecs::world &world) { world.component<EventPayload>("stagehand::EventPayload"); });
} // namespace stagehand
