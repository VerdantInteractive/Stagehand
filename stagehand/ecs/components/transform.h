#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"

namespace stagehand::transform {

    GODOT_VARIANT_(Position2D, godot::Vector2);
    FLOAT_(Rotation2D);
    GODOT_VARIANT_(Scale2D, godot::Vector2, {1.0, 1.0});
    GODOT_VARIANT_(Transform2D, godot::Transform2D);

    GODOT_VARIANT_(Position3D, godot::Vector3);
    GODOT_VARIANT_(Rotation3D, godot::Quaternion);
    GODOT_VARIANT_(Scale3D, godot::Vector3, {1.0, 1.0, 1.0});
    GODOT_VARIANT_(Transform3D, godot::Transform3D);

} // namespace stagehand::transform
