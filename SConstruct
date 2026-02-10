#!/usr/bin/env python

CPP_STANDARD = "c++20"

import os, sys
from SCons.Script import ARGUMENTS, SConscript, Alias, Default

sys.path.insert(0, os.path.join(os.getcwd(), "scripts/scons_helpers"))
from submodule_check import check_and_init_submodules
from godot_project import check_and_setup_project_file_structure

# Ensure git submodules are initialized and updated before proceeding with the build
check_and_init_submodules()

# Check that the Godot project file structure is set up correctly and get the project directory path
PROJECT_DIRECTORY = check_and_setup_project_file_structure("../../")

# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPPATH are to tell the pre-processor where to look for header files
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Default to the LLVM/Clang toolchain
if "use_llvm" not in ARGUMENTS: ARGUMENTS["use_llvm"] = "yes"

# Only build for x86_64 on macOS
if sys.platform == "darwin" and "arch" not in ARGUMENTS: ARGUMENTS["arch"] = "x86_64"

env = SConscript("dependencies/godot-cpp/SConstruct")

# Shared build directory for object files
BUILD_DIR = "build/obj"

# Optimize for modern CPUs, including BMI2 instructions for the heightmap
if env["arch"] == "x86_64":
    if env.get("is_msvc", False):
        env.Append(CCFLAGS=["/arch:AVX2"])
    else:
        env.Append(CCFLAGS=["-march=x86-64-v3"])

def find_source_files(base_dir):
    """Recursively find .cpp files under a base directory."""
    cpp_files = []

    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith(".cpp"):
                cpp_files.append(os.path.join(root, f).replace("\\", "/"))

    return cpp_files

# Source code paths
sources = find_source_files("stagehand")
project_cpp_sources = find_source_files(f"{PROJECT_DIRECTORY}/cpp")

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
        cxx_flags=[f"/std:{CPP_STANDARD}"]
        project_env.Append(LIBS=["Ws2_32", "Dbghelp"])
    else: # mingw32
        cxx_flags=[f"-std={CPP_STANDARD}"]
        project_env.Append(LIBS=["ws2_32", "dbghelp"])

    flecs_env = project_env.Clone()
    flecs_c_obj = flecs_env.SharedObject(
        target=os.path.join(BUILD_DIR, "flecs"),
        source=[flecs_c_source],
        CFLAGS=FLECS_WINDOWS_OPTS if env.get("is_msvc", False) else FLECS_UNIX_OPTS,
    )
else:
    cxx_flags=[f"-std={CPP_STANDARD}"]
    flecs_env = project_env.Clone()
    flecs_c_obj = flecs_env.SharedObject(
        target=os.path.join(BUILD_DIR, "flecs"),
        source=[flecs_c_source],
        CFLAGS=FLECS_UNIX_OPTS,
    )

# Build stagehand sources into shared build directory
stagehand_objs = []
for src in sources:
    obj_target = os.path.join(BUILD_DIR, "stagehand", os.path.splitext(os.path.basename(src))[0])
    stagehand_objs.extend(project_env.SharedObject(
        target=obj_target,
        source=src,
        CXXFLAGS=project_env["CXXFLAGS"] + cxx_flags,
    ))

project_cpp_objs = []
for src in project_cpp_sources:
    rel_path = os.path.relpath(src, f"{PROJECT_DIRECTORY}/cpp")
    obj_target = os.path.join(BUILD_DIR, "project", os.path.splitext(rel_path)[0])
    project_cpp_objs.extend(project_env.SharedObject(
        target=obj_target,
        source=src,
        CXXFLAGS=project_env["CXXFLAGS"] + cxx_flags,
    ))

project_objs = stagehand_objs + project_cpp_objs + [flecs_c_obj]

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

def build_unit_tests(root_env, project_root, flecs_opts, cxx_flags, tests_root=None, build_dir=None, flecs_c_obj=None, stagehand_objs=None, project_env=None):
    """Build and return the unit test program."""
    from SCons.Script import ARGUMENTS, Environment, File

    target = root_env["target"]

    if tests_root is None:
        tests_dir = os.path.join(project_root, "tests")
    else:
        tests_dir = tests_root
    tests_build_dir = os.path.join(tests_dir, "build")
    output_dir = os.path.join(tests_build_dir, "stagehand_tests")
    deps_dir = os.path.join(project_root, "dependencies")
    flecs_distr = os.path.join(deps_dir, "flecs", "distr")
    gtest_dir = os.path.join(deps_dir, "googletest", "googletest")
    godotcpp_dir = os.path.join(deps_dir, "godot-cpp")

    def detect_platform_key():
        plat_arg = ARGUMENTS.get("platform", "")
        if plat_arg:
            plat = plat_arg
        else:
            if sys.platform == "darwin":
                plat = "macos"
            elif sys.platform.startswith("win"):
                plat = "windows"
            elif sys.platform.startswith("linux"):
                plat = "linux"
            else:
                plat = sys.platform

        # Map template_debug/template_release to debug/release for platform key
        target_map = {"template_debug": "debug", "template_release": "release", "editor": "debug"}
        target_key = target_map.get(target, "debug")

        arch = None
        try:
            import platform as _platform
            machine = (_platform.machine() or "").lower()
            if "arm64" in machine or "aarch64" in machine or "arm64" in ARGUMENTS.get("arch", ""):
                arch = "arm64"
            elif "rv64" in machine or "riscv" in machine:
                arch = "rv64"
            elif "64" in machine:
                arch = "x86_64"
            else:
                arch = "x86_32"
        except Exception:
            arch = None

        if plat in ("windows", "linux", "android") and arch is not None:
            return f"{plat}.{target_key}.{arch}"
        return f"{plat}.{target_key}"

    def find_gdextension_value(key):
        gdext_path = os.path.join(project_root, "stagehand.gdextension")
        if not os.path.isfile(gdext_path):
            raise FileNotFoundError(f"{gdext_path} not found")
        with open(gdext_path, "r", encoding="utf-8") as fh:
            for raw in fh:
                line = raw.strip()
                if line.startswith(key + " ="):
                    parts = line.split("=", 1)[1].strip()
                    if parts.startswith("\"") or parts.startswith("'"):
                        return parts.strip().strip("\"").strip("'")
                    return parts
        return None

    key = detect_platform_key()
    val = find_gdextension_value(key)
    if val is None:
        raise RuntimeError(
            f"Could not determine library path from stagehand.gdextension for key '{key}'"
        )

    file_name = os.path.basename(val)
    if not file_name.startswith("libstagehand"):
        raise RuntimeError(f"Unexpected stagehand library name: {file_name}")

    stagehand_suffix = file_name[len("libstagehand"):]
    for ext in (".framework", ".xcframework", ".dll", ".so", ".a", ".wasm"):
        if stagehand_suffix.endswith(ext):
            stagehand_suffix = stagehand_suffix[: -len(ext)]
            break

    lib_suffix = root_env.get("LIBSUFFIX", ".a")

    godot_lib_name = "libgodot-cpp" + stagehand_suffix + lib_suffix
    godotcpp_lib_path = os.path.join(godotcpp_dir, "bin", godot_lib_name)

    test_sources = find_source_files(tests_dir)
    gtest_source = os.path.join(gtest_dir, "src", "gtest-all.cc")

    # Clone the project environment to inherit all compiler settings, flags, and defines
    test_env = project_env.Clone()
    
    # Add test-specific include paths (prepend so they take priority)
    test_env.Prepend(CPPPATH=[
        tests_dir,
        project_root,
        os.path.join(gtest_dir, "include"),
        gtest_dir,
    ])
    
    # Ensure Flecs and Godot headers are available (already in project_env but verify)
    if flecs_distr not in test_env["CPPPATH"] and flecs_distr not in str(test_env.get("CPPPATH", [])):
        test_env.Append(CPPPATH=[flecs_distr])
    
    # GoogleTest requires exceptions
    is_msvc = root_env.get("is_msvc", False)
    if is_msvc:
        test_env.Append(CXXFLAGS=["/EHsc"])
    else:
        test_env.Append(CXXFLAGS=["-fexceptions"])
    
    # Clear OBJPREFIX for test builds
    test_env["OBJPREFIX"] = ""

    # Build GoogleTest with inherited configuration
    gtest_obj = test_env.SharedObject(
        target=os.path.join(tests_build_dir, "gtest-all"),
        source=gtest_source,
    )

    # Build test sources only (reuse stagehand and flecs objects from main build)
    test_objs = []
    for src in test_sources:
        rel = os.path.relpath(src, project_root)
        obj_path = os.path.join(tests_build_dir, rel.replace(".cpp", ""))
        test_objs.append(test_env.SharedObject(target=obj_path, source=src))

    godotcpp_lib = File(godotcpp_lib_path)
    
    # Combine test objects with reused stagehand/flecs objects from main build
    all_objs = test_objs + [gtest_obj, godotcpp_lib]
    if flecs_c_obj is not None:
        all_objs.insert(0, flecs_c_obj)
    if stagehand_objs is not None:
        all_objs = stagehand_objs + all_objs
    
    return test_env.Program(
        target=output_dir,
        source=all_objs,
    )

Default(library)

# Unit tests target
test_program = SConscript(
    "tests/SConstruct",
    exports={
        "root_env": env,
        "build_unit_tests": build_unit_tests,
        "project_root": os.path.normpath(os.path.abspath(".")),
        "build_dir": BUILD_DIR,
        "flecs_c_obj": flecs_c_obj,
        "stagehand_objs": stagehand_objs,
        "project_env": project_env,
        "flecs_opts": FLECS_OPTS + FLECS_COMMON_OPTS,
        "cxx_flags": cxx_flags,
    },
)
Alias("unit_tests", test_program)
