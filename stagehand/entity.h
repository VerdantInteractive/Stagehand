#pragma once
#include <flecs.h>
#include <type_traits>
#include <utility>

namespace stagehand {

    // SFINAE check to detect if a component has a ChangeTag type alias
    template <typename T, typename = void> struct has_change_tag : std::false_type {};

    template <typename T> struct has_change_tag<T, std::void_t<typename T::ChangeTag>> : std::true_type {};

    /// A zero-overhead wrapper around flecs::entity that automatically manages HasChanged tags.
    /// It has the same memory layout as flecs::entity and can be implicitly converted.
    struct entity : public flecs::entity {
        // Inherit constructors
        using flecs::entity::entity;
        entity(flecs::entity e) : flecs::entity(e) {}

      private:
        template <typename T> inline void mark_changed() const {
            if constexpr (has_change_tag<T>::value) {
                this->enable<typename T::ChangeTag>();
            }
        }

      public:
        /// Shadowed set method that applies the component and its change tag.
        template <typename T> inline entity &set(const T &value) {
            flecs::entity::set<T>(value);
            mark_changed<T>();
            return *this;
        }

        /// Shadowed set method for rvalues.
        template <typename T> inline entity &set(T &&value) {
            flecs::entity::set<T>(std::forward<T>(value));
            mark_changed<std::decay_t<T>>();
            return *this;
        }

        /// Helper to modify a component in-place and trigger the change tag.
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
            mark_changed<T>();
            return *this;
        }
    };
} // namespace stagehand
