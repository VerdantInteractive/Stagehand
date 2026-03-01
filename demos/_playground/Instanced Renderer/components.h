#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/rendering.h"

#include "godot_cpp/variant/vector4.hpp"

namespace instanced_renderer {
    GODOT_VARIANT(InstanceColour, godot::Vector4, {0.0f, 1.0f, 1.0f, 1.0f}).then([](auto c) { c.template add<stagehand::rendering::IsInstanceUniform>(); });
} // namespace instanced_renderer
