#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

class FlecsScript : public godot::Resource {
    // We can't extend the engine's native abstract Script class from a GDExtension in a way that makes the class instantiable by Ref<T>::instantiate()
    // or the resource system. So we extend Resource instead.
    GDCLASS(FlecsScript, godot::Resource);

  private:
    godot::String code;
    godot::String parse_error;

  protected:
    static void _bind_methods();

  public:
    void set_contents(const godot::String &p_code);
    godot::String get_contents() const;
    godot::String get_parse_error() const;
};
