#include "stagehand/registry.h"

#include <algorithm>
#include <mutex>

#include <godot_cpp/variant/array.hpp>

namespace stagehand {
    namespace {
        /// Returns the static vector of registration callbacks.
        std::vector<RegistrationCallback> &get_callbacks() {
            static std::vector<RegistrationCallback> callbacks;
            return callbacks;
        }

        /// Returns the static vector of module-scoped registration callbacks.
        /// Each entry pairs the module name with the callback that should be executed when that module is imported into a world.
        std::vector<std::pair<std::string, RegistrationCallback>> &get_module_callbacks() {
            static std::vector<std::pair<std::string, RegistrationCallback>> callbacks;
            return callbacks;
        }

        /// Returns the static mutex used to protect the callback list.
        std::mutex &get_mutex() {
            static std::mutex mtx;
            return mtx;
        }

        std::string normalize_entity_path(std::string path) {
            while (path.rfind("::", 0) == 0) {
                path.erase(0, 2);
            }
            return path;
        }

        std::string get_entity_path(const flecs::entity &entity) {
            flecs::string entity_path = entity.path();
            if (entity_path.c_str() && entity_path.c_str()[0] != '\0') {
                return normalize_entity_path(std::string(entity_path.c_str()));
            }

            flecs::string_view entity_name = entity.name();
            if (entity_name.c_str() && entity_name.c_str()[0] != '\0') {
                return std::string(entity_name.c_str());
            }

            return {};
        }

        std::string get_namespace_from_path(const std::string &path) {
            const size_t separator_index = path.rfind("::");
            if (separator_index == std::string::npos) {
                return {};
            }
            return path.substr(0, separator_index);
        }

        std::string find_module_path(const flecs::entity &entity) {
            flecs::entity current = entity;
            while (current.is_valid()) {
                if (current.has(flecs::Module)) {
                    return get_entity_path(current);
                }

                flecs::entity parent = current.parent();
                if (!parent.is_valid() || parent.id() == 0 || parent.id() == current.id()) {
                    break;
                }
                current = parent;
            }

            return {};
        }

        bool should_exclude_flecs_builtin(const std::string &path, const std::string &module_path, bool include_flecs_builtin) {
            if (include_flecs_builtin) {
                return false;
            }

            constexpr const char *flecs_prefix = "flecs::";
            const bool is_flecs_path = path.rfind(flecs_prefix, 0) == 0;
            const bool is_flecs_module = module_path.rfind(flecs_prefix, 0) == 0;
            return is_flecs_path || is_flecs_module;
        }
    } // namespace

    Registry::Registry(RegistrationCallback callback) { register_callback(std::move(callback)); }

    Registry::Registry(const char *module_name, RegistrationCallback callback) {
        if (!callback)
            return;
        // Store the module name and the raw callback. The callback will be executed later only when the module is actually imported.
        get_module_callbacks().emplace_back(std::string(module_name), std::move(callback));
    }
    void register_callback(RegistrationCallback callback) {
        if (!callback)
            return;
        std::lock_guard<std::mutex> lock(get_mutex());
        get_callbacks().push_back(std::move(callback));
    }

    void register_components_and_systems_with_world(flecs::world &world) {
        std::lock_guard<std::mutex> lock(get_mutex());
        // Only run non-module callbacks during global registration. Module callbacks are executed when their module is explicitly imported.
        for (RegistrationCallback callback : get_callbacks()) {
            if (callback != nullptr) {
                callback(world);
            }
        }
    }

    void run_module_callbacks_for(flecs::world &world, const std::string &module_name) {
        std::lock_guard<std::mutex> lock(get_mutex());
        for (auto &pair : get_module_callbacks()) {
            if (pair.first == module_name && pair.second) {
                // Execute the callback inside the module scope so entities created by the callback receive the proper qualified name.
                flecs::entity mod = world.entity(module_name.c_str());
                mod.add(flecs::Module);
                auto guard = world.scope(mod.id());
                pair.second(world);
                (void)guard;
            }
        }
    }

    bool has_module_callbacks_for(const std::string &module_name) {
        std::lock_guard<std::mutex> lock(get_mutex());
        for (auto &pair : get_module_callbacks()) {
            if (pair.first == module_name) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> get_registered_module_names() {
        std::lock_guard<std::mutex> lock(get_mutex());

        std::vector<std::string> module_names;
        module_names.reserve(get_module_callbacks().size());
        for (const auto &pair : get_module_callbacks()) {
            module_names.push_back(pair.first);
        }

        std::sort(module_names.begin(), module_names.end());
        module_names.erase(std::unique(module_names.begin(), module_names.end()), module_names.end());
        return module_names;
    }

    std::unordered_map<std::string, ComponentFunctions> &get_component_registry() {
        static std::unordered_map<std::string, ComponentFunctions> registry;
        return registry;
    }

    godot::Dictionary to_registered_entity_dictionary(const RegisteredEntityInfo &info) {
        godot::Dictionary entry;
        entry["id"] = static_cast<uint64_t>(info.id);
        entry["name"] = godot::String(info.name.c_str());
        entry["path"] = godot::String(info.path.c_str());
        entry["namespace"] = godot::String(info.namespace_path.c_str());
        entry["module"] = godot::String(info.module_path.c_str());

        entry["is_component"] = info.is_component;
        entry["is_prefab"] = info.is_prefab;
        entry["is_system"] = info.is_system;
        entry["is_tag"] = info.is_tag;

        entry["component_size"] = static_cast<uint64_t>(info.component_size);
        entry["component_alignment"] = static_cast<uint64_t>(info.component_alignment);

        godot::Array kinds;
        if (info.is_component) {
            kinds.push_back("component");
        }
        if (info.is_prefab) {
            kinds.push_back("prefab");
        }
        if (info.is_system) {
            kinds.push_back("system");
        }
        entry["kinds"] = kinds;

        return entry;
    }

    std::vector<RegisteredEntityInfo> collect_registered_entities(flecs::world &world, const bool include_flecs_builtin) {
        std::unordered_map<flecs::entity_t, RegisteredEntityInfo> entries;

        world.each<flecs::Component>([&entries](flecs::entity entity, const flecs::Component &component_info) {
            RegisteredEntityInfo &entry = entries[entity.id()];
            entry.id = entity.id();
            entry.is_component = true;
            entry.component_size = component_info.size;
            entry.component_alignment = component_info.alignment;
            entry.is_tag = component_info.size == 0;
        });

        world.each(flecs::Prefab, [&entries](flecs::entity entity) {
            RegisteredEntityInfo &entry = entries[entity.id()];
            entry.id = entity.id();
            entry.is_prefab = true;
        });

        world.each(flecs::System, [&entries](flecs::entity entity) {
            RegisteredEntityInfo &entry = entries[entity.id()];
            entry.id = entity.id();
            entry.is_system = true;
        });

        std::vector<RegisteredEntityInfo> result;
        result.reserve(entries.size());

        for (auto &[entity_id, entry] : entries) {
            const flecs::entity entity = world.entity(entity_id);
            if (!entity.is_valid()) {
                continue;
            }

            entry.path = get_entity_path(entity);
            if (entry.path.empty()) {
                continue;
            }

            entry.name = entity.name().c_str() ? std::string(entity.name().c_str()) : entry.path;
            entry.namespace_path = get_namespace_from_path(entry.path);
            entry.module_path = find_module_path(entity);

            if (should_exclude_flecs_builtin(entry.path, entry.module_path, include_flecs_builtin)) {
                continue;
            }

            result.push_back(std::move(entry));
        }

        std::sort(result.begin(), result.end(), [](const RegisteredEntityInfo &left, const RegisteredEntityInfo &right) {
            if (left.path == right.path) {
                return left.id < right.id;
            }
            return left.path < right.path;
        });

        return result;
    }
} // namespace stagehand
