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

    struct Renderers {
        // Map from renderer type to a map of prefab names to multimesh RIDs.
        // Key for the inner map is a string representation of the MultiMesh RID ID (we group all prefab queries for a single MultiMesh into one
        // MultiMeshRendererConfig). Use godot::RID as the inner map key so each MultiMesh RID maps directly to its MultiMeshRendererConfig. Ensure
        // std::hash<godot::RID> is available below.
        std::unordered_map<RendererType, std::unordered_map<godot::RID, MultiMeshRendererConfig>> renderers_by_type;
    };
} // namespace stagehand::entity_rendering

inline stagehand::Registry register_entity_rendering_components([](flecs::world &world) {
    world.component<stagehand::entity_rendering::Renderers>().add(flecs::Singleton);
});
