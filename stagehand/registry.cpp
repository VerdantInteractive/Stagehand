#include <mutex>

#include "stagehand/registry.h"

namespace stagehand {

    namespace
    {
        std::vector<RegistrationCallback>& get_callbacks()
        {
            static std::vector<RegistrationCallback> callbacks;
            return callbacks;
        }

        std::mutex& get_mutex()
        {
            static std::mutex mtx;
            return mtx;
        }
    }

    Registry::Registry(RegistrationCallback callback)
    {
        std::lock_guard<std::mutex> lock(get_mutex());
        get_callbacks().push_back(callback);
    }

    void register_components_and_systems_with_world(flecs::world& world)
    {
        std::lock_guard<std::mutex> lock(get_mutex());
        for (RegistrationCallback callback : get_callbacks())
        {
            if (callback != nullptr)
            {
                callback(world);
            }
        }
    }

    std::unordered_map<std::string, ComponentGetter>& get_component_getters()
    {
        static std::unordered_map<std::string, ComponentGetter> getters;
        return getters;
    }

    std::unordered_map<std::string, ComponentSetter>& get_component_setters()
    {
        static std::unordered_map<std::string, ComponentSetter> setters;
        return setters;
    }

} // namespace stagehand
