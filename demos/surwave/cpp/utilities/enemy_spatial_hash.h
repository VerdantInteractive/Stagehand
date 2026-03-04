#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace enemy_spatial_hash {

    struct GridCellKey {
        std::int32_t x;
        std::int32_t y;

        bool operator==(const GridCellKey& other) const {
            return x == other.x && y == other.y;
        }
    };

    struct GridCellKeyHasher {
        std::size_t operator()(const GridCellKey& key) const {
            const std::uint64_t packed_x = static_cast<std::uint64_t>(static_cast<std::uint32_t>(key.x));
            const std::uint64_t packed_y = static_cast<std::uint64_t>(static_cast<std::uint32_t>(key.y));
            return static_cast<std::size_t>((packed_x << 32U) ^ packed_y);
        }
    };

    using CellOccupants = std::vector<std::int32_t>;
    using SpatialHash = std::unordered_map<GridCellKey, CellOccupants, GridCellKeyHasher>;

    inline GridCellKey make_key(const godot::Vector2& position, godot::real_t cell_size) {
        const godot::real_t cell_x = godot::Math::floor(position.x / cell_size);
        const godot::real_t cell_y = godot::Math::floor(position.y / cell_size);
        return GridCellKey{ static_cast<std::int32_t>(cell_x), static_cast<std::int32_t>(cell_y) };
    }

    template <typename PositionAccessor>
    inline void populate_spatial_hash(
        std::int32_t entity_count,
        godot::real_t cell_size,
        const PositionAccessor& position_accessor,
        std::vector<GridCellKey>& entity_cells,
        SpatialHash& spatial_hash) {

        const std::size_t required_size = static_cast<std::size_t>(entity_count);
        entity_cells.clear();
        entity_cells.resize(required_size);

        spatial_hash.clear();
        spatial_hash.reserve(required_size * 2U);

        for (std::int32_t entity_index = 0; entity_index < entity_count; ++entity_index) {
            const godot::Vector2 position_value = position_accessor(entity_index);
            const GridCellKey key = make_key(position_value, cell_size);
            entity_cells[static_cast<std::size_t>(entity_index)] = key;
            spatial_hash[key].push_back(entity_index);
        }
    }

} // namespace enemy_spatial_hash
