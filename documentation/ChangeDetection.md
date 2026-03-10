
## Change Detection

To avoid the error-prone manual management of `HasChanged*` tags, Stagehand provides zero-overhead abstractions that automatically toggle these tags when components are modified.

To enable these features, components must be defined using Stagehand's macros that are described in the Components manual.

Change tracking can be enabled or disabled at component definition time:

- Use normal macros (e.g. `FLOAT`, `GODOT_VARIANT`, `VECTOR`) to enable change tracking.
- Use underscore macros (e.g. `FLOAT_`, `GODOT_VARIANT_`, `VECTOR_`) to opt out of change tracking and avoid the extra overhead.

### The `stagehand::entity` Wrapper

`stagehand::entity` is a lightweight wrapper around `flecs::entity` and can be used in its place. It intercepts component modifications to handle change detection logic. It has the same memory layout as `flecs::entity` and incurs no runtime overhead in optimized builds.

If a component was defined with an underscore macro (`*_`), `stagehand::entity` methods still work normally but skip change-tag toggling for that component.

#### Usage in Systems

Replace `flecs::entity` with `stagehand::entity` in your system signatures to gain access to the enhanced API.

```cpp
#include "stagehand/entity.h"
// ...
world.system<Position, Velocity>()
    .each([](stagehand::entity e, Position& p, Velocity& v) {
        
        // 1. Optimized Modify (Preferred)
        // Uses the reference already obtained by the query without any additional lookups.
        // In the following example, HasChangedPosition is triggered.
        e.modify(p, [](Position& val) {
            val.x += 1.0f;
        });

        // 2. Lookup Modify
        // Looks up the component on the entity. Safe if component might be missing.
        // In the following example, HasChangedVelocity is triggered.
        e.modify<Velocity>([](Velocity& val) {
            val.x = 0.0f;
        });

        // 3. Set
        // Overwrites the component. Triggers HasChangedVelocity.
        e.set<Velocity>({10.0f, 0.0f, 0.0f});
    });
```

Another system can then match on the relevant change detection tag to skip unnecessary work. For example:

```cpp
// ...
world.system<Position, const HasChangedVelocity>()
    .each([](stagehand::entity e, Position& position, Velocity& velocity) {
        e.set()
    });
```

### The Stream Operator (`<<`)

For a concise "fire-and-forget" syntax, you can use the `<<` operator to set component values on a raw `flecs::entity`. This is particularly useful for initialization or simple updates.

```cpp
// Sets the Position component and enables HasChangedPosition (if Position was defined with tracking enabled)
e << Position{10, 20, 30};

// Works with Godot variants too
e << NodePath("some/path");
```

### Summary of Modification Patterns

| Pattern | Use Case | Overhead | Change Tagging |
| :--- | :--- | :--- | :--- |
| `e.modify(comp, lambda)` | Inside `.each()` loops when you have the component ref. | Zero | Automatic when component has tracking; no-op otherwise |
| `e.modify<T>(lambda)` | When you only have the entity handle. | Low (1 lookup) | Automatic when component has tracking; no-op otherwise |
| `e.set<T>(val)` | Overwriting a component value. | Low (1 lookup) | Automatic when component has tracking; no-op otherwise |
| `e << val` | Concise syntax for setting values. | Low (1 lookup) | Automatic when component has tracking; no-op otherwise |
