#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"

namespace game_of_life {
    // Cell

    TAG(IsAlive);
    TAG(WasAlive);

    /// A tag to indicate whether a state change happened within a cell's neighbourhood.
    TAG(IsInActiveNeighbourhood);

    /// Component to track a cell's position in the grid.
    GODOT_VARIANT(GridPosition, Vector2i);

    /// Component that stores references to a cell's eight neighbouring grid cells as an array of ecs_entity_t.
    ARRAY(GridNeighbours, ecs_entity_t, 8);

    UINT8(AliveNeighbourCount, 0);
} // namespace game_of_life
