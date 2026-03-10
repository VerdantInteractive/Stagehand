#pragma once

#include "stagehand/registry.h"

namespace stagehand {
    /// A trait that can be added to the Changed... tag components to indicate that they are used for tracking changes to components they are associated with.
    struct IsChangeDetectionTag {};
    inline auto register_change_detection_trait =
        stagehand::ComponentRegistrar<IsChangeDetectionTag>([](flecs::world &world) { world.component<IsChangeDetectionTag>().add(flecs::Trait); });

    namespace internal {
        template <typename T, typename = void> struct component_has_change_tag : std::false_type {};

        template <typename T> struct component_has_change_tag<T, std::void_t<typename T::ChangeTag>> : std::true_type {};

        template <typename T> inline constexpr bool component_has_change_tag_v = component_has_change_tag<T>::value;

        template <typename T, typename ChangeTag> inline void register_change_detection_for_component(flecs::world &world) {
            world.component<ChangeTag>().template add<stagehand::IsChangeDetectionTag>().add(flecs::CanToggle);
            world.component<T>().add(flecs::With, world.component<ChangeTag>());
        }

        template <typename T> inline void mark_component_changed_if_needed(flecs::entity entity) {
            if constexpr (component_has_change_tag_v<T>) {
                entity.template enable<typename T::ChangeTag>();
            }
        }

        template <typename T> inline void register_change_detection_if_needed(flecs::world &world) {
            if constexpr (component_has_change_tag_v<T>) {
                using ChangeTag = typename T::ChangeTag;
                register_change_detection_for_component<T, ChangeTag>(world);
            }
        }
    } // namespace internal
} // namespace stagehand
