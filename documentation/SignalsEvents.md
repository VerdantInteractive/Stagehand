# Stagehand Signals & Events

This document describes how Stagehand transports event data between Flecs and Godot, with a focus on:

- `FlecsWorld::emit_event()` (GDScript-facing API)
- `stagehand::emit_signal()` (C++ helper for Flecs systems)
- `stagehand::EventPayload` (shared event payload)

## Overview

Stagehand uses a single payload type, `stagehand::EventPayload`, to carry event metadata:

- `name` (`godot::StringName`): logical event/signal name
- `data` (`godot::Dictionary`): event payload data
- `source_entity_id` (`uint64_t`): optional source entity identifier

Two producer paths feed this payload into Flecs:

1. `emit_event()` from GDScript
2. `emit_signal()` from C++ systems

Then a world-level Flecs observer forwards every Stagehand `EventPayload` to Godot through the `stagehand_signal_emitted(name, data)` signal.

## `emit_event()` (GDScript → Flecs)

Signature (bound on `FlecsWorld`):

```gdscript
emit_event(event_name: StringName, data: Dictionary = {}, source_entity_id: int = 0)
```

### Behavior

- Builds an `EventPayload` with `name`, `data`, and `source_entity_id`.
- Chooses the Flecs event emitter entity:
  - If `source_entity_id != 0`: requires that entity to be alive.
  - If `source_entity_id == 0`: uses internal entity `stagehand::internal::no_source_event_emitter`.
- Emits through Flecs as:
  - event type: `EventPayload`
  - id filter: `flecs::Any`
  - payload context: `ctx(payload)`

### Error handling

- If called before world initialization, a warning is pushed and nothing is emitted.
- If `source_entity_id` is non-zero but invalid, a warning is pushed and nothing is emitted.

### GDScript example

```gdscript
extends FlecsWorld

func _ready() -> void:
    stagehand_signal_emitted.connect(_on_stagehand_signal_emitted)

    emit_event(&"EnemyDied", {
        "enemy_type": "skeleton",
        "xp": 25
    })

func _on_stagehand_signal_emitted(name: StringName, data: Dictionary) -> void:
    if name == &"EnemyDied":
        print("XP awarded: ", data.get("xp", 0))
```

## `emit_signal()` (C++ Flecs systems → Flecs)

Defined in `stagehand/utilities/emit_signal.h` and centered on `EventPayload`.

### Overloads

```cpp
void emit_signal(const flecs::world &world, const stagehand::EventPayload &payload);
void emit_signal(const flecs::entity &source_entity, const stagehand::EventPayload &payload);
void emit_signal(const flecs::iter &it, size_t index, const stagehand::EventPayload &payload);
```

### Source entity inference rules

- `emit_signal(world, payload)`:
  - Cannot infer source entity from `flecs::world` alone.
  - Uses `payload.source_entity_id` when set.
  - Falls back to internal emitter `stagehand::internal::no_source_event_emitter` when unset.
- `emit_signal(source_entity, payload)`:
  - If `payload.source_entity_id == 0`, it is automatically filled from `source_entity.id()`.
- `emit_signal(it, index, payload)`:
  - Uses `it.entity(index)` and forwards to the entity overload.

### Deferred emission semantics

`emit_signal()` uses `world.defer(...)` to emit at a safe synchronization point. This avoids immediate structural side effects while systems are iterating.

In practice, delivery is observed after world progression (`world.progress(...)` / `FlecsWorld::progress(...)`).

### C++ examples

Emit from a known entity:

```cpp
flecs::entity source_entity = world.entity("game::Player");

stagehand::EventPayload payload;
payload.name = "player_spawned";
payload.data = godot::Dictionary{{"hp", 100}};

stagehand::emit_signal(source_entity, payload);
```

Emit from iterator row entity:

```cpp
world.system<>().each([](flecs::iter &it, size_t index) {
    stagehand::EventPayload payload;
    payload.name = "row_event";
    payload.data = godot::Dictionary();
    stagehand::emit_signal(it, index, payload);
});
```

Emit with world-only context and explicit source ID:

```cpp
stagehand::EventPayload payload;
payload.name = "manual_source";
payload.data = godot::Dictionary();
payload.source_entity_id = static_cast<uint64_t>(some_entity.id());

stagehand::emit_signal(world, payload);
```

## Bridge to Godot signal

`FlecsWorld::register_signal_observer()` installs a Flecs observer that listens for:

- event type: `EventPayload`
- id filter: `flecs::Any`

For each matching event, Stagehand emits the Godot signal:

```text
stagehand_signal_emitted(name: StringName, data: Dictionary)
```

Notes:

- The Godot signal currently forwards `name` and `data` (not `source_entity_id`).
- Consumers that need source identity should include it in `data` or read it via ECS-side observers/components.

## When to use which API

- Use `emit_event()` when initiating from GDScript/gameplay scripts.
- Use `emit_signal(source_entity, payload)` in C++ systems when you already have an entity.
- Use `emit_signal(it, index, payload)` inside iterator-based system callbacks.
- Use `emit_signal(world, payload)` only when no entity context exists; set `payload.source_entity_id` explicitly if provenance matters.

## Related source files

- `stagehand/world.h`
- `stagehand/world.cpp`
- `stagehand/utilities/emit_signal.h`
- `stagehand/ecs/components/event_payload.h`
- `tests/integration/tests/event_emission/event_emission.gd`
- `tests/integration/tests/signals/signals.gd`