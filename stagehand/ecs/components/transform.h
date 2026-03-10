#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"

namespace stagehand::transform {

    GODOT_VARIANT(Position2D, godot::Vector2);
    FLOAT(Rotation2D);
    GODOT_VARIANT(Scale2D, godot::Vector2, {1.0, 1.0});
    GODOT_VARIANT(Transform2D, godot::Transform2D);

    GODOT_VARIANT(Position3D, godot::Vector3);
    GODOT_VARIANT(Rotation3D, godot::Quaternion);
    GODOT_VARIANT(Scale3D, godot::Vector3, {1.0, 1.0, 1.0});
    GODOT_VARIANT(Transform3D, godot::Transform3D);

} // namespace stagehand::transform
