#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    struct Signal {
        godot::StringName name;
        godot::Dictionary data;
    };

} // namespace stagehand

inline stagehand::Registry register_godot_signal_component([](flecs::world& world)
{
    world.component<stagehand::Signal>("stagehand::Signal");
});

// Helper function to emit Godot signals from Flecs systems safely
inline void emit_godot_signal(const flecs::world& world, flecs::entity source_entity, const godot::StringName& name, const godot::Dictionary& data = godot::Dictionary()) {
    stagehand::Signal signal{ name, data };
    // Defer the emission to ensure it happens at a safe synchronization point (main thread usually)
    world.defer([source_entity, signal]() {
        source_entity.world().event<stagehand::Signal>()
            .entity(source_entity)
            .ctx(signal)
            .emit();
    });
}
