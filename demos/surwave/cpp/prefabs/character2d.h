#pragma once

#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/rendering.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"

namespace stagehand_demos::surwave {
    inline flecs::entity Character2DPrefab;
}

using namespace stagehand_demos::surwave;
using Position2D = stagehand::transform::Position2D;
using Rotation2D = stagehand::transform::Rotation2D;
using Scale2D = stagehand::transform::Scale2D;
using Velocity2D = stagehand::physics::Velocity2D;
using RenderingCustomData = stagehand::rendering::CustomData;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    // clang-format off
    Character2DPrefab = world.prefab("Character2D")
                            .set<Velocity2D>({godot::Vector2(0.0f, 0.0f)})

                            .set<Position2D>({godot::Vector2(0.0f, 0.0f)})
                            .set<Rotation2D>({0.0f})
                            .set<Scale2D>({godot::Vector2(1.0f, 1.0f)})
                            .set<godot::Transform2D>(godot::Transform2D())

                            .set<RenderingCustomData>({0.0f, 0.0f, 0.0f, 0.0f});
    // clang-format on
});
