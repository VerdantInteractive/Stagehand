# Stagehand ECS Patterns & Best Practices

Stagehand introduces specific patterns to streamline ECS usage, particularly focusing on ergonomic component modification and automatic change detection.

## Automatic Change Detection

To avoid the error-prone manual management of `HasChanged*` tags, Stagehand provides zero-overhead abstractions that automatically toggle these tags when components are modified.

### 1. The `stagehand::entity` Wrapper

`stagehand::entity` is a lightweight wrapper around `flecs::entity` that intercepts component modifications to handle change detection logic. It has the same memory layout as `flecs::entity` and incurs no runtime overhead in optimized builds.

#### Usage in Systems

Replace `flecs::entity` with `stagehand::entity` in your system signatures to gain access to the enhanced API.

```cpp
#include "stagehand/entity.h"

// ...

world.system<Position, Velocity>()
    .each([](stagehand::entity e, Position& p, Velocity& v) {
        
        // 1. Optimized Modify (Preferred)
        // Uses the reference already obtained by the query. 
        // No internal lookup. Triggers HasChangedPosition.
        e.modify(p, [](Position& val) {
            val.x += 1.0f;
        });

        // 2. Lookup Modify
        // Looks up the component on the entity. Safe if component might be missing.
        // Triggers HasChangedVelocity.
        e.modify<Velocity>([](Velocity& val) {
            val.x = 0.0f;
        });

        // 3. Set
        // Overwrites the component. Triggers HasChangedVelocity.
        e.set<Velocity>({10.0f, 0.0f, 0.0f});

        // 4. Assign
        // Optimized overwrite. Uses get_mut internally.
        // Faster than set() but assumes component exists (unsafe if missing).
        // Triggers HasChangedVelocity.
        e.assign<Velocity>({10.0f, 0.0f, 0.0f});
    });
```

### 2. The Stream Operator (`<<`)

For a concise "fire-and-forget" syntax, you can use the `<<` operator to set component values on a raw `flecs::entity`. This is particularly useful for initialization or simple updates.

```cpp
// Sets the Position component and enables HasChangedPosition
e << Position{10, 20, 30};

// Works with Godot variants too
e << NodePath("some/path");
```

## Component Definition

To enable these features, components must be defined using Stagehand's macros. These macros generate the component struct, the associated `HasChanged` tag, and the necessary type traits.

### Primitive Wrappers

Use macros from `stagehand/ecs/components/macros.h` for primitive types.

```cpp
// Defines struct Health { int32_t value; ... };
// Defines struct HasChangedHealth {};
INT32(Health); 

// Defines struct Speed { float value; ... };
FLOAT(Speed);
```

### Godot Variants

Use `GODOT_VARIANT` from `stagehand/ecs/components/godot_variants.h` to wrap Godot types.

```cpp
// Defines struct Position2D : public godot::Vector2 { ... };
GODOT_VARIANT(Position2D, godot::Vector2);
```

### Containers

Use `VECTOR` or `ARRAY` for STL containers.

```cpp
// Defines struct PathPoints { std::vector<Vector2> value; ... };
VECTOR(PathPoints, Vector2);
```

## Summary of Modification Patterns

| Pattern | Use Case | Overhead | Change Tagging |
| :--- | :--- | :--- | :--- |
| `e.modify(comp, lambda)` | Inside `.each()` loops when you have the component ref. | Zero | Automatic |
| `e.modify<T>(lambda)` | When you only have the entity handle. | Low (1 lookup) | Automatic |
| `e.set<T>(val)` | Overwriting a component value. | Low (1 lookup) | Automatic |
| `e << val` | Concise syntax for setting values. | Low (1 lookup) | Automatic |
