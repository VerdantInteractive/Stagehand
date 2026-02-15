#include "stagehand/nodes/multi_mesh_renderer.h"

#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/systems/entity_rendering_multimesh.h"

std::unordered_map<godot::RID, godot::PackedFloat32Array> g_multimesh_buffer_cache;

template <MultiMeshRendererType T>
void register_multimesh_renderer(flecs::world &world, T *renderer, stagehand::entity_rendering::Renderers &renderers, int &renderer_count) {
    godot::RID multimesh_rid;
    godot::MultiMesh *multimesh = nullptr;
    const godot::PackedStringArray prefabs = renderer->get_prefabs_rendered();
    char sort_axis = '\0';

    const godot::Ref<godot::MultiMesh> mm = renderer->get_multimesh();
    if (mm.is_valid()) {
        multimesh = mm.ptr();
    }
    switch (renderer->get_draw_order()) {
    case MULTIMESH_DRAW_ORDER_X:
        sort_axis = 'x';
        break;
    case MULTIMESH_DRAW_ORDER_Y:
        sort_axis = 'y';
        break;
    default:
        if constexpr (std::is_same_v<T, MultiMeshRenderer3D>) {
            if (renderer->get_draw_order() == MULTIMESH_DRAW_ORDER_Z)
                sort_axis = 'z';
        }
        break;
    }

    if (multimesh) {
        multimesh_rid = multimesh->get_rid();
        // Pre-populate the buffer cache to avoid the initial resizing
        g_multimesh_buffer_cache[multimesh_rid] = multimesh->get_buffer();
    } else {
        godot::UtilityFunctions::push_warning(renderer->get_class() + godot::String(" node has no MultiMesh resource assigned."));
        return;
    }

    if (prefabs.is_empty()) {
        godot::UtilityFunctions::push_warning(godot::String("Child node '") + renderer->get_name() + "' has empty 'prefabs_rendered'.");
        return;
    }

    auto &renderer_map = renderers.renderers_by_type[stagehand::entity_rendering::RendererType::MultiMesh];
    // Use RID directly as the key. try_emplace will create a new renderer only if one for this RID doesn't exist.
    auto [it, inserted] = renderer_map.try_emplace(multimesh_rid);
    if (inserted) {
        it->second.rid = multimesh_rid;
        it->second.transform_format = multimesh->get_transform_format();
        it->second.use_colors = multimesh->is_using_colors();
        it->second.use_custom_data = multimesh->is_using_custom_data();
        it->second.instance_count = multimesh->get_instance_count();
        it->second.visible_instance_count = multimesh->get_visible_instance_count();
        renderer_count++;
    }
    stagehand::entity_rendering::MultiMeshRendererConfig *mm_renderer = &it->second;

    // Build a single query for all prefabs associated with this renderer.
    // This ensures that entities from different prefabs are sorted together.
    auto query = world.query_builder();

    using TransformType = std::conditional_t<std::is_same_v<T, MultiMeshRenderer2D>, Transform2D, Transform3D>;

    query.with<const TransformType>();

    if (sort_axis != '\0') {
        switch (sort_axis) {
        case 'x':
            query.order_by<TransformType>([](flecs::entity_t, const TransformType *t1, flecs::entity_t, const TransformType *t2) {
                if constexpr (std::is_same_v<T, MultiMeshRenderer2D>) {
                    const auto o1 = t1->get_origin();
                    const auto o2 = t2->get_origin();
                    if (o1.x > o2.x)
                        return 1;
                    if (o1.x < o2.x)
                        return -1;
                    return 0;
                } else {
                    if (t1->origin.x > t2->origin.x)
                        return 1;
                    if (t1->origin.x < t2->origin.x)
                        return -1;
                    return 0;
                }
            });
            break;
        case 'y':
            query.order_by<TransformType>([](flecs::entity_t, const TransformType *t1, flecs::entity_t, const TransformType *t2) {
                if constexpr (std::is_same_v<T, MultiMeshRenderer2D>) {
                    const auto o1 = t1->get_origin();
                    const auto o2 = t2->get_origin();
                    if (o1.y > o2.y)
                        return 1;
                    if (o1.y < o2.y)
                        return -1;
                    return 0;
                } else {
                    if (t1->origin.y > t2->origin.y)
                        return 1;
                    if (t1->origin.y < t2->origin.y)
                        return -1;
                    return 0;
                }
            });
            break;
        case 'z':
            if constexpr (std::is_same_v<T, MultiMeshRenderer3D>) {
                query.order_by<TransformType>([](flecs::entity_t, const TransformType *t1, flecs::entity_t, const TransformType *t2) {
                    if (t1->origin.z > t2->origin.z)
                        return 1;
                    if (t1->origin.z < t2->origin.z)
                        return -1;
                    return 0;
                });
            }
            break;
        }
    }

    if (multimesh->is_using_colors()) {
        query.with<const Color>();
    }
    if (multimesh->is_using_custom_data()) {
        query.with<const stagehand::entity_rendering::CustomData>();
    }

    // Chain prefabs with the OR operator. The logic is to add `.or_()` to all but the last term.
    int prefab_count = prefabs.size();
    for (int j = 0; j < prefab_count; ++j) {
        godot::String prefab_name = prefabs[j];
        std::string prefab_name_str = prefab_name.utf8().get_data();
        query.with(flecs::IsA, world.lookup(prefab_name_str.c_str()));
        if (j < prefab_count - 1) {
            query.or_();
        }
    }

    mm_renderer->queries.push_back(query.build());
}

template void register_multimesh_renderer<MultiMeshRenderer2D>(flecs::world &, MultiMeshRenderer2D *, stagehand::entity_rendering::Renderers &, int &);

template void register_multimesh_renderer<MultiMeshRenderer3D>(flecs::world &, MultiMeshRenderer3D *, stagehand::entity_rendering::Renderers &, int &);

template <typename T> void bind_multimesh_renderer_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("set_prefabs_rendered", "prefabs"),
                                static_cast<void (T::*)(const godot::PackedStringArray &)>(&T::set_prefabs_rendered));
    godot::ClassDB::bind_method(godot::D_METHOD("get_prefabs_rendered"), static_cast<godot::PackedStringArray (T::*)() const>(&T::get_prefabs_rendered));
    godot::ClassDB::bind_method(godot::D_METHOD("set_draw_order", "draw_order"), static_cast<void (T::*)(MultiMeshDrawOrder)>(&T::set_draw_order));
    godot::ClassDB::bind_method(godot::D_METHOD("get_draw_order"), static_cast<MultiMeshDrawOrder (T::*)() const>(&T::get_draw_order));

    godot::ClassDB::add_property(T::get_class_static(), godot::PropertyInfo(godot::Variant::PACKED_STRING_ARRAY, "prefabs_rendered"), "set_prefabs_rendered",
                                 "get_prefabs_rendered");

    if constexpr (std::is_same_v<T, MultiMeshRenderer3D>) {
        godot::ClassDB::add_property(T::get_class_static(), godot::PropertyInfo(godot::Variant::INT, "draw_order", godot::PROPERTY_HINT_ENUM, "None,X,Y,Z"),
                                     "set_draw_order", "get_draw_order");
    } else {
        godot::ClassDB::add_property(T::get_class_static(), godot::PropertyInfo(godot::Variant::INT, "draw_order", godot::PROPERTY_HINT_ENUM, "None,X,Y"),
                                     "set_draw_order", "get_draw_order");
    }

    godot::ClassDB::bind_integer_constant(T::get_class_static(), godot::StringName(), "MULTIMESH_DRAW_ORDER_NONE", MULTIMESH_DRAW_ORDER_NONE);
    godot::ClassDB::bind_integer_constant(T::get_class_static(), godot::StringName(), "MULTIMESH_DRAW_ORDER_X", MULTIMESH_DRAW_ORDER_X);
    godot::ClassDB::bind_integer_constant(T::get_class_static(), godot::StringName(), "MULTIMESH_DRAW_ORDER_Y", MULTIMESH_DRAW_ORDER_Y);

    if constexpr (std::is_same_v<T, MultiMeshRenderer3D>) {
        godot::ClassDB::bind_integer_constant(T::get_class_static(), godot::StringName(), "MULTIMESH_DRAW_ORDER_Z", MULTIMESH_DRAW_ORDER_Z);
    }
}

void MultiMeshRenderer2D::_bind_methods() { bind_multimesh_renderer_methods<MultiMeshRenderer2D>(); }

void MultiMeshRenderer3D::_bind_methods() { bind_multimesh_renderer_methods<MultiMeshRenderer3D>(); }
