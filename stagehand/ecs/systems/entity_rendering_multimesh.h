#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/registry.h"
#include "stagehand/ecs/components/entity_rendering.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/nodes/multi_mesh_renderer.h"
#include "stagehand/utilities/godot_hashes.h"
#include "stagehand/names.h"

using namespace stagehand::entity_rendering;

namespace stagehand::entity_rendering {
    inline flecs::system EntityRenderingMultiMesh;
}

// Buffer format: https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer
// The per - instance data size and expected data order is :
// 2D :
//     - Position : 8 floats(8 floats for Transform2D)
//     - Position + Vertex color : 12 floats(8 floats for Transform2D, 4 floats for Color)
//     - Position + Custom data : 12 floats(8 floats for Transform2D, 4 floats of custom data)
//     - Position + Vertex color + Custom data : 16 floats(8 floats for Transform2D, 4 floats for Color, 4 floats of custom data)
// 3D :
//     - Position : 12 floats(12 floats for Transform3D)
//     - Position + Vertex color : 16 floats(12 floats for Transform3D, 4 floats for Color)
//     - Position + Custom data : 16 floats(12 floats for Transform3D, 4 floats of custom data)
//     - Position + Vertex color + Custom data : 20 floats(12 floats for Transform3D, 4 floats for Color, 4 floats of custom data)
// 
// Instance transforms are in row - major order.Specifically:
// For Transform2D the float - order is : (x.x, y.x, padding_float, origin.x, x.y, y.y, padding_float, origin.y).
// For Transform3D the float - order is : (basis.x.x, basis.y.x, basis.z.x, origin.x, basis.x.y, basis.y.y, basis.z.y, origin.y, basis.x.z, basis.y.z, basis.z.z, origin.z).


// Collect instances for a single prefab and update the corresponding multimesh buffer.
// This helper builds a query specialized for the transform type (2D or 3D) and
// conditionally includes vertex colors and custom data as query terms when the renderer expects them.
template <typename TransformType>
void update_renderer_for_prefab(
    godot::RenderingServer* rendering_server,
    const MultiMeshRendererConfig& renderer)
{
    size_t floats_per_instance = 0;
    if (renderer.transform_format == godot::MultiMesh::TRANSFORM_2D) {
        floats_per_instance = 8;
    }
    else { // TRANSFORM_3D
        floats_per_instance = 12;
    }

    floats_per_instance += (renderer.use_colors ? 4 : 0) + (renderer.use_custom_data ? 4 : 0);

    size_t total_matches = 0;
    for (const auto& q : renderer.queries) {
        total_matches += static_cast<size_t>(q.count());
    }

    if (total_matches == 0 && renderer.instance_count == 0) {
        rendering_server->multimesh_set_visible_instances(renderer.rid, 0);
        return;
    }

    size_t instance_capacity_required = std::max(renderer.instance_count, total_matches);

    // Use a growth strategy (next power of 2) to avoid frequent reallocations when the instance count fluctuates.
    size_t instance_capacity = 16;
    while (instance_capacity < instance_capacity_required) {
        instance_capacity *= 2;
    }

    godot::PackedFloat32Array& buffer = g_multimesh_buffer_cache[renderer.rid];

    size_t required_size = instance_capacity * floats_per_instance;

    if (buffer.size() < required_size || buffer.size() > (required_size * 2)) {
        godot::RenderingServer::MultimeshTransformFormat transform_format =
            renderer.transform_format == godot::MultiMesh::TRANSFORM_2D
            ? godot::RenderingServer::MULTIMESH_TRANSFORM_2D
            : godot::RenderingServer::MULTIMESH_TRANSFORM_3D;

        rendering_server->multimesh_allocate_data(
            renderer.rid,
            static_cast<int32_t>(instance_capacity),
            transform_format,
            renderer.use_colors,
            renderer.use_custom_data,
            false);
        if (buffer.size() > 0) {
            godot::UtilityFunctions::push_warning(godot::String(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH) + ": Resizing buffer for RID " + godot::String::num_uint64(renderer.rid.get_id()) + " from " + godot::String::num_int64(buffer.size()) + " to " + godot::String::num_int64(required_size));
        }
        buffer.resize(required_size);
    }

    float* buffer_ptr = buffer.ptrw();
    size_t instance_count = 0;

    for (const auto& q : renderer.queries) {
        q.run([&](flecs::iter& it) {
            while (it.next()) {
                auto transform_field = it.field<const TransformType>(0);

                for (auto i : it) {
                    if (instance_count >= instance_capacity) {
                        break;
                    }

                    size_t buffer_cursor = instance_count * floats_per_instance;
                    const TransformType& transform = transform_field[i];

                    if constexpr (std::is_same_v<TransformType, Transform2D>) {
                        buffer_ptr[buffer_cursor++] = transform.columns[0].x;
                        buffer_ptr[buffer_cursor++] = transform.columns[1].x;
                        buffer_ptr[buffer_cursor++] = 0.0f;
                        buffer_ptr[buffer_cursor++] = transform.columns[2].x;
                        buffer_ptr[buffer_cursor++] = transform.columns[0].y;
                        buffer_ptr[buffer_cursor++] = transform.columns[1].y;
                        buffer_ptr[buffer_cursor++] = 0.0f;
                        buffer_ptr[buffer_cursor++] = transform.columns[2].y;
                    }
                    else if constexpr (std::is_same_v<TransformType, Transform3D>) {
                        // RenderingServer expects Transform3D data in a specific order (rows of the 3x4 matrix):
                        const Vector3& row0 = transform.basis.rows[0];
                        const Vector3& row1 = transform.basis.rows[1];
                        const Vector3& row2 = transform.basis.rows[2];

                        buffer_ptr[buffer_cursor++] = row0.x;
                        buffer_ptr[buffer_cursor++] = row1.x;
                        buffer_ptr[buffer_cursor++] = row2.x;
                        buffer_ptr[buffer_cursor++] = transform.origin.x;

                        buffer_ptr[buffer_cursor++] = row0.y;
                        buffer_ptr[buffer_cursor++] = row1.y;
                        buffer_ptr[buffer_cursor++] = row2.y;
                        buffer_ptr[buffer_cursor++] = transform.origin.y;

                        buffer_ptr[buffer_cursor++] = row0.z;
                        buffer_ptr[buffer_cursor++] = row1.z;
                        buffer_ptr[buffer_cursor++] = row2.z;
                        buffer_ptr[buffer_cursor++] = transform.origin.z;
                    }

                    int next_field_idx = 1;
                    if (renderer.use_colors) {
                        const Color& color = it.field<const Color>(next_field_idx++)[i];
                        buffer_ptr[buffer_cursor++] = color.r;
                        buffer_ptr[buffer_cursor++] = color.g;
                        buffer_ptr[buffer_cursor++] = color.b;
                        buffer_ptr[buffer_cursor++] = color.a;
                    }
                    if (renderer.use_custom_data) {
                        const CustomData& custom_data = it.field<const CustomData>(next_field_idx++)[i];
                        buffer_ptr[buffer_cursor++] = custom_data.x;
                        buffer_ptr[buffer_cursor++] = custom_data.y;
                        buffer_ptr[buffer_cursor++] = custom_data.z;
                        buffer_ptr[buffer_cursor++] = custom_data.w;
                    }
                    instance_count++;
                }
            }
        });
        if (instance_count >= instance_capacity) {
            break;
        }
    }

    rendering_server->multimesh_set_buffer(renderer.rid, buffer);
    rendering_server->multimesh_set_visible_instances(renderer.rid, static_cast<int32_t>(instance_count));
}


inline stagehand::Registry register_entity_rendering_multimesh_system([](flecs::world& world)
{
    // This system iterates over all MultiMesh renderers and updates their buffers.
    // It's designed to be efficient by using pre-built queries stored in the MultiMeshRendererConfig component.
    stagehand::entity_rendering::EntityRenderingMultiMesh = world.system(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH)
        .kind(stagehand::OnRender)
        .run([](flecs::iter& it) {

        if (!it.world().has<Renderers>())
        {
            return; // No renderers component
        }
        const Renderers& renderers = it.world().get<Renderers>();

        auto multimesh_renderers_it = renderers.renderers_by_type.find(RendererType::MultiMesh);
        if (multimesh_renderers_it == renderers.renderers_by_type.end())
        {
            return; // No multimesh renderers
        }

        godot::RenderingServer* rendering_server = godot::RenderingServer::get_singleton();
        if (!rendering_server)
        {
            godot::UtilityFunctions::push_error(godot::String(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH) + ": RenderingServer singleton not available");
            return;
        }

        for (auto& prefab_renderer_pair : multimesh_renderers_it->second)
        {
            const MultiMeshRendererConfig& renderer = prefab_renderer_pair.second;

            if (renderer.transform_format == godot::MultiMesh::TRANSFORM_2D)
            {
                update_renderer_for_prefab<Transform2D>(rendering_server, renderer);
            }
            else
            {
                update_renderer_for_prefab<Transform3D>(rendering_server, renderer);
            }
        }
    });
});
