#include "stagehand/editor/component_schema.h"

#include <godot_cpp/core/class_db.hpp>

#include "stagehand/registry.h"

namespace stagehand {

    static void ensure_registry_populated() {
        if (get_component_registry().empty()) {
            flecs::world tmp_world;
            register_components_and_systems_with_world(tmp_world);
        }
    }

    void ComponentSchema::_bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("get_registered_components"), &ComponentSchema::get_registered_components);
        godot::ClassDB::bind_method(godot::D_METHOD("get_component_default", "name"), &ComponentSchema::get_component_default);
    }

    ComponentSchema::ComponentSchema() {}

    ComponentSchema::~ComponentSchema() {}

    godot::Dictionary ComponentSchema::get_registered_components() const {
        ensure_registry_populated();

        // Create a temporary world to inspect component metadata
        flecs::world tmp_world;
        register_components_and_systems_with_world(tmp_world);

        godot::Dictionary components_by_namespace;
        const auto &registry = get_component_registry();

        for (const auto &[name, funcs] : registry) {
            if (!funcs.inspector)
                continue;
            ComponentInfo info;
            funcs.inspector(tmp_world, info);

            if (info.is_singleton) {
                continue;
            }

            godot::String full_name = info.name;
            godot::String namespace_name = "";
            int last_colon = full_name.rfind("::");
            if (last_colon != -1) {
                namespace_name = full_name.substr(0, last_colon);
            }

            if (!components_by_namespace.has(namespace_name)) {
                components_by_namespace[namespace_name] = godot::PackedStringArray();
            }
            godot::PackedStringArray list = components_by_namespace[namespace_name];
            list.append(godot::String(name.c_str()));
            components_by_namespace[namespace_name] = list;
        }
        return components_by_namespace;
    }

    godot::Variant ComponentSchema::get_component_default(const godot::String &name) const {
        ensure_registry_populated();
        std::string c_name = name.utf8().get_data();
        const auto &registry = get_component_registry();
        auto it = registry.find(c_name);
        if (it != registry.end() && it->second.defaulter) {
            return it->second.defaulter();
        }
        return godot::Variant();
    }

} // namespace stagehand