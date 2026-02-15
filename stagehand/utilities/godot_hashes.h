/// Centralized hash specializations for Godot types used as unordered_map keys.
#pragma once

#include <cstdint>
#include <functional>

#include <godot_cpp/variant/rid.hpp>

namespace std {
    template <> struct hash<godot::RID> {
        [[nodiscard]] constexpr std::size_t operator()(const godot::RID &r) const noexcept { return std::hash<int64_t>()(r.get_id()); }
    };
} // namespace std
