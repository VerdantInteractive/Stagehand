#pragma once
#include <flecs.h>
#include <type_traits>
#include <utility>

namespace stagehand {

    /// A lightweight wrapper around flecs::entity.
    /// Has the same memory layout as flecs::entity and can be implicitly converted.
    struct entity : public flecs::entity {
        // Inherit constructors
        using flecs::entity::entity;
        entity(flecs::entity e) : flecs::entity(e) {}

      public:
        /// Helper to modify a component in-place.
        /// Usage: e.modify<Position>([](Position& p) { p.x += 1; });
        template <typename T, typename Func> inline entity &modify(Func &&func) {
            T *ptr = this->try_get_mut<T>();
            if (ptr) {
                return modify(*ptr, std::forward<Func>(func));
            }
            return *this;
        }

        /// Optimized modify helper when you already have a reference to the component.
        /// Usage: e.modify(p, [](Position& val) { val.x += 1; });
        template <typename T, typename Func> inline entity &modify(T &component, Func &&func) {
            func(component);
            this->modified<T>();
            return *this;
        }
    };
} // namespace stagehand
