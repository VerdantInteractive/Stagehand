#pragma once

#include <limits>

#include <godot_cpp/core/math_defs.hpp>

namespace stagehand_demos::surwave {
    inline constexpr godot::real_t kEnemyDeathInvulnerableHitPoints = std::numeric_limits<godot::real_t>::max();
}
