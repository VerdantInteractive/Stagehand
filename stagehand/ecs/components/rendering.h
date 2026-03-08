#pragma once

#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "flecs.h"

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/ecs/components/macros.h"
#include "stagehand/registry.h"
#include "stagehand/utilities/godot_hashes.h" // IWYU pragma: keep

namespace stagehand::rendering {
    GODOT_VARIANT_(CustomData, Vector4); // Used as MultiMesh instance custom data in the Entity Rendering (MultiMesh) system

    enum class RendererType {
        Instanced,
        MultiMesh,
    };

    struct MultiMeshRendererConfig {
        godot::RID rid;
        // One MultiMeshInstance can render multiple prefab types. Store a list of queries (one per prefab) for each renderer.
        std::vector<flecs::query<>> queries;
        godot::MultiMesh::TransformFormat transform_format;
        bool use_colors;
        bool use_custom_data;
        uint32_t instance_count;
        uint32_t visible_instance_count;
    };

    // ── Instanced Renderer Types ─────────────────────────────────────────────

    /// Configuration for a single LOD level within an InstancedRendererConfig.
    struct InstancedRendererLODConfig {
        godot::RID mesh_rid;
        float visibility_range_begin = 0.0f;
        float visibility_range_end = 0.0f;
        float visibility_range_begin_margin = 0.0f;
        float visibility_range_end_margin = 0.0f;
        godot::RenderingServer::VisibilityRangeFadeMode visibility_range_fade_mode = godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF;
    };

    /// Configuration for one InstancedRenderer3D node.
    /// Each renderer manages RenderingServer instances (one per entity per LOD level).
    struct InstancedRendererConfig {
        struct UniformInitConfig {
            int value_field_index;
            godot::StringName parameter_name;
        };

        struct UniformUpdateConfig {
            int value_field_index;
            godot::StringName parameter_name;
            flecs::query<> query;
        };

        godot::RID scenario_rid;
        godot::RID material_rid;
        std::vector<InstancedRendererLODConfig> lod_configs;
        flecs::query<> reconcile_query;
        flecs::query<> transform_update_query;
        std::vector<UniformInitConfig> initial_uniforms;
        std::vector<UniformUpdateConfig> uniform_updates;

        /// Per-slot instance RIDs, indexed as [slot_index * lod_count + lod_index].
        /// Slots remain allocated so their RIDs can be reused across entity churn.
        std::vector<godot::RID> instance_rids;
        std::vector<ecs_entity_t> slot_entities;
        std::vector<uint64_t> slot_generations;
        std::vector<uint64_t> slot_created_generations;
        std::vector<uint32_t> free_slots;
        /// Direct lookup from the stripped Flecs entity id to a renderer slot.
        /// The slot is validated against slot_entities before use so recycled Flecs IDs cannot accidentally reuse a stale slot.
        std::vector<uint32_t> slot_by_entity_id;

        static constexpr uint32_t INVALID_SLOT = std::numeric_limits<uint32_t>::max();

        uint64_t current_generation = 0;
        uint32_t active_entity_count = 0;
    };

    /// A trait that can be added to components to indicate that they are used as instance uniform parameters in the InstancedRenderer3D
    TAG(IsInstanceUniform).then([](auto c) { c.add(flecs::Trait); });

    struct Renderers {
        // Map from renderer type to a map of RIDs to renderer configs.
        // For MultiMesh: key is the MultiMesh RID.
        // For Instanced: key is the first LOD mesh RID (used as a unique identifier).
        std::unordered_map<RendererType, std::unordered_map<godot::RID, MultiMeshRendererConfig>> renderers_by_type;

        // Instanced renderers have a different config type, stored in a separate map.
        std::vector<InstancedRendererConfig> instanced_renderers;
    };
} // namespace stagehand::rendering

REGISTER([](flecs::world &world) { world.component<stagehand::rendering::Renderers>().add(flecs::Singleton); });
