#pragma once

#include "stagehand/ecs/components/macros.h"

namespace stagehand_demos::surwave {

    FLOAT_(HitPoints);
    FLOAT_(HitRadius);
    FLOAT_(MovementSpeed);
    FLOAT_(MeleeDamage);
    FLOAT_(AnimationFrameOffset);
    FLOAT_(DeathTimer);
    FLOAT_(HitReactionTimer);
    FLOAT_(HFlipTimer);
    FLOAT_(VFlipTimer);
    FLOAT_(ProjectileHitTimeout);
    FLOAT_(ShockwaveHitTimeout);

} // namespace stagehand_demos::surwave
