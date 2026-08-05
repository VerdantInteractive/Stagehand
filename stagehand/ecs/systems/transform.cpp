#include "stagehand/ecs/systems/transform.h"

namespace stagehand::transform {
    REGISTER([](flecs::world &world) {
        register_transform_decompose_system<Transform2D>(world);
        register_transform_decompose_system<Transform3D>(world);
    });

    REGISTER([](flecs::world &world) {
        register_transform_compose_system<Transform2D>(world);
        register_transform_compose_system<Transform3D>(world);
    });
} // namespace stagehand::transform
