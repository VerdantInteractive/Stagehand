#pragma once

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    /// Component used to signal events to Godot.
    struct GodotSignal {
        godot::StringName name;
        godot::Dictionary data;
    };

} // namespace stagehand

inline stagehand::Registry register_godot_signal_component([](flecs::world &world) { world.component<stagehand::GodotSignal>("stagehand::GodotSignal"); });

/// Helper function to emit Godot signals from Flecs systems safely.
/// @param world The Flecs world.
/// @param source_entity The entity emitting the signal.
/// @param name The name of the signal.
/// @param data Optional dictionary of data associated with the signal.
inline void
emit_godot_signal(const flecs::world &world, flecs::entity source_entity, const godot::StringName &name, const godot::Dictionary &data = godot::Dictionary()) {
    const stagehand::GodotSignal signal{.name = name, .data = data};
    // Defer the emission to ensure it happens at a safe synchronization point (main thread usually)
    world.defer([source_entity, signal]() { source_entity.world().event<stagehand::GodotSignal>().entity(source_entity).ctx(signal).emit(); });
}
