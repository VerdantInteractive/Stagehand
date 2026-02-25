/// Centralized hash specializations for Godot types used as unordered_map keys.
#pragma once

#include <cstdint>
#include <functional>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace stagehand {
    struct StringNameHasher {
        std::size_t operator()(const godot::StringName &s) const {
            return s.hash();
        }
    };
} // namespace stagehand

namespace std {
    template <> struct hash<godot::RID> {
        [[nodiscard]] constexpr std::size_t operator()(const godot::RID &r) const noexcept { return std::hash<int64_t>()(r.get_id()); }
    };
} // namespace std
