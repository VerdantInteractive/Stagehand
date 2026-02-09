#include <algorithm>
#include <vector>
#include <functional>
#include <string_view>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include "stagehand/scripts_loader.h"

using godot::UtilityFunctions;

namespace stagehand {

void ScriptsLoader::load(flecs::world& world) const
{
    constexpr std::string_view res_prefix = "res://";

    // We'll collect resource-style paths (res://...) using Godot's DirAccess so exported
    // builds work. If the configured `root_path` isn't already a resource path, treat it
    // as relative to `res://` (purely resource-based loading).
    std::string res_root = root_path;
    std::string_view root_view = res_root;
    if (!root_view.starts_with(res_prefix))
    {
        // Ensure we don't end up with a double slash when combining.
        if (!res_root.empty() && res_root[0] == '/') { res_root = res_root.substr(1); }
        res_root = std::string(res_prefix) + res_root;
    }

    // Collect resource paths by recursively walking the Godot virtual filesystem.
    std::vector<std::string> resource_paths;
    std::function<void(const godot::String&)> walk;
    walk = [&](const godot::String& base) {
        godot::Ref<godot::DirAccess> dir = godot::DirAccess::open(base);
        if (dir.is_null())
        {
            UtilityFunctions::push_warning(godot::String("Flecs scripts path does not exist: ") + base);
            return;
        }

        dir->list_dir_begin();
        for (godot::String name = dir->get_next(); !name.is_empty(); name = dir->get_next())
        {
            if (name == "." || name == "..") { continue; }

            bool is_dir = dir->current_is_dir();
            // Avoid ambiguous operator+ overloads with godot::String by constructing the
            // child path explicitly.
            godot::String child = base;
            if (!base.ends_with("/")) { child += godot::String("/"); }
            child += name;
            if (is_dir)
            {
                walk(child);
            }
            else if (name.get_extension() == "flecs")
            {
                resource_paths.push_back(child.utf8().get_data());
            }
        }
        dir->list_dir_end();
    };

    walk(godot::String(res_root.c_str()));

    // Sort resource paths by directory depth (count of '/') then alphabetically to mimic previous ordering
    std::sort(resource_paths.begin(), resource_paths.end(), [](const std::string& a, const std::string& b) {
        auto depth = [](const std::string& s) {
            return static_cast<size_t>(std::count(s.begin(), s.end(), '/'));
        };
        size_t da = depth(a);
        size_t db = depth(b);
        if (da != db) { return da < db; }
        return a < b;
    });

    std::vector<std::string> loaded_scripts;
    for (const std::string& path_str : resource_paths)
    {
        godot::String godot_path(path_str.c_str());
        godot::String file_contents = godot::FileAccess::get_file_as_string(godot_path);
        std::string script_str = file_contents.utf8().get_data();
        // Normalize CRLF to LF to prevent parsing issues with flecs scripts.
        std::erase(script_str, '\r');

        if (script_str.empty())
        {
            UtilityFunctions::push_error(godot::String("Failed to read flecs script file: ") + godot_path);
            continue;
        }

        // Run the script from the in-memory string; pass the resource path for error reporting.
        int result = world.script_run(path_str.c_str(), script_str.c_str());
        if (result != 0)
        {
            UtilityFunctions::push_error(godot::String("Error running flecs script: ") + godot_path);
        }
        else
        {
            // Report path relative to the configured root when it's a resource path.
            std::string_view path_view = path_str;
            std::string_view root_view = root_path;
            if (path_view.starts_with(res_prefix) && root_view.starts_with(res_prefix))
            {
                std::string rel = path_str.substr(root_path.size());
                if (!rel.empty() && rel[0] == '/') { rel.erase(0, 1); }
                loaded_scripts.push_back(rel);
            }
            else
            {
                loaded_scripts.push_back(path_str);
            }
        }
    }

    if (!loaded_scripts.empty())
    {
        godot::String output = godot::String::num_int64(loaded_scripts.size()) + " Flecs scripts loaded: ";
        for (size_t i = 0; i < loaded_scripts.size(); ++i) {
            output += loaded_scripts[i].c_str();
            if (i < loaded_scripts.size() - 1) { output += ", "; }
            else { output += "."; }
        }
        UtilityFunctions::print(output);
    }
}

} // namespace stagehand
