#pragma once

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/registry.h"

#include "demos/surwave/cpp/components/singletons.h"

using namespace stagehand_demos::surwave;

REGISTER_IN_MODULE(stagehand_demos::surwave, [](flecs::world &world) {
    world.system<stagehand::transform::Position2D, godot::Transform2D, const stagehand::physics::Velocity2D>("Velocity to Position")
        .kind(flecs::PostUpdate)
        .each([](flecs::iter &it, size_t i, stagehand::transform::Position2D &position, godot::Transform2D &transform,
                 const stagehand::physics::Velocity2D &velocity) {
            position += velocity * it.delta_time();
            transform.columns[2] = position;
        });
});
