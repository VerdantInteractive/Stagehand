#include "stagehand/gdextension_init.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "stagehand/nodes/instanced_renderer_3d.h"
#include "stagehand/nodes/multi_mesh_renderer.h"
#include "stagehand/resources/prefab.h"
#include "stagehand/world.h"

using godot::MODULE_INITIALIZATION_LEVEL_SCENE;
using godot::ModuleInitializationLevel;

void initialize_flecs_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    GDREGISTER_RUNTIME_CLASS(stagehand::FlecsWorld);
    GDREGISTER_CLASS(stagehand::Prefab);

    GDREGISTER_RUNTIME_CLASS(InstancedRenderer3D);
    GDREGISTER_CLASS(InstancedRenderer3DLODConfiguration);

    GDREGISTER_RUNTIME_CLASS(MultiMeshRenderer2D);
    GDREGISTER_RUNTIME_CLASS(MultiMeshRenderer3D);
}

void uninitialize_flecs_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT stagehand_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                  const GDExtensionClassLibraryPtr p_library,
                                                  GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_flecs_module);
    init_obj.register_terminator(uninitialize_flecs_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
