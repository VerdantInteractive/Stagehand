#pragma once

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"

namespace stagehand::transform {

    TAG(ChangedPosition2D);
    GODOT_VARIANT(Position2D, godot::Vector2);
    
    TAG(ChangedRotation2D);
    FLOAT(Rotation2D);
    
    TAG(ChangedScale2D);
    GODOT_VARIANT(Scale2D, godot::Vector2, {1.0, 1.0});

    TAG(ChangedTransform2D);
    GODOT_VARIANT(Transform2D, godot::Transform2D);


    TAG(ChangedPosition3D);
    GODOT_VARIANT(Position3D, godot::Vector3);

    TAG(ChangedRotation3D);
    GODOT_VARIANT(Rotation3D, godot::Quaternion);

    TAG(ChangedScale3D);
    GODOT_VARIANT(Scale3D, godot::Vector3, {1.0, 1.0, 1.0});

    TAG(ChangedTransform3D);
    GODOT_VARIANT(Transform3D, godot::Transform3D);

} // namespace stagehand::transform
