#pragma once

#include <godot_cpp/classes/editor_export_plugin.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

class FlecsScriptEditorExportPlugin : public godot::EditorExportPlugin {
    GDCLASS(FlecsScriptEditorExportPlugin, godot::EditorExportPlugin);

  protected:
    static void _bind_methods() {}

  public:
    FlecsScriptEditorExportPlugin() = default;
    ~FlecsScriptEditorExportPlugin() override = default;

    void _export_begin(const godot::PackedStringArray &p_features, bool p_is_debug, const godot::String &p_path, uint32_t p_flags) override;
};
