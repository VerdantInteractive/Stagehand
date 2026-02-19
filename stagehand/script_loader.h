#pragma once

#include <string>

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "flecs.h"

namespace stagehand {

    /// Class that loads Flecs script files (*.flecs) from a directory (recursively).
    /// Higher level directories are processed first.
    class ScriptLoader {
      public:
        /// Construct with a filesystem path to the scripts root (relative or absolute). Default to a Godot resource path. The loader will convert `res://`
        /// paths to absolute filesystem paths using ProjectSettings when running inside the engine. This keeps the scripts folder inside the project resources.
        explicit ScriptLoader(const std::string &scripts_root = "res://") : root_path(scripts_root) {}

        /// Load scripts into the provided world. Prints errors via Godot and a short summary when finished.
        void load(flecs::world &world, const godot::TypedArray<godot::String> &modules_to_load) const;

      private:
        std::string root_path;
    };

} // namespace stagehand
