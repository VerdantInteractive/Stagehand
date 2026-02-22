#include "flecs_script_resource_format_loader.h"

#include <flecs.h>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "flecs_script.h"

using namespace godot;

PackedStringArray FlecsScriptResourceFormatLoader::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("flecs");
    return extensions;
}

bool FlecsScriptResourceFormatLoader::_recognize_path(const String &p_path, const StringName &p_type) const { return p_path.get_extension() == "flecs"; }

String FlecsScriptResourceFormatLoader::_get_resource_type(const String &p_path) const { return "FlecsScriptResource"; }

bool FlecsScriptResourceFormatLoader::_handles_type(const StringName &p_type) const { return ClassDB::is_parent_class(p_type, "FlecsScriptResource"); }

Variant FlecsScriptResourceFormatLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    ERR_FAIL_COND_V_MSG(file.is_null(), Variant(), "Cannot open file '" + p_path + "'.");

    String code = file->get_as_text();

    Ref<FlecsScript> flecs_script;
    flecs_script.instantiate();
    flecs_script->set_contents(code);

    return flecs_script;
}
