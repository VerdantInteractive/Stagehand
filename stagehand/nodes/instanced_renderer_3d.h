#pragma once

#include <vector>

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "flecs.h"

#include "stagehand/ecs/components/entity_rendering.h"

/// Configuration for a single LOD level on an InstancedRenderer3D.
struct InstancedRendererLOD {
    godot::Ref<godot::Mesh> mesh;
    float fade_min = 0.0f;
    float fade_max = 0.0f;
    float fade_min_margin = 0.0f;
    float fade_max_margin = 0.0f;
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

    void set_lod_count(int p_count);
    [[nodiscard]] int get_lod_count() const { return static_cast<int>(lod_levels.size()); }

    void set_lod_mesh(int index, const godot::Ref<godot::Mesh> &mesh);
    [[nodiscard]] godot::Ref<godot::Mesh> get_lod_mesh(int index) const;

    void set_lod_fade_min(int index, float value);
    [[nodiscard]] float get_lod_fade_min(int index) const;

    void set_lod_fade_max(int index, float value);
    [[nodiscard]] float get_lod_fade_max(int index) const;

    void set_lod_fade_min_margin(int index, float value);
    [[nodiscard]] float get_lod_fade_min_margin(int index) const;

    void set_lod_fade_max_margin(int index, float value);
    [[nodiscard]] float get_lod_fade_max_margin(int index) const;

    [[nodiscard]] const std::vector<InstancedRendererLOD> &get_lod_levels() const { return lod_levels; }

    /// Validates configuration and emits warnings for issues.
    /// Returns true if the configuration is valid enough to render.
    [[nodiscard]] bool validate_configuration() const;

  protected:
    static void _bind_methods();
    bool _set(const godot::StringName &p_name, const godot::Variant &p_value);
    bool _get(const godot::StringName &p_name, godot::Variant &r_ret) const;
    void _get_property_list(godot::List<godot::PropertyInfo> *p_list) const;

  private:
    godot::PackedStringArray prefabs_rendered;
    std::vector<InstancedRendererLOD> lod_levels;
};

// Helper to register an InstancedRenderer3D node into the ECS world
void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::entity_rendering::Renderers &renderers, int &renderer_count);
