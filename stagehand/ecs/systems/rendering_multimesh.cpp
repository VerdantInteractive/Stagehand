#include "stagehand/ecs/systems/rendering_multimesh.h"

namespace stagehand::rendering {
    REGISTER([](flecs::world &world) {
        // This system iterates over all MultiMesh renderers and updates their buffers.
        // It's designed to be efficient by using pre-built queries stored in the MultiMeshRendererConfig component.
        // clang-format off
    EntityRenderingMultiMesh = world.system(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH)
        .kind(stagehand::OnRender)
        .run([](flecs::iter &it) {
            // clang-format on
            if (!it.world().has<Renderers>()) {
                return; // No renderers component
            }
            const Renderers &renderers = it.world().get<Renderers>();

            auto multimesh_renderers_it = renderers.renderers_by_type.find(RendererType::MultiMesh);
            if (multimesh_renderers_it == renderers.renderers_by_type.end()) {
                return; // No multimesh renderers
            }

            godot::RenderingServer *rendering_server = godot::RenderingServer::get_singleton();
            if (!rendering_server) {
                godot::UtilityFunctions::push_error(godot::String(stagehand::names::systems::ENTITY_RENDERING_MULTIMESH) +
                                                    ": RenderingServer singleton not available");
                return;
            }

            for (auto &prefab_renderer_pair : multimesh_renderers_it->second) {
                const MultiMeshRendererConfig &renderer = prefab_renderer_pair.second;

                if (renderer.transform_format == godot::MultiMesh::TRANSFORM_2D) {
                    update_renderer_for_prefab<Transform2D>(rendering_server, renderer);
                } else {
                    update_renderer_for_prefab<Transform3D>(rendering_server, renderer);
                }
            }
        });
    });
} // namespace stagehand::rendering
