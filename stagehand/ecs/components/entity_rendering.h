#pragma once

#include <unordered_map>
#include <vector>

#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "stagehand/ecs/components/godot_variants.h"
#include "stagehand/registry.h"
#include "stagehand/utilities/godot_hashes.h" // IWYU pragma: keep

namespace stagehand::entity_rendering {
    GODOT_VARIANT(CustomData, Vector4); // Used as MultiMesh instance custom data in the Entity Rendering (MultiMesh) system

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
        size_t instance_count;
        size_t visible_instance_count;
    };

    // ── Instanced Renderer Types ─────────────────────────────────────────────

    /// Configuration for a single LOD level within an InstancedRendererConfig.
    struct InstancedRendererLODConfig {
        godot::RID mesh_rid;
        float fade_min = 0.0f;
        float fade_max = 0.0f;
        float fade_min_margin = 0.0f;
        float fade_max_margin = 0.0f;
    };

    /// Configuration for one InstancedRenderer3D node.
    /// Each renderer manages RenderingServer instances (one per entity per LOD level).
    struct InstancedRendererConfig {
        godot::RID scenario_rid;
        std::vector<InstancedRendererLODConfig> lod_configs;
        flecs::query<> query;

        /// Per-entity instance RIDs, indexed as [entity_index * lod_count + lod_index].
        /// Managed by the instanced rendering system.
        std::vector<godot::RID> instance_rids;

        /// Number of entities tracked in the previous frame.
        size_t previous_entity_count = 0;
    };

    struct Renderers {
        // Map from renderer type to a map of RIDs to renderer configs.
        // For MultiMesh: key is the MultiMesh RID.
        // For Instanced: key is the first LOD mesh RID (used as a unique identifier).
        std::unordered_map<RendererType, std::unordered_map<godot::RID, MultiMeshRendererConfig>> renderers_by_type;

        // Instanced renderers have a different config type, stored in a separate map.
        std::vector<InstancedRendererConfig> instanced_renderers;
    };
} // namespace stagehand::entity_rendering

inline stagehand::Registry register_entity_rendering_components([](flecs::world &world) {
    world.component<stagehand::entity_rendering::Renderers>().add(flecs::Singleton);
});
