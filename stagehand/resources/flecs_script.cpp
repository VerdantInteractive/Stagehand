#include "flecs_script.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "flecs.h"

#include "stagehand/registry.h"

void FlecsScript::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_code", "code"), &FlecsScript::set_contents);
    godot::ClassDB::bind_method(godot::D_METHOD("get_code"), &FlecsScript::get_contents);
    godot::ClassDB::bind_method(godot::D_METHOD("get_parse_error"), &FlecsScript::get_parse_error);

    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "code", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_NO_EDITOR), "set_code", "get_code");
}

void FlecsScript::set_contents(const godot::String &p_code) {
    code = p_code;

    // In-editor validation: parse the script using the editor Flecs world.
    // Store any parse error on the resource so tests and editor UI can query it
    // without producing ERROR output.
    if (godot::Engine::get_singleton()->is_editor_hint()) {
        flecs::world &editor_world = stagehand::get_editor_world();
        ecs_script_eval_result_t eval_result = {NULL};
        const char *name = "<editor>";
        const char *content_cstr = code.utf8().get_data();
        ecs_script_t *script = ecs_script_parse(editor_world.c_ptr(), name, content_cstr, NULL, &eval_result);
        if (!script) {
            if (eval_result.error) {
                parse_error = godot::String(eval_result.error);
                ecs_os_free(eval_result.error);
            } else {
                parse_error = "Flecs script parse failed (unknown error)";
            }
        } else {
            ecs_script_free(script);
            parse_error = godot::String();
        }
    } else {
        parse_error = godot::String();
    }
}

godot::String FlecsScript::get_contents() const { return code; }

godot::String FlecsScript::get_parse_error() const { return parse_error; }
