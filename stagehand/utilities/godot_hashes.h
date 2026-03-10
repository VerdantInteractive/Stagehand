/// Centralized hash specializations for Godot types used as unordered_map keys.
#pragma once

#include <cstdint>
#include <functional>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace std {
    template <> struct hash<godot::RID> {
        [[nodiscard]] std::size_t operator()(const godot::RID &r) const noexcept { return std::hash<uint64_t>()(r.get_id()); }
    };

    template <> struct hash<godot::StringName> {
        [[nodiscard]] std::size_t operator()(const godot::StringName &s) const noexcept {
            return static_cast<std::size_t>(s.hash());
        }
    };
} // namespace std
