#include "stagehand/prefab.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

#include "stagehand/world.h"

namespace stagehand {

    Prefab::Prefab() {}

    Prefab::~Prefab() {}

    void Prefab::set_prefab_name(const godot::String &p_name) { prefab_name = p_name; }

    godot::String Prefab::get_prefab_name() const { return prefab_name; }

    void Prefab::set_parent(const godot::Ref<Prefab> &p_parent) { parent = p_parent; }

    godot::Ref<Prefab> Prefab::get_parent() const { return parent; }

    void Prefab::is_a(const godot::Ref<Prefab> &p_parent) { parent = p_parent; }

    void Prefab::set_components(const godot::TypedDictionary<godot::String, godot::Variant> &p_components) { components = p_components; }

    godot::TypedDictionary<godot::String, godot::Variant> Prefab::get_components() const { return components; }

    uint64_t Prefab::register_with_world(FlecsWorld *world) {
        if (!world || prefab_name.is_empty()) {
            return 0;
        }

        // Create (or get existing) prefab entity
        uint64_t prefab_id = world->create_prefab(prefab_name);

        // Set components
        godot::Array keys = components.keys();
        for (int i = 0; i < keys.size(); ++i) {
            godot::String key = keys[i];
            godot::Variant value = components[key];
            world->set_component(key, value, prefab_id);
        }

        // Handle inheritance
        if (parent.is_valid()) {
            uint64_t parent_id = parent->register_with_world(world);
            if (parent_id != 0) {
                world->world.entity(static_cast<ecs_entity_t>(prefab_id)).is_a(world->world.entity(static_cast<ecs_entity_t>(parent_id)));
            }
        }

        return prefab_id;
    }

    void Prefab::_bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("set_prefab_name", "name"), &Prefab::set_prefab_name);
        godot::ClassDB::bind_method(godot::D_METHOD("get_prefab_name"), &Prefab::get_prefab_name);
        godot::ClassDB::bind_method(godot::D_METHOD("set_parent", "parent"), &Prefab::set_parent);
        godot::ClassDB::bind_method(godot::D_METHOD("get_parent"), &Prefab::get_parent);
        godot::ClassDB::bind_method(godot::D_METHOD("is_a", "parent"), &Prefab::is_a);
        godot::ClassDB::bind_method(godot::D_METHOD("set_components", "components"), &Prefab::set_components);
        godot::ClassDB::bind_method(godot::D_METHOD("get_components"), &Prefab::get_components);
        godot::ClassDB::bind_method(godot::D_METHOD("register_to_world", "world"), &Prefab::register_with_world);

        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "prefab_name"), "set_prefab_name", "get_prefab_name");
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "parent", godot::PROPERTY_HINT_RESOURCE_TYPE, "Prefab"), "set_parent", "get_parent");
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "components", godot::PROPERTY_HINT_TYPE_STRING,
                                         godot::String::num_int64(godot::Variant::STRING) + "/" + godot::String::num_int64(godot::PROPERTY_HINT_NONE) + ":",
                                         godot::PROPERTY_USAGE_DEFAULT),
                     "set_components", "get_components");
    }

} // namespace stagehand
