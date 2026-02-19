#include "stagehand/registry.h"

#include <mutex>

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

    std::unordered_map<std::string, ComponentFunctions> &get_component_registry() {
        static std::unordered_map<std::string, ComponentFunctions> registry;
        return registry;
    }

} // namespace stagehand
