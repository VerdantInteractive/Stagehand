#pragma once

#include <godot_cpp/core/class_db.hpp>

using godot::ModuleInitializationLevel;

void initialize_flecs_module(ModuleInitializationLevel p_level);
void uninitialize_flecs_module(ModuleInitializationLevel p_level);
