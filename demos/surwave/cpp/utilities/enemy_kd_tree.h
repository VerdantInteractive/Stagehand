#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include <godot_cpp/variant/vector2.hpp>

// Based on the idea from "Optimizing the brute force implementation of Boids simulation using a k-d tree"
// Stefan Borkovski, Jožef Stefan International Postgraduate School, Ljubljana, Slovenia

namespace enemy_kd_tree {

    struct Node2D {
        std::int32_t entity_index = -1;
        godot::Vector2 point = godot::Vector2(0.0f, 0.0f);
        std::int32_t axis = 0;
        std::int32_t left_child = -1;
        std::int32_t right_child = -1;
    };

    class KdTree2D {
    public:
        KdTree2D() : root_index(-1) {}

        void clear() {
            nodes.clear();
            permutation.clear();
            root_index = -1;
        }

        [[nodiscard]] bool empty() const {
            return root_index < 0;
        }

        template <typename PositionAccessor>
        void build(std::int32_t entity_count, const PositionAccessor& position_accessor) {
            clear();
            if (entity_count <= 0) {
                return;
            }

            const std::size_t node_count = static_cast<std::size_t>(entity_count);
            nodes.resize(node_count);
            permutation.resize(node_count);
            for (std::int32_t index = 0; index < entity_count; ++index) {
                permutation[static_cast<std::size_t>(index)] = index;
            }

            root_index = build_recursive(0, entity_count, 0, position_accessor);
        }

        template <typename PositionAccessor>
        void refresh_points(std::int32_t entity_count, const PositionAccessor& position_accessor) {
            if (entity_count <= 0) {
                return;
            }

            const std::size_t required_size = static_cast<std::size_t>(entity_count);
            if (required_size > nodes.size()) {
                return;
            }

            for (std::int32_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                nodes[static_cast<std::size_t>(entity_index)].point = position_accessor(entity_index);
            }
        }

        template <typename Visitor>
        void radius_query(const godot::Vector2& origin, godot::real_t radius_squared, const Visitor& visitor, std::int32_t max_results = -1) const {
            if (root_index < 0 || radius_squared <= 0.0f) {
                return;
            }

            std::int32_t remaining = max_results;
            query_recursive(root_index, origin, radius_squared, visitor, remaining);
        }

    private:
        template <typename PositionAccessor>
        class AxisComparator {
        public:
            AxisComparator(const PositionAccessor* accessor, std::int32_t split_axis) : accessor_pointer(accessor), axis(split_axis) {}

            bool operator()(std::int32_t lhs, std::int32_t rhs) const {
                const godot::Vector2 lhs_position = (*accessor_pointer)(lhs);
                const godot::Vector2 rhs_position = (*accessor_pointer)(rhs);
                if (axis == 0) {
                    return lhs_position.x < rhs_position.x;
                }
                return lhs_position.y < rhs_position.y;
            }

        private:
            const PositionAccessor* accessor_pointer;
            std::int32_t axis;
        };

        template <typename PositionAccessor>
        std::int32_t build_recursive(std::int32_t start, std::int32_t end, std::int32_t depth, const PositionAccessor& position_accessor) {
            if (start >= end) {
                return -1;
            }

            const std::int32_t axis = depth & 1;
            const std::int32_t median = start + (end - start) / 2;
            AxisComparator<PositionAccessor> comparator(&position_accessor, axis);
            std::nth_element(
                permutation.begin() + start,
                permutation.begin() + median,
                permutation.begin() + end,
                comparator);

            const std::size_t median_offset = static_cast<std::size_t>(median);
            const std::int32_t entity_index = permutation[median_offset];
            Node2D& node = nodes[static_cast<std::size_t>(entity_index)];
            node.entity_index = entity_index;
            node.axis = axis;
            node.point = position_accessor(entity_index);
            node.left_child = build_recursive(start, median, depth + 1, position_accessor);
            node.right_child = build_recursive(median + 1, end, depth + 1, position_accessor);
            return entity_index;
        }

        template <typename Visitor>
        bool query_recursive(
            std::int32_t node_index,
            const godot::Vector2& origin,
            godot::real_t radius_squared,
            const Visitor& visitor,
            std::int32_t& remaining) const {
            if (node_index < 0) {
                return true;
            }

            if (remaining == 0) {
                return false;
            }

            const Node2D& node = nodes[static_cast<std::size_t>(node_index)];
            const godot::Vector2 offset = node.point - origin;
            const godot::real_t distance_squared = offset.length_squared();
            if (distance_squared <= radius_squared) {
                visitor(node.entity_index, node.point, distance_squared);
                if (remaining > 0) {
                    remaining -= 1;
                    if (remaining == 0) {
                        return false;
                    }
                }
            }

            const godot::real_t axis_delta = node.axis == 0 ? origin.x - node.point.x : origin.y - node.point.y;
            const std::int32_t near_child = axis_delta <= 0.0f ? node.left_child : node.right_child;
            const std::int32_t far_child = axis_delta <= 0.0f ? node.right_child : node.left_child;

            if (!query_recursive(near_child, origin, radius_squared, visitor, remaining)) {
                return false;
            }
            const godot::real_t axis_distance_squared = axis_delta * axis_delta;
            if (axis_distance_squared <= radius_squared) {
                if (!query_recursive(far_child, origin, radius_squared, visitor, remaining)) {
                    return false;
                }
            }
            return true;
        }

        std::vector<Node2D> nodes;
        std::vector<std::int32_t> permutation;
        std::int32_t root_index;
    };

} // namespace enemy_kd_tree
