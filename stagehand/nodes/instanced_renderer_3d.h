#pragma once

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "stagehand/classes/instanced_renderer_3d_lod_configuration.h"
#include "stagehand/ecs/components/rendering.h"

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

    void set_prefabs_rendered(const godot::PackedStringArray &p_prefabs);
    [[nodiscard]] godot::PackedStringArray get_prefabs_rendered() const { return prefabs_rendered; }

    void set_lod_levels(const godot::TypedArray<InstancedRenderer3DLODConfiguration> &p_lod_levels);
    [[nodiscard]] godot::TypedArray<InstancedRenderer3DLODConfiguration> get_lod_levels() const { return lod_levels; }

    void set_material(const godot::Ref<godot::Material> &p_material) { material = p_material; }
    [[nodiscard]] godot::Ref<godot::Material> get_material() const { return material; }

    void set_discovered_instance_uniforms(const godot::PackedStringArray &p_uniforms) { discovered_instance_uniforms = p_uniforms; }
    [[nodiscard]] godot::PackedStringArray get_discovered_instance_uniforms() const { return discovered_instance_uniforms; }

    /// Returns configuration warnings to display in the Godot Scene dock.
    [[nodiscard]] godot::PackedStringArray _get_configuration_warnings() const override;

    void _enter_tree() override;
    void _exit_tree() override;

    /// Returns true if the configuration is valid enough to render.
    [[nodiscard]] bool validate_configuration() const;

  protected:
    static void _bind_methods();

  private:
    godot::PackedStringArray prefabs_rendered;
    godot::TypedArray<InstancedRenderer3DLODConfiguration> lod_levels;
    godot::Ref<godot::Material> material;
    godot::PackedStringArray discovered_instance_uniforms;
};

// Helper to register an InstancedRenderer3D node into the ECS world
void register_instanced_renderer(flecs::world &world, InstancedRenderer3D *renderer, stagehand::rendering::Renderers &renderers, int &renderer_count);
