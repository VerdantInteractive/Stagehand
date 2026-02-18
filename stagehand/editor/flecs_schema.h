#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace stagehand {

    class FlecsSchema : public godot::Object {
        GDCLASS(FlecsSchema, godot::Object);

      protected:
        static void _bind_methods();

      public:
        FlecsSchema();
        ~FlecsSchema();

        godot::Dictionary get_registered_components() const;
        godot::Variant get_component_default(const godot::String &name) const;
    };

} // namespace stagehand