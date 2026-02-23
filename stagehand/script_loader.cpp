#include "stagehand/script_loader.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <string_view>
#include <vector>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using godot::UtilityFunctions;

namespace stagehand {

    /// Helper to extract module name from script content if present. Returns empty string if no module statement is found.
    static std::string get_module_name(const std::string &script) {
        std::istringstream stream(script);
        std::string line;
        while (std::getline(stream, line)) {
            // Strip comments
            size_t comment_pos = line.find("//");
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }

            // Trim whitespace
            size_t first = line.find_first_not_of(" \t\r");
            if (first == std::string::npos)
                continue;
            size_t last = line.find_last_not_of(" \t\r");
            line = line.substr(first, (last - first + 1));

            if (line.rfind("module", 0) == 0) { // Starts with "module"
                // Extract name
                size_t name_start = line.find_first_not_of(" \t", 6); // 6 is length of "module"
                if (name_start != std::string::npos) {
                    std::string name = line.substr(name_start);
                    // Name might be followed by whitespace
                    size_t name_end = name.find_first_of(" \t");
                    if (name_end != std::string::npos) {
                        name = name.substr(0, name_end);
                    }
                    return name;
                }
            }
        }
        return "";
    }

    void ScriptLoader::run_all(flecs::world &world, const godot::TypedArray<godot::String> &modules_to_import) const {
        constexpr std::string_view res_prefix = "res://";

        // We'll collect resource-style paths (res://...) using Godot's DirAccess so exported builds work.
        // If the configured `root_path` isn't already a resource path, treat it as relative to `res://` (purely resource-based loading).
        std::string res_root = root_path;
        std::string_view root_view = res_root;
        if (!root_view.starts_with(res_prefix)) {
            // Ensure we don't end up with a double slash when combining.
            if (!res_root.empty() && res_root[0] == '/') {
                res_root = res_root.substr(1);
            }
            res_root = std::string(res_prefix) + res_root;
        }

        // Collect resource paths by recursively walking the Godot virtual filesystem.
        std::vector<std::string> resource_paths;
        std::function<void(const godot::String &)> walk;
        walk = [&](const godot::String &base) {
            godot::Ref<godot::DirAccess> dir = godot::DirAccess::open(base);
            if (dir.is_null()) {
                UtilityFunctions::push_warning(godot::String("Flecs scripts path does not exist: ") + base);
                return;
            }

            dir->list_dir_begin();
            for (godot::String name = dir->get_next(); !name.is_empty(); name = dir->get_next()) {
                if (name == "." || name == "..") {
                    continue;
                }

                // Avoid ambiguous operator+ overloads with godot::String by constructing the child path explicitly.
                godot::String child = base;
                if (!base.ends_with("/")) {
                    child += godot::String("/");
                }
                child += name;
                // Recursively walk the directory, unless it's the stagehand root directory as we don't want to parse example scripts in the Flecs codebase.
                if (dir->current_is_dir() && !dir->file_exists("stagehand.gdextension")) {
                    walk(child);
                } else if (name.get_extension() == "flecs") {
                    resource_paths.push_back(child.utf8().get_data());
                }
            }
            dir->list_dir_end();
        };

        walk(godot::String(res_root.c_str()));

        // Sort resource paths by directory depth (count of '/') then alphabetically to mimic previous ordering
        std::sort(resource_paths.begin(), resource_paths.end(), [](const std::string &a, const std::string &b) {
            constexpr auto depth = [](const std::string &s) noexcept -> size_t { return static_cast<size_t>(std::count(s.begin(), s.end(), '/')); };
            const size_t da = depth(a);
            const size_t db = depth(b);
            if (da != db) {
                return da < db;
            }
            return a < b;
        });

        std::vector<std::string> loaded_scripts;
        for (const std::string &path_str : resource_paths) {
            godot::String godot_path(path_str.c_str());
            godot::String file_contents = godot::FileAccess::get_file_as_string(godot_path);
            std::string script_str = file_contents.utf8().get_data();
            // Normalize CRLF to LF to prevent parsing issues with flecs scripts.
            std::erase(script_str, '\r');

            if (script_str.empty()) {
                UtilityFunctions::push_error(godot::String("Failed to read flecs script file: ") + godot_path);
                continue;
            }

            // If the script declares a module, only load it if the module is in the list of modules to load for the world.
            std::string module_name = get_module_name(script_str);
            if (!module_name.empty()) {
                bool should_load = false;
                // Check exact match (dot separator)
                if (modules_to_import.has(godot::String(module_name.c_str()))) {
                    should_load = true;
                } else {
                    // Check C++ style match (double colon separator)
                    std::string cpp_name = module_name;
                    size_t pos = 0;
                    while ((pos = cpp_name.find(".", pos)) != std::string::npos) {
                        cpp_name.replace(pos, 1, "::");
                        pos += 2;
                    }
                    if (modules_to_import.has(godot::String(cpp_name.c_str()))) {
                        should_load = true;
                    }
                }

                if (!should_load) {
                    continue;
                }
            }

            // Run the script from the in-memory string; pass the resource path for error reporting.
            int result = world.script_run(path_str.c_str(), script_str.c_str());
            if (result != 0) {
                UtilityFunctions::push_error(godot::String("Error running flecs script: ") + godot_path);
            } else {
                // Report path relative to the configured root when it's a resource path.
                std::string_view path_view = path_str;
                std::string_view root_view = root_path;
                if (path_view.starts_with(res_prefix) && root_view.starts_with(res_prefix)) {
                    std::string rel = path_str.substr(root_path.size());
                    if (!rel.empty() && rel[0] == '/') {
                        rel.erase(0, 1);
                    }
                    loaded_scripts.push_back(rel);
                } else {
                    loaded_scripts.push_back(path_str);
                }
            }
        }

        if (!loaded_scripts.empty()) {
            godot::String output = godot::String::num_int64(loaded_scripts.size()) + " Flecs scripts loaded: ";
            for (size_t i = 0; i < loaded_scripts.size(); ++i) {
                output += loaded_scripts[i].c_str();
                if (i < loaded_scripts.size() - 1) {
                    output += ", ";
                } else {
                    output += ".";
                }
            }
            UtilityFunctions::print(output);
        }
    }

} // namespace stagehand
