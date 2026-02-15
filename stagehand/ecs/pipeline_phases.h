#pragma once

#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand {
    inline flecs::entity OnEarlyUpdate;
    inline flecs::entity OnLateUpdate;
    inline flecs::entity PreRender;
    inline flecs::entity OnRender;
    inline flecs::entity PostRender;

    inline stagehand::Registry register_pipeline_phases([](flecs::world &world) {
        OnEarlyUpdate = world.entity(stagehand::names::phases::ON_EARLY_UPDATE).add(flecs::Phase).depends_on(flecs::PreUpdate);
        world.entity(flecs::OnUpdate).depends_on(OnEarlyUpdate);
        OnLateUpdate = world.entity(stagehand::names::phases::ON_LATE_UPDATE).add(flecs::Phase).depends_on(flecs::OnUpdate);
        PreRender = world.entity(stagehand::names::phases::PRE_RENDER).add(flecs::Phase).depends_on(flecs::OnStore);
        OnRender = world.entity(stagehand::names::phases::ON_RENDER).add(flecs::Phase).depends_on(PreRender);
        PostRender = world.entity(stagehand::names::phases::POST_RENDER).add(flecs::Phase).depends_on(OnRender);
    });
} // namespace stagehand
