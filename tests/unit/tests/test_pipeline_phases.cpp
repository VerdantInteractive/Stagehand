/// Unit tests for custom pipeline phases.
/// Tests verify:
///   1. All custom phases are registered as valid entities in the world.
///   2. Phases have the Phase tag.
///   3. Phase ordering is correct (dependency chain).
///   4. Systems assigned to custom phases execute during world.progress().

#include <flecs.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace {
    struct PipelinePhasesFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Phase registration
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PipelinePhasesFixture, OnEarlyUpdateIsValid) {
    ASSERT_TRUE(stagehand::OnEarlyUpdate.is_valid());
    ASSERT_TRUE(stagehand::OnEarlyUpdate.has(flecs::Phase));
}

TEST_F(PipelinePhasesFixture, OnLateUpdateIsValid) {
    ASSERT_TRUE(stagehand::OnLateUpdate.is_valid());
    ASSERT_TRUE(stagehand::OnLateUpdate.has(flecs::Phase));
}

TEST_F(PipelinePhasesFixture, PreRenderIsValid) {
    ASSERT_TRUE(stagehand::PreRender.is_valid());
    ASSERT_TRUE(stagehand::PreRender.has(flecs::Phase));
}

TEST_F(PipelinePhasesFixture, OnRenderIsValid) {
    ASSERT_TRUE(stagehand::OnRender.is_valid());
    ASSERT_TRUE(stagehand::OnRender.has(flecs::Phase));
}

TEST_F(PipelinePhasesFixture, PostRenderIsValid) {
    ASSERT_TRUE(stagehand::PostRender.is_valid());
    ASSERT_TRUE(stagehand::PostRender.has(flecs::Phase));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Phases can be looked up by name
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PipelinePhasesFixture, PhasesAreLookableByName) {
    ASSERT_TRUE(world.lookup(stagehand::names::phases::ON_EARLY_UPDATE).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::phases::ON_LATE_UPDATE).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::phases::PRE_RENDER).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::phases::ON_RENDER).is_valid());
    ASSERT_TRUE(world.lookup(stagehand::names::phases::POST_RENDER).is_valid());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Phase ordering: systems run in correct order
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PipelinePhasesFixture, PhasesExecuteInCorrectOrder) {
    auto order = std::make_shared<std::vector<std::string>>();

    world.system("test::on_early_update").kind(stagehand::OnEarlyUpdate).run([order](flecs::iter &) { order->push_back("OnEarlyUpdate"); });

    world.system("test::on_update").kind(flecs::OnUpdate).run([order](flecs::iter &) { order->push_back("OnUpdate"); });

    world.system("test::on_late_update").kind(stagehand::OnLateUpdate).run([order](flecs::iter &) { order->push_back("OnLateUpdate"); });

    world.system("test::pre_render").kind(stagehand::PreRender).run([order](flecs::iter &) { order->push_back("PreRender"); });

    world.system("test::on_render").kind(stagehand::OnRender).run([order](flecs::iter &) { order->push_back("OnRender"); });

    world.system("test::post_render").kind(stagehand::PostRender).run([order](flecs::iter &) { order->push_back("PostRender"); });

    world.progress(0.016f);

    ASSERT_GE(order->size(), 6u);

    // Find the index of each phase in the execution order
    auto index_of = [&order](const std::string &name) -> int {
        for (int i = 0; i < static_cast<int>(order->size()); ++i) {
            if ((*order)[i] == name) {
                return i;
            }
        }
        return -1;
    };

    int early = index_of("OnEarlyUpdate");
    int update = index_of("OnUpdate");
    int late = index_of("OnLateUpdate");
    int pre_render = index_of("PreRender");
    int on_render = index_of("OnRender");
    int post_render = index_of("PostRender");

    ASSERT_NE(early, -1);
    ASSERT_NE(update, -1);
    ASSERT_NE(late, -1);
    ASSERT_NE(pre_render, -1);
    ASSERT_NE(on_render, -1);
    ASSERT_NE(post_render, -1);

    // Verify ordering
    EXPECT_LT(early, update) << "OnEarlyUpdate should run before OnUpdate";
    EXPECT_LT(update, late) << "OnUpdate should run before OnLateUpdate";
    EXPECT_LT(late, pre_render) << "OnLateUpdate should run before PreRender";
    EXPECT_LT(pre_render, on_render) << "PreRender should run before OnRender";
    EXPECT_LT(on_render, post_render) << "OnRender should run before PostRender";
}

// ═══════════════════════════════════════════════════════════════════════════════
// Custom phase system actually runs
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(PipelinePhasesFixture, SystemsInCustomPhasesExecuteOnProgress) {
    int counter = 0;

    world.system("test::pre_render_counter").kind(stagehand::PreRender).run([&counter](flecs::iter &) { ++counter; });

    world.progress(0.016f);
    ASSERT_EQ(counter, 1);

    world.progress(0.016f);
    ASSERT_EQ(counter, 2);
}

TEST_F(PipelinePhasesFixture, PostRenderSystemRunsAfterOtherPhases) {
    bool pre_render_ran = false;
    bool post_render_ran = false;
    bool post_render_after_pre = false;

    world.system("test::pre_render_flag").kind(stagehand::PreRender).run([&pre_render_ran](flecs::iter &) { pre_render_ran = true; });

    world.system("test::post_render_check").kind(stagehand::PostRender).run([&pre_render_ran, &post_render_ran, &post_render_after_pre](flecs::iter &) {
        post_render_ran = true;
        post_render_after_pre = pre_render_ran;
    });

    world.progress(0.016f);

    ASSERT_TRUE(pre_render_ran);
    ASSERT_TRUE(post_render_ran);
    ASSERT_TRUE(post_render_after_pre);
}
