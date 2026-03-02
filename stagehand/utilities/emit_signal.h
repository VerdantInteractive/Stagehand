#pragma once

#include <cstddef>

#include "stagehand/ecs/components/event_payload.h"

namespace stagehand {
    /// Helper function to emit Godot signals from Flecs systems safely.
    /// @param world The Flecs world.
    /// @param payload Signal payload. source_entity_id is used as source when non-zero.
    inline void emit_signal(const flecs::world &world, const stagehand::EventPayload &payload) {
        stagehand::EventPayload signal = payload;

        ecs_entity_t emitter_entity_id = 0;
        if (signal.source_entity_id != 0) {
            emitter_entity_id = static_cast<ecs_entity_t>(signal.source_entity_id);
        } else {
            emitter_entity_id = world.entity("stagehand::internal::no_source_event_emitter").id();
        }

        // Defer the emission to ensure it happens at a safe synchronization point (main thread usually)
        world.defer(
            [world, signal, emitter_entity_id]() { world.event<stagehand::EventPayload>().id(flecs::Any).entity(emitter_entity_id).ctx(signal).emit(); });
    }

    /// Emits a signal using an entity as source.
    /// @param source_entity The entity emitting the signal.
    /// @param payload Signal payload. source_entity_id is populated from source_entity when missing.
    inline void emit_signal(const flecs::entity &source_entity, const stagehand::EventPayload &payload) {
        stagehand::EventPayload payload_with_source = payload;
        if (payload_with_source.source_entity_id == 0) {
            payload_with_source.source_entity_id = static_cast<uint64_t>(source_entity.id());
        }

        emit_signal(source_entity.world(), payload_with_source);
    }

    /// Emits a signal using the entity at an iterator row as source.
    /// @param it Flecs iterator.
    /// @param index Row index in the iterator.
    /// @param payload Signal payload.
    inline void emit_signal(const flecs::iter &it, const size_t index, const stagehand::EventPayload &payload) { emit_signal(it.entity(index), payload); }
} // namespace stagehand
