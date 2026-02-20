#include "stagehand/editor/flecs_script_editor_export_plugin.h"

#include <functional>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FlecsScriptEditorExportPlugin::_export_begin(const PackedStringArray &p_features, bool p_is_debug, const String &p_path, uint32_t p_flags) {
    const String res_root = "res://";

    std::function<void(const String &)> walk;
    walk = [&](const String &base) {
        Ref<DirAccess> dir = DirAccess::open(base);
        if (dir.is_null()) {
            return;
        }

        dir->list_dir_begin();
        for (String name = dir->get_next(); !name.is_empty(); name = dir->get_next()) {
            if (name == "." || name == "..")
                continue;

            String child = base;
            if (!base.ends_with("/"))
                child += "/";
            child += name;

            if (dir->current_is_dir()) {
                walk(child);
            } else {
                String lower = child.to_lower();
                if (lower.ends_with(".flecs")) {
                    Ref<FileAccess> f = FileAccess::open(child, FileAccess::READ);
                    if (f.is_null()) {
                        UtilityFunctions::push_warning(String("Failed to read Flecs script for export: ") + child);
                        continue;
                    }
                    uint64_t size = f->get_length();
                    PackedByteArray bytes = f->get_buffer(static_cast<int>(size));
                    add_file(child, bytes, false);
                }
            }
        }
        dir->list_dir_end();
    };

    walk(res_root);
}
