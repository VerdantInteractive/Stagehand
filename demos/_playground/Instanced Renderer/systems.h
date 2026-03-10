#pragma once

#include <cmath>

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/entity.h"
#include "stagehand/registry.h"

#include "demos/_playground/Instanced Renderer/components.h"
#include "prefabs.h" // IWYU pragma: keep

REGISTER_IN_MODULE(instanced_renderer, [](flecs::world &world) {
    world.system<stagehand::transform::Position3D, instanced_renderer::InstanceColour>("Movement")
        .write<stagehand::transform::HasChangedPosition3D>()
        .each([](stagehand::entity e, stagehand::transform::Position3D &position, instanced_renderer::InstanceColour &instance_colour) {
            e.modify(position, [](stagehand::transform::Position3D &p) { p.z += 0.01f; });
            e.modify(instance_colour, [e](instanced_renderer::InstanceColour &c) {
                float time = (float)e.world().get_info()->world_time_total;
                c.x = (std::sin(time) + 1.0f) * 0.5f;
            });
        });
});
