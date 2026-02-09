#pragma once

#include <unordered_map>

#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/binder_common.hpp>

#include <flecs.h>
#include "stagehand/utilities/godot_hashes.h"

enum MultiMeshDrawOrder {
    MULTIMESH_DRAW_ORDER_NONE = 0,
    MULTIMESH_DRAW_ORDER_X = 1,
    MULTIMESH_DRAW_ORDER_Y = 2,
    MULTIMESH_DRAW_ORDER_Z = 3,
};

// Forward declaration for the component struct used in registration
namespace stagehand { namespace entity_rendering { struct Renderers; } }

// Global cache for multimesh buffers to avoid reallocation
extern std::unordered_map<godot::RID, godot::PackedFloat32Array> g_multimesh_buffer_cache;

// Helper to register a Godot MultiMesh node into the ECS world
template <typename T>
void register_multimesh_renderer(flecs::world& world, T* renderer, stagehand::entity_rendering::Renderers& renderers, int& renderer_count);

template <typename T>
class MultiMeshRendererBase : public T {
public:
    void set_prefabs_rendered(const godot::PackedStringArray& p_prefabs) {
        prefabs_rendered = p_prefabs;
    }

    [[nodiscard]] godot::PackedStringArray get_prefabs_rendered() const {
        return prefabs_rendered;
    }

    void set_draw_order(MultiMeshDrawOrder p_draw_order) {
        draw_order = p_draw_order;
    }

    [[nodiscard]] MultiMeshDrawOrder get_draw_order() const {
        return draw_order;
    }

private:
    godot::PackedStringArray prefabs_rendered;
    MultiMeshDrawOrder draw_order = MULTIMESH_DRAW_ORDER_NONE;
};

class MultiMeshRenderer2D : public MultiMeshRendererBase<godot::MultiMeshInstance2D> {
    GDCLASS(MultiMeshRenderer2D, godot::MultiMeshInstance2D)

public:
protected:
    static void _bind_methods();
};

class MultiMeshRenderer3D : public MultiMeshRendererBase<godot::MultiMeshInstance3D> {
    GDCLASS(MultiMeshRenderer3D, godot::MultiMeshInstance3D)

public:
protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(MultiMeshDrawOrder);
