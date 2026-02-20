#pragma once

#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

class FlecsScriptResourceFormatLoader : public godot::ResourceFormatLoader {
    GDCLASS(FlecsScriptResourceFormatLoader, godot::ResourceFormatLoader);

  protected:
    static void _bind_methods() {}

  public:
    virtual godot::PackedStringArray _get_recognized_extensions() const override;
    virtual bool _recognize_path(const godot::String &p_path, const godot::StringName &p_type) const override;
    virtual godot::String _get_resource_type(const godot::String &p_path) const override;
    virtual bool _handles_type(const godot::StringName &p_type) const override;
    virtual godot::Variant
    _load(const godot::String &p_path, const godot::String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;
};
