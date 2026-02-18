#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace stagehand {

    class ComponentSchema : public godot::Object {
        GDCLASS(ComponentSchema, godot::Object);

      protected:
        static void _bind_methods();

      public:
        ComponentSchema();
        ~ComponentSchema();

        godot::Dictionary get_registered_components() const;
        godot::Variant get_component_default(const godot::String &name) const;
    };

} // namespace stagehand
