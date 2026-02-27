#pragma once

namespace stagehand_demos::game_of_life::names {

#define NAMESPACE_STR "stagehand_demos::game_of_life"

    constexpr const char *NAMESPACE = NAMESPACE_STR;

    namespace prefabs {
        constexpr const char *CELL = NAMESPACE_STR "::Cell";
    }

#undef NAMESPACE_STR
} // namespace stagehand_demos::game_of_life::names
