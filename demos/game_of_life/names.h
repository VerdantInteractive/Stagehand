#pragma once

namespace demos::game_of_life::names {

#define NAMESPACE_STR "demos::game_of_life"

    constexpr const char *NAMESPACE = NAMESPACE_STR;

    namespace prefabs {
        constexpr const char *CELL = NAMESPACE_STR "::Cell";
    }

#undef NAMESPACE_STR
} // namespace demos::game_of_life::names
