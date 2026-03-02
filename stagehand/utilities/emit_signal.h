#pragma once

#include "stagehand/ecs/components/signal.h"

namespace stagehand {
    /// Helper function to emit Godot signals from Flecs systems safely.
    /// @param world The Flecs world.
    /// @param source_entity The entity emitting the signal.
    /// @param name The name of the signal.
    /// @param data Optional dictionary of data associated with the signal.
    inline void
    emit_signal(const flecs::world &world, flecs::entity source_entity, const godot::StringName &name, const godot::Dictionary &data = godot::Dictionary()) {
        const stagehand::Signal signal{.name = name, .data = data};
        // Defer the emission to ensure it happens at a safe synchronization point (main thread usually)
        world.defer([source_entity, signal]() { source_entity.world().event<stagehand::Signal>().entity(source_entity).ctx(signal).emit(); });
    }
} // namespace stagehand
