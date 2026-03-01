#pragma once

#include <cmath>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/registry.h"

#include "demos/_test/Instanced Renderer/components.h"
#include "prefabs.h" // IWYU pragma: keep

REGISTER_IN_MODULE(instanced_renderer, [](flecs::world &world) {
    world.system<stagehand::transform::Position3D, instanced_renderer::InstanceColour>("Movement")
        .write<stagehand::transform::HasChangedPosition3D>()
        .each([](flecs::entity e, stagehand::transform::Position3D &position, instanced_renderer::InstanceColour &instance_colour) {
            position.z += 0.01f;
            float time = (float)e.world().get_info()->world_time_total;
            instance_colour.x = (std::sin(time) + 1.0f) * 0.5f;
            e.enable<stagehand::transform::HasChangedPosition3D>();
            e.enable<instanced_renderer::HasChangedInstanceColour>();
            // godot::UtilityFunctions::print(godot::String("Entity ") + godot::String::num_int64(entity.id()) + " position: " + godot::String(position));
        });
});
