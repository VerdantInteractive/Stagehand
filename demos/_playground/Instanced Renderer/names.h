#pragma once

namespace instanced_renderer::names {

#define NAMESPACE_STR "instanced_renderer"

    constexpr const char *NAMESPACE = NAMESPACE_STR;

    namespace prefabs {
        constexpr const char *TEST_INSTANCE = NAMESPACE_STR "::TestInstance";
    }

#undef NAMESPACE_STR
} // namespace instanced_renderer::names
