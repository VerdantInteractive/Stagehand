#pragma once

#include <type_traits>

#include <godot_cpp/variant/basis.hpp>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/entity.h"
#include "stagehand/names.h"
#include "stagehand/registry.h"

namespace stagehand::transform {

    template <typename TransformT> struct TransformSystemTraits {
        static constexpr bool IS_2D = std::is_same_v<TransformT, Transform2D>;
        static constexpr bool IS_3D = std::is_same_v<TransformT, Transform3D>;

        static_assert(IS_2D || IS_3D, "TransformSystemTraits only supports Transform2D and Transform3D.");

        using Transform = TransformT;
        using Position = std::conditional_t<IS_2D, Position2D, Position3D>;
        using Rotation = std::conditional_t<IS_2D, Rotation2D, Rotation3D>;
        using Scale = std::conditional_t<IS_2D, Scale2D, Scale3D>;

        static constexpr const char *DECOMPOSE_SYSTEM_NAME =
            IS_2D ? stagehand::names::systems::TRANSFORM_DECOMPOSE_2D : stagehand::names::systems::TRANSFORM_DECOMPOSE_3D;

        static constexpr const char *COMPOSE_SYSTEM_NAME =
            IS_2D ? stagehand::names::systems::TRANSFORM_COMPOSE_2D : stagehand::names::systems::TRANSFORM_COMPOSE_3D;

        static void decompose_transform(stagehand::entity entity, const Transform &transform, Position &position, Rotation &rotation, Scale &scale) {
            if constexpr (IS_2D) {
                entity.set<Position2D>(Position2D(transform.get_origin()));
                entity.set<Rotation2D>(Rotation2D(transform.get_rotation()));
                entity.set<Scale2D>(Scale2D(transform.get_scale()));
            } else {
                entity.set<Position3D>(Position3D(transform.origin));
                entity.set<Rotation3D>(Rotation3D(transform.basis.get_rotation_quaternion()));
                entity.set<Scale3D>(Scale3D(transform.basis.get_scale()));
            }
        }

        static void compose_transform(Transform &transform, const Position &position, const Rotation &rotation, const Scale &scale) {
            if constexpr (IS_2D) {
                transform.set_origin(position);
                transform.set_rotation_and_scale(rotation, scale);
            } else {
                transform = Transform3D(Basis(rotation).scaled(scale), position);
            }
        }
    };

    template <typename TransformT> void register_transform_decompose_system(flecs::world &world) {
        using Traits = TransformSystemTraits<TransformT>;

        // clang-format off
        world.system<
                const typename Traits::Transform,
                typename Traits::Position,
                typename Traits::Rotation,
                typename Traits::Scale
            >(Traits::DECOMPOSE_SYSTEM_NAME)
            .kind(flecs::PostLoad)
            .multi_threaded()
            .each([](
                stagehand::entity entity,
                const typename Traits::Transform &transform,
                typename Traits::Position &position,
                typename Traits::Rotation &rotation,
                typename Traits::Scale &scale
            ){
                // clang-format on
                Traits::decompose_transform(entity, transform, position, rotation, scale);
            });
    }

    template <typename TransformT> void register_transform_compose_system(flecs::world &world) {
        using Traits = TransformSystemTraits<TransformT>;

        // clang-format off
        world.system<
                typename Traits::Transform,
                const typename Traits::Position,
                const typename Traits::Rotation,
                const typename Traits::Scale
            >(Traits::COMPOSE_SYSTEM_NAME)
            .kind(stagehand::PreRender)
            .template term_at<typename Traits::Transform>().out()
            .multi_threaded()
            .each([](
                stagehand::entity entity,
                typename Traits::Transform &transform,
                const typename Traits::Position &position,
                const typename Traits::Rotation &rotation,
                const typename Traits::Scale &scale
            ){
                // clang-format on
                entity.modify(transform,
                              [&](typename Traits::Transform &current_transform) { Traits::compose_transform(current_transform, position, rotation, scale); });
            });
    }

} // namespace stagehand::transform
