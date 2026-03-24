/// Unit tests for the ENUM component macro.
/// Tests verify:
///   1. Enum struct layout and value semantics.
///   2. Flecs component registration for enum types.
///   3. Getter/setter registration in the component registry.
///   4. Entity-level roundtrips.

#include <cstdint>
#include <flecs.h>
#include <gtest/gtest.h>
#include <string>

#include "stagehand/ecs/components/macros.h"
#include "stagehand/entity.h"
#include "stagehand/registry.h"

namespace test_enum_macro {

    enum class Direction : uint8_t {
        North = 0,
        East = 1,
        South = 2,
        West = 3,
    };
    ENUM(Direction);

    enum class Priority : uint16_t {
        Low = 0,
        Medium = 1,
        High = 2,
        Critical = 3,
    };
    ENUM(Priority, uint16_t);

    enum class Alignment : uint8_t {
        Left = 0,
        Center = 1,
        Right = 2,
    };
    ENUM(Alignment);

    enum class Layer : uint32_t {
        Background = 0,
        Foreground = 1,
        UI = 2,
    };
    ENUM(Layer, uint32_t);

} // namespace test_enum_macro

// ═══════════════════════════════════════════════════════════════════════════════
// Fixture
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct EnumMacroFixture : ::testing::Test {
        flecs::world world;

        void SetUp() override { stagehand::register_components_and_systems_with_world(world); }
    };
} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Value semantics
// ═══════════════════════════════════════════════════════════════════════════════

TEST(EnumValue, EnumDefaultInitializesCorrectly) {
    test_enum_macro::Direction dir{};
    ASSERT_EQ(dir, test_enum_macro::Direction::North);
}

TEST(EnumValue, EnumCanBeAssigned) {
    test_enum_macro::Direction dir = test_enum_macro::Direction::East;
    ASSERT_EQ(dir, test_enum_macro::Direction::East);

    dir = test_enum_macro::Direction::West;
    ASSERT_EQ(dir, test_enum_macro::Direction::West);
}

TEST(EnumValue, EnumCastsToUnderlyingType) {
    test_enum_macro::Priority p = test_enum_macro::Priority::Critical;
    ASSERT_EQ(static_cast<uint16_t>(p), 3u);
}

TEST(EnumValue, EnumCanBeConstructedFromUnderlying) {
    test_enum_macro::Layer layer = static_cast<test_enum_macro::Layer>(2u);
    ASSERT_EQ(layer, test_enum_macro::Layer::UI);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Flecs registration
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EnumMacroFixture, EnumIsRegisteredInWorld) {
    auto comp = world.component<test_enum_macro::Direction>();
    ASSERT_NE(comp.id(), 0u);
}

TEST_F(EnumMacroFixture, EnumWithExplicitTypeIsRegistered) {
    auto comp = world.component<test_enum_macro::Priority>();
    ASSERT_NE(comp.id(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Entity roundtrips
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EnumMacroFixture, EnumComponentOnEntityRoundtrip) {
    auto entity = world.entity();
    entity.set<test_enum_macro::Direction>(test_enum_macro::Direction::South);

    const test_enum_macro::Direction *dir = entity.try_get<test_enum_macro::Direction>();
    ASSERT_NE(dir, nullptr);
    ASSERT_EQ(*dir, test_enum_macro::Direction::South);
}

TEST_F(EnumMacroFixture, EnumComponentOverwrite) {
    auto entity = world.entity();
    entity.set<test_enum_macro::Priority>(test_enum_macro::Priority::Low);
    entity.set<test_enum_macro::Priority>(test_enum_macro::Priority::High);

    const test_enum_macro::Priority *p = entity.try_get<test_enum_macro::Priority>();
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(*p, test_enum_macro::Priority::High);
}

TEST_F(EnumMacroFixture, MultipleEnumsOnSameEntity) {
    auto entity = world.entity();
    entity.set<test_enum_macro::Direction>(test_enum_macro::Direction::West);
    entity.set<test_enum_macro::Priority>(test_enum_macro::Priority::Medium);
    entity.set<test_enum_macro::Layer>(test_enum_macro::Layer::UI);

    ASSERT_EQ(*entity.try_get<test_enum_macro::Direction>(), test_enum_macro::Direction::West);
    ASSERT_EQ(*entity.try_get<test_enum_macro::Priority>(), test_enum_macro::Priority::Medium);
    ASSERT_EQ(*entity.try_get<test_enum_macro::Layer>(), test_enum_macro::Layer::UI);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Getter/setter registration
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EnumMacroFixture, EnumGetterIsRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("Direction");
    ASSERT_NE(it, registry.end());
    ASSERT_TRUE(static_cast<bool>(it->second.getter));
}

TEST_F(EnumMacroFixture, EnumSetterIsRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("Direction");
    ASSERT_NE(it, registry.end());
    ASSERT_TRUE(static_cast<bool>(it->second.setter));
}

TEST_F(EnumMacroFixture, EnumGetterAndSetterAreRegistered) {
    auto &registry = stagehand::get_component_registry();
    auto it = registry.find("Alignment");
    ASSERT_NE(it, registry.end());
    ASSERT_TRUE(static_cast<bool>(it->second.getter));
    ASSERT_TRUE(static_cast<bool>(it->second.setter));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Query with enum component
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(EnumMacroFixture, EnumComponentQueryWorks) {
    world.entity().set<test_enum_macro::Direction>(test_enum_macro::Direction::North);
    world.entity().set<test_enum_macro::Direction>(test_enum_macro::Direction::South);
    world.entity().set<test_enum_macro::Direction>(test_enum_macro::Direction::South);

    int south_count = 0;
    world.each([&south_count](const test_enum_macro::Direction &dir) {
        if (dir == test_enum_macro::Direction::South) {
            ++south_count;
        }
    });

    ASSERT_EQ(south_count, 2);
}
