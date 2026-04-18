#pragma once

#include "stagehand/ecs/components/physics.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand {
    REGISTER([](flecs::world &world) {
        world.prefab(stagehand::names::prefabs::XPBD_CLOTH_3D)
            .is_a(world.lookup(stagehand::names::prefabs::ENTITY_3D))
            .set<stagehand::physics::PhysicsBodyType>(stagehand::physics::PhysicsBodyType::XPBD3D)
            .set<stagehand::physics::XPBDCloth3DConfig>(stagehand::physics::XPBDCloth3DConfig())
            .set<stagehand::physics::XPBDCloth3DGrab>(stagehand::physics::XPBDCloth3DGrab());
    });
} // namespace stagehand
