#include "stagehand/gdextension_init.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "stagehand/editor/component_schema.h"
#include "stagehand/editor/flecs_script_editor_export_plugin.h"
#include "stagehand/nodes/instanced_renderer_3d.h"
#include "stagehand/nodes/multi_mesh_renderer.h"
#include "stagehand/resources/flecs_script.h"
#include "stagehand/resources/flecs_script_resource_format_loader.h"
#include "stagehand/world.h"

static godot::Ref<FlecsScriptResourceFormatLoader> flecs_script_loader;

void initialize_flecs_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    GDREGISTER_CLASS(stagehand::FlecsWorld);
    GDREGISTER_CLASS(stagehand::ComponentSchema);

    GDREGISTER_RUNTIME_CLASS(InstancedRenderer3D);
    GDREGISTER_CLASS(InstancedRenderer3DLODConfiguration);

    GDREGISTER_RUNTIME_CLASS(MultiMeshRenderer2D);
    GDREGISTER_RUNTIME_CLASS(MultiMeshRenderer3D);

    GDREGISTER_CLASS(FlecsScript); // Resource subclass
    GDREGISTER_CLASS(FlecsScriptResourceFormatLoader);

    if (p_level == godot::MODULE_INITIALIZATION_LEVEL_EDITOR) {
        GDREGISTER_CLASS(FlecsScriptEditorExportPlugin);
    }

    flecs_script_loader.instantiate();
    godot::ResourceLoader::get_singleton()->add_resource_format_loader(flecs_script_loader);
}

void uninitialize_flecs_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    godot::ResourceLoader::get_singleton()->remove_resource_format_loader(flecs_script_loader);
    flecs_script_loader.unref();
}

extern "C" {
GDExtensionBool GDE_EXPORT stagehand_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                  const GDExtensionClassLibraryPtr p_library,
                                                  GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_flecs_module);
    init_obj.register_terminator(uninitialize_flecs_module);
    init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
