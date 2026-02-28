#pragma once

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/registry.h"

#include "prefabs.h" // IWYU pragma: keep

using namespace instanced_renderer;

REGISTER_IN_MODULE(instanced_renderer, [](flecs::world &world) {
    world
        .system<stagehand::transform::Position3D>("Movement")
        .write<stagehand::transform::ChangedPosition3D>()
        .each([](flecs::entity e, stagehand::transform::Position3D &position) {
            position.z += 0.001f;
            e.add<stagehand::transform::ChangedPosition3D>();
            // godot::UtilityFunctions::print(godot::String("Entity ") + godot::String::num_int64(entity.id()) + " position: " + godot::String(position));
        });
});
