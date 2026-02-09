#pragma once

#include "stagehand/ecs/components/macros.h"
#include "stagehand/ecs/components/godot_variants.h"

namespace stagehand {

    GODOT_VARIANT(Position2D, godot::Vector2);
    GODOT_VARIANT(Position3D, godot::Vector3);

    FLOAT(Rotation2D);
    GODOT_VARIANT(Rotation3D, godot::Quaternion);

    GODOT_VARIANT(Scale2D, godot::Vector2, { 1.0, 1.0 });
    GODOT_VARIANT(Scale3D, godot::Vector3, { 1.0, 1.0, 1.0 });

} // namespace stagehand
