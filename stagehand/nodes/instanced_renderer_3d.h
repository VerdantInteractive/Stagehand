#pragma once

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "stagehand/ecs/components/entity_rendering.h"

/// Resource representing a single LOD level on an InstancedRenderer3D.
class InstancedRenderer3DLODConfiguration : public godot::Resource {
    GDCLASS(InstancedRenderer3DLODConfiguration, godot::Resource)

  public:
    void set_mesh(const godot::Ref<godot::Mesh> &p_mesh) { mesh = p_mesh; }
    [[nodiscard]] godot::Ref<godot::Mesh> get_mesh() const { return mesh; }

    void set_fade_min(float p_value) { fade_min = p_value; }
    [[nodiscard]] float get_fade_min() const { return fade_min; }

    void set_fade_max(float p_value) { fade_max = p_value; }
    [[nodiscard]] float get_fade_max() const { return fade_max; }

    void set_fade_min_margin(float p_value) { fade_min_margin = p_value; }
    [[nodiscard]] float get_fade_min_margin() const { return fade_min_margin; }

    void set_fade_max_margin(float p_value) { fade_max_margin = p_value; }
    [[nodiscard]] float get_fade_max_margin() const { return fade_max_margin; }

    void set_visibility_fade_mode(godot::RenderingServer::VisibilityRangeFadeMode p_value) { visibility_fade_mode = p_value; }
    [[nodiscard]] godot::RenderingServer::VisibilityRangeFadeMode get_visibility_fade_mode() const { return visibility_fade_mode; }

  protected:
    static void _bind_methods();

  private:
    godot::Ref<godot::Mesh> mesh;
    float fade_min = 0.0f;
    float fade_max = 0.0f;
    float fade_min_margin = 0.0f;
    float fade_max_margin = 0.0f;
    godot::RenderingServer::VisibilityRangeFadeMode visibility_fade_mode = godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF;
};

/// A Node3D that renders ECS entities with Transform3D components as individual
/// RenderingServer geometry instances, supporting multiple LOD levels.
///
/// Each InstancedRenderer3D maps to one or many Flecs prefabs via
/// `prefabs_rendered`. For every entity that is an instance of those prefabs
/// (and has a Transform3D component), the renderer creates one
/// RenderingServer instance per LOD level and updates its transform each frame.
class InstancedRenderer3D : public godot::Node3D {
    GDCLASS(InstancedRenderer3D, godot::Node3D)

  public:
    static constexpr int MAX_LOD_LEVELS = 8;

    void set_prefabs_rendered(const godot::PackedStringArray &p_prefabs) { prefabs_rendered = p_prefabs; }
    [[nodiscard]] godot::PackedStringArray get_prefabs_rendered() const { return prefabs_rendered; }

    void set_lod_levels(const godot::TypedArray<InstancedRenderer3DLODConfiguration> &p_lod_levels) { lod_levels = p_lod_levels; }
    [[nodiscard]] godot::TypedArray<InstancedRenderer3DLODConfiguration> get_lod_levels() const { return lod_levels; }

    /// Validates configuration and emits warnings for issues.
    /// Returns true if the configuration is valid enough to render.
    [[nodiscard]] bool validate_configuration() const;

  protected:
    static void _bind_methods();

  private:
    godot::PackedStringArray prefabs_rendered;
    godot::TypedArray<InstancedRenderer3DLODConfiguration> lod_levels;
};

// Helper to register an InstancedRenderer3D node into the ECS world
void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::entity_rendering::Renderers &renderers, int &renderer_count);
