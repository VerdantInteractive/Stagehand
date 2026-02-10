#!/usr/bin/env python

import os, sys
from SCons.Script import ARGUMENTS, SConscript

sys.path.insert(0, os.path.join(os.getcwd(), "scripts/scons_helpers"))
from submodule_check import check_and_init_submodules
from godot_project import check_and_setup_project_file_structure

check_and_init_submodules()
PROJECT_DIRECTORY = check_and_setup_project_file_structure("../../")

CPP_STANDARD = "c++23"

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPPATH are to tell the pre-processor where to look for header files
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Default to LLVM toolchain on Windows
if sys.platform.startswith("win") and "use_llvm" not in ARGUMENTS:
    ARGUMENTS["use_llvm"] = "yes"

# Only build for x86_64 on macOS
if sys.platform == "darwin" and "arch" not in ARGUMENTS:
    ARGUMENTS["arch"] = "x86_64"


env = SConscript("dependencies/godot-cpp/SConstruct")

# Optimize for modern CPUs, including BMI2 instructions for the heightmap
if env["arch"] == "x86_64":
    if env.get("is_msvc", False):
        env.Append(CCFLAGS=["/arch:AVX2"])
    else:
        env.Append(CCFLAGS=["-march=x86-64-v3"])

def find_source_files(base_dir):
    """
    Recursively finds all .cpp files and directories containing .h/.hpp files
    within a given base directory. Paths are returned in a SCons-friendly format.
    """
    cpp_files = []
    include_paths = set() # Use a set to automatically handle unique paths

    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith(".cpp"):
                cpp_files.append(os.path.join(root, f).replace("\\", "/"))
            if f.endswith((".h", ".hpp")):
                include_paths.add(root.replace("\\", "/"))

    return cpp_files, sorted(list(include_paths)) # Sort for deterministic order

# Source code paths
sources, include_paths = find_source_files("stagehand")
project_cpp_sources, project_cpp_include_paths = find_source_files(f"{PROJECT_DIRECTORY}/cpp")

env.Append(CPPPATH=["dependencies/godot-cpp/include", "dependencies/godot-cpp/gen/include", "dependencies/flecs/distr/", ".", f"{PROJECT_DIRECTORY}/cpp"])

flecs_c_source = "dependencies/flecs/distr/flecs.c"

# Clone the env for everything *outside* of godot-cpp so our flags/defines don't leak into godot-cpp builds.
project_env = env.Clone()

# Flecs build options
FLECS_COMMON_OPTS = [
    "FLECS_CPP_NO_AUTO_REGISTRATION",
    # "ecs_ftime_t=double",
]

FLECS_DEVELOPMENT_OPTS = [
    "FLECS_DEBUG",
]
FLECS_PRODUCTION_OPTS = [
    "FLECS_NDEBUG",
    "FLECS_CUSTOM_BUILD",
    "FLECS_CPP",
    "FLECS_DISABLE_COUNTERS",
    "FLECS_LOG",
    "FLECS_META",
    "FLECS_PIPELINE",
    "FLECS_SCRIPT",
    "FLECS_SYSTEM",
    "FLECS_TIMER",
]
FLECS_OPTS = FLECS_DEVELOPMENT_OPTS if env["target"] == "template_debug" else FLECS_PRODUCTION_OPTS

FLECS_WINDOWS_OPTS = [f"/D{o}" for o in (FLECS_OPTS + FLECS_COMMON_OPTS)] + ["/TC", "/DWIN32_LEAN_AND_MEAN"]
FLECS_UNIX_OPTS =    [f"-D{o}" for o in (FLECS_OPTS + FLECS_COMMON_OPTS)] + ["-std=gnu99"]

# Ensure all translation units (C and C++) see the same Flecs defines (e.g. ecs_ftime_t=double)
# Convert any "name=value" strings into (name, value) tuples so SCons emits the
# proper -D / /D forms and handles quoting correctly across platforms.
cppdefines_list = []
for opt in (FLECS_OPTS + FLECS_COMMON_OPTS):
    if "=" in opt:
        name, value = opt.split("=", 1)
        cppdefines_list.append((name, value))
    else:
        cppdefines_list.append(opt)
project_env.Append(CPPDEFINES=cppdefines_list)

def filter_cppdefines(cppdefines, remove_names):
    if cppdefines is None:
        return []
    if isinstance(cppdefines, dict):
        return {key: value for key, value in cppdefines.items() if key not in remove_names}
    filtered = []
    for define in cppdefines:
        if isinstance(define, tuple) and len(define) >= 1:
            define_name = define[0]
        else:
            define_name = define
        if define_name in remove_names:
            continue
        filtered.append(define)
    return filtered

# Flecs debug/prod configuration is controlled via FLECS_DEBUG/FLECS_NDEBUG and the NDEBUG that godot-cpp may add triggers a Flecs configuration warning.
project_env["CPPDEFINES"] = filter_cppdefines(
    project_env.get("CPPDEFINES", []),
    {"NDEBUG"},
)

# Re-add NDEBUG only for non-debug templates so standard asserts are disabled in
# production builds, without conflicting with Flecs' FLECS_DEBUG in template_debug.
if env["target"] != "template_debug":
    project_env.Append(CPPDEFINES=["NDEBUG"])

cxx_flags = []
if env["platform"] == "windows":
    if env.get("is_msvc", False):
        cxx_flags=[f"/std:c++latest"] # MSVC does not have a c++23 mode yet
        project_env.Append(LIBS=["Ws2_32", "Dbghelp"])
    else: # mingw32
        cxx_flags=[f"-std={CPP_STANDARD}"]
        project_env.Append(LIBS=["ws2_32", "dbghelp"])

    flecs_env = project_env.Clone()
    flecs_c_obj = flecs_env.SharedObject(
        target="flecs_c_obj",
        source=[flecs_c_source],
        CFLAGS=FLECS_WINDOWS_OPTS if env.get("is_msvc", False) else FLECS_UNIX_OPTS,
    )
else:
    cxx_flags=[f"-std={CPP_STANDARD}"]
    flecs_env = project_env.Clone()
    flecs_c_obj = flecs_env.SharedObject(
        target="flecs_c_obj",
        source=[flecs_c_source],
        CFLAGS=FLECS_UNIX_OPTS,
    )


project_objs = project_env.SharedObject(
    sources + project_cpp_sources,
    CXXFLAGS=project_env["CXXFLAGS"] + cxx_flags,
) + [flecs_c_obj]

if env["platform"] == "macos":
    library = project_env.SharedLibrary(
        "{}/cpp/bin/libstagehand.{}.{}.framework/libstagehand.{}.{}".format(
            PROJECT_DIRECTORY, env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=project_objs,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = project_env.StaticLibrary(
            "{}/cpp/bin/libstagehand.{}.{}.simulator.a".format(PROJECT_DIRECTORY, env["platform"], env["target"]),
            source=project_objs,
        )
    else:
        library = project_env.StaticLibrary(
            "{}/cpp/bin/libstagehand.{}.{}.a".format(PROJECT_DIRECTORY, env["platform"], env["target"]),
            source=project_objs,
        )
else:
    library = project_env.SharedLibrary(
        "{}/cpp/bin/libstagehand{}{}".format(PROJECT_DIRECTORY, env["suffix"], env["SHLIBSUFFIX"]),
        source=project_objs,
    )

Default(library)
