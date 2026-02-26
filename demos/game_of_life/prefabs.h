
#pragma once

#include "stagehand/registry.h"

#include "components.h"
#include "names.h"

namespace game_of_life {
    inline flecs::entity Cell;

    inline stagehand::Registry register_cell_prefab([](flecs::world &world) {
        Cell = world.prefab(names::prefabs::CELL).add<GridPosition>().add<GridNeighbours>().set<AliveNeighbourCount>({0});
    });
} // namespace game_of_life
