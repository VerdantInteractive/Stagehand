#include <godot_cpp/core/class_db.hpp>

#include "stagehand/nodes/instanced_renderer_3d.h"

void InstancedRenderer3DLODConfiguration::set_mesh(const godot::Ref<godot::Mesh> &p_mesh) {
    mesh = p_mesh;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::set_visibility_range_begin(float p_value) {
    visibility_range_begin = p_value;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::set_visibility_range_end(float p_value) {
    visibility_range_end = p_value;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::set_visibility_range_begin_margin(float p_value) {
    visibility_range_begin_margin = p_value;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::set_visibility_range_end_margin(float p_value) {
    visibility_range_end_margin = p_value;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::set_visibility_range_fade_mode(godot::RenderingServer::VisibilityRangeFadeMode p_value) {
    visibility_range_fade_mode = p_value;
    emit_changed();
}

void InstancedRenderer3DLODConfiguration::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_mesh", "mesh"), &InstancedRenderer3DLODConfiguration::set_mesh);
    godot::ClassDB::bind_method(godot::D_METHOD("get_mesh"), &InstancedRenderer3DLODConfiguration::get_mesh);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_begin", "visibility_range_begin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_begin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_begin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_begin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_end", "visibility_range_end"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_end);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_end"), &InstancedRenderer3DLODConfiguration::get_visibility_range_end);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_begin_margin", "visibility_range_begin_margin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_begin_margin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_begin_margin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_begin_margin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_end_margin", "visibility_range_end_margin"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_end_margin);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_end_margin"), &InstancedRenderer3DLODConfiguration::get_visibility_range_end_margin);

    godot::ClassDB::bind_method(godot::D_METHOD("set_visibility_range_fade_mode", "visibility_range_fade_mode"),
                                &InstancedRenderer3DLODConfiguration::set_visibility_range_fade_mode);
    godot::ClassDB::bind_method(godot::D_METHOD("get_visibility_range_fade_mode"), &InstancedRenderer3DLODConfiguration::get_visibility_range_fade_mode);

    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::OBJECT, "mesh", godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_begin", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"),
                                 "set_visibility_range_begin", "get_visibility_range_begin");
    godot::ClassDB::add_property(
        "InstancedRenderer3DLODConfiguration",
        godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_begin_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"),
        "set_visibility_range_begin_margin", "get_visibility_range_begin_margin");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_end", godot::PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"),
                                 "set_visibility_range_end", "get_visibility_range_end");
    godot::ClassDB::add_property("InstancedRenderer3DLODConfiguration",
                                 godot::PropertyInfo(godot::Variant::FLOAT, "visibility_range_end_margin", godot::PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"),
                                 "set_visibility_range_end_margin", "get_visibility_range_end_margin");
    godot::ClassDB::add_property(
        "InstancedRenderer3DLODConfiguration",
        godot::PropertyInfo(godot::Variant::INT, "visibility_range_fade_mode", godot::PROPERTY_HINT_ENUM, "Disabled:0,Self:1,Dependencies:2"),
        "set_visibility_range_fade_mode", "get_visibility_range_fade_mode");

    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_DISABLED",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_DISABLED);
    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_SELF",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF);
    godot::ClassDB::bind_integer_constant(InstancedRenderer3DLODConfiguration::get_class_static(), godot::StringName(), "VISIBILITY_RANGE_FADE_DEPENDENCIES",
                                          godot::RenderingServer::VISIBILITY_RANGE_FADE_DEPENDENCIES);
}
