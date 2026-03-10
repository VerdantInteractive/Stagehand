#pragma once

#include "stagehand/ecs/components/godot_variants.h"

namespace stagehand {
    // Hide the comma from the macro parser
    using WorldConfigurationType = godot::TypedDictionary<godot::String, godot::Variant>;
    GODOT_VARIANT(WorldConfiguration, WorldConfigurationType).then([](auto c) { c.add(flecs::Singleton); });

} // namespace stagehand
