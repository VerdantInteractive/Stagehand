// Centralized hash specializations for Godot types used as unordered_map keys
#pragma once

#include <functional>
#include <cstdint>

#include <godot_cpp/variant/rid.hpp>

namespace std
{
    template <>
    struct hash<godot::RID>
    {
        std::size_t operator()(const godot::RID& r) const noexcept
        {
            return std::hash<int64_t>()(r.get_id());
        }
    };
}
