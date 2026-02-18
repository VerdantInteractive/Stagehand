#include "stagehand/registry.h"

#include <mutex>

namespace stagehand {

    namespace {
        /// Returns the static vector of registration callbacks.
        std::vector<RegistrationCallback> &get_callbacks() {
            static std::vector<RegistrationCallback> callbacks;
            return callbacks;
        }

        /// Returns the static mutex used to protect the callback list.
        std::mutex &get_mutex() {
            static std::mutex mtx;
            return mtx;
        }
    } // namespace

    Registry::Registry(RegistrationCallback callback) { register_callback(std::move(callback)); }

    Registry::Registry(const char *module_name, RegistrationCallback callback) {
        if (!callback)
            return;
        // Wrap the provided callback so it runs inside the named module's scope.
        register_callback([name = std::string(module_name), cb = std::move(callback)](flecs::world &world) {
            // Create/find module entity and run the callback while the world's
            // scope is set to that module. Use the C++ wrapper's scoped API to
            // ensure the previous scope is restored on exit.
            flecs::entity mod = world.entity(name.c_str());
            mod.add(flecs::Module);
            auto guard = world.scope(mod.id());

            // Execute the original registration logic inside the module scope.
            cb(world);
            (void)guard; // silence unused-warning if any
        });
    }
    void register_callback(RegistrationCallback callback) {
        if (!callback)
            return;
        std::lock_guard<std::mutex> lock(get_mutex());
        get_callbacks().push_back(std::move(callback));
    }

    void register_components_and_systems_with_world(flecs::world &world) {
        std::lock_guard<std::mutex> lock(get_mutex());
        for (RegistrationCallback callback : get_callbacks()) {
            if (callback != nullptr) {
                callback(world);
            }
        }
    }

    std::unordered_map<std::string, ComponentGetter> &get_component_getters() {
        static std::unordered_map<std::string, ComponentGetter> getters;
        return getters;
    }

    std::unordered_map<std::string, ComponentSetter> &get_component_setters() {
        static std::unordered_map<std::string, ComponentSetter> setters;
        return setters;
    }

    std::unordered_map<std::string, ComponentDefaulter> &get_component_defaulters() {
        static std::unordered_map<std::string, ComponentDefaulter> defaulters;
        return defaulters;
    }

    std::unordered_map<std::string, ComponentInspector> &get_component_inspectors() {
        static std::unordered_map<std::string, ComponentInspector> inspectors;
        return inspectors;
    }

} // namespace stagehand
