/// Unit tests for world-level component and system operations that do not require a running Godot engine instance.

#include <flecs.h>
#include <gtest/gtest.h>
#include <string>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"

namespace unit_world_component_ops {
    struct UnitAccumulatorParams {
        bool has_amount = false;
        int amount = 0;
    };

    INT32(UnitTickCount, 0);
    INT32(UnitAccumulatorValue, 0);
    TAG(UnitTestTag);

    REGISTER([](flecs::world &world) {
        world.set<UnitTickCount>({0});
        world.set<UnitAccumulatorValue>({0});

        world.system("unit_world_component_ops::TickCounter").kind(flecs::OnUpdate).run([](flecs::iter &it) {
            UnitTickCount &tick_count = it.world().ensure<UnitTickCount>();
            tick_count.value++;
        });

        world.system("unit_world_component_ops::Accumulator").kind(0).run([](flecs::iter &it) {
            const UnitAccumulatorParams *parameters = static_cast<const UnitAccumulatorParams *>(it.param());
            if (parameters == nullptr || !parameters->has_amount) {
                return;
            }

            UnitAccumulatorValue &accumulator = it.world().ensure<UnitAccumulatorValue>();
            accumulator.value += parameters->amount;
        });
    });
} // namespace unit_world_component_ops

namespace {
    struct WorldComponentOpsFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

TEST_F(WorldComponentOpsFixture, NonExistentSystemLookupReturnsInvalidEntity) {
    flecs::entity missing = world.lookup("CompletelyFakeSystem");
    ASSERT_FALSE(missing.is_valid());
}

TEST_F(WorldComponentOpsFixture, ValidEntityThatIsNotSystemDoesNotHaveSystemTag) {
    flecs::entity entity = world.entity("MyTestEntity");
    ASSERT_TRUE(entity.is_valid());
    ASSERT_FALSE(entity.has(flecs::System));
}

TEST_F(WorldComponentOpsFixture, OnDemandSystemWithWrongParameterKeyLeavesAccumulatorUnchanged) {
    unit_world_component_ops::UnitAccumulatorValue &accumulator = world.ensure<unit_world_component_ops::UnitAccumulatorValue>();
    accumulator.value = 0;

    flecs::entity system_entity = world.lookup("unit_world_component_ops::Accumulator");
    ASSERT_TRUE(system_entity.is_valid());
    flecs::system system = world.system(system_entity);

    unit_world_component_ops::UnitAccumulatorParams parameters;
    parameters.has_amount = false;
    parameters.amount = 99;
    system.run(0.0f, static_cast<void *>(&parameters));

    ASSERT_EQ(world.get<unit_world_component_ops::UnitAccumulatorValue>().value, 0);
}

TEST_F(WorldComponentOpsFixture, OnDemandSystemWithEmptyParametersLeavesAccumulatorUnchanged) {
    unit_world_component_ops::UnitAccumulatorValue &accumulator = world.ensure<unit_world_component_ops::UnitAccumulatorValue>();
    accumulator.value = 0;

    flecs::entity system_entity = world.lookup("unit_world_component_ops::Accumulator");
    ASSERT_TRUE(system_entity.is_valid());
    flecs::system system = world.system(system_entity);

    unit_world_component_ops::UnitAccumulatorParams parameters;
    parameters.has_amount = false;
    system.run(0.0f, static_cast<void *>(&parameters));

    ASSERT_EQ(world.get<unit_world_component_ops::UnitAccumulatorValue>().value, 0);
}

TEST_F(WorldComponentOpsFixture, UnknownComponentNameIsNotPresentInRegistry) {
    const auto &registry = stagehand::get_component_registry();
    ASSERT_EQ(registry.find("TotallyNonExistent"), registry.end());
}

TEST_F(WorldComponentOpsFixture, RapidInvalidOperationsDoNotCorruptWorldState) {
    const auto &registry = stagehand::get_component_registry();

    for (int index = 0; index < 20; ++index) {
        const std::string system_name = "NoSuchSystem" + std::to_string(index);
        const std::string component_name = "NoSuchComponent" + std::to_string(index);

        flecs::entity system = world.lookup(system_name.c_str());
        ASSERT_FALSE(system.is_valid());

        ASSERT_EQ(registry.find(component_name), registry.end());
    }
}

TEST_F(WorldComponentOpsFixture, WorldProgressStillRunsSystemsAfterInvalidOperations) {
    unit_world_component_ops::UnitTickCount &tick_count = world.ensure<unit_world_component_ops::UnitTickCount>();
    tick_count.value = 0;

    for (int index = 0; index < 20; ++index) {
        const std::string system_name = "NoSuchSystem" + std::to_string(index);
        const std::string component_name = "NoSuchComponent" + std::to_string(index);

        flecs::entity system = world.lookup(system_name.c_str());
        ASSERT_FALSE(system.is_valid());

        const auto &registry = stagehand::get_component_registry();
        ASSERT_EQ(registry.find(component_name), registry.end());
    }

    world.progress(0.016f);
    ASSERT_EQ(world.get<unit_world_component_ops::UnitTickCount>().value, 1);
}

TEST_F(WorldComponentOpsFixture, SingletonTagAddHasRemoveWorks) {
    const flecs::entity_t component_id = world.component<unit_world_component_ops::UnitTestTag>().id();
    flecs::entity singleton = world.entity(component_id);

    ASSERT_FALSE(singleton.has(component_id));

    singleton.add(component_id);
    ASSERT_TRUE(singleton.has(component_id));

    singleton.remove(component_id);
    ASSERT_FALSE(singleton.has(component_id));
}
