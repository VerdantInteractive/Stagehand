#!/usr/bin/env python

import os, sys
import glob
from SCons.Script import ARGUMENTS, SConscript, Alias, Default

sys.path.insert(0, os.path.join(os.getcwd(), "scripts/scons_helpers"))
from submodule_check import check_and_init_submodules
from godot_project import check_and_setup_project_file_structure

check_and_init_submodules()
PROJECT_DIRECTORY = check_and_setup_project_file_structure("../../")

CPP_STANDARD = "c++23"

# Build configuration

# Default to LLVM toolchain on Windows
if sys.platform.startswith("win") and "use_llvm" not in ARGUMENTS:
    ARGUMENTS["use_llvm"] = "yes"

# Only build for x86_64 on macOS
if sys.platform == "darwin" and "arch" not in ARGUMENTS:
    ARGUMENTS["arch"] = "x86_64"


env = SConscript("dependencies/godot-cpp/SConstruct")
env["CPP_STANDARD"] = CPP_STANDARD

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

def build_unit_tests(root_env, project_root, tests_root=None):
    """Build and return the unit test program."""
    from SCons.Script import ARGUMENTS, Environment, File

    release_build = int(ARGUMENTS.get("release", 0))
    cpp_std = root_env.get("CPP_STANDARD", CPP_STANDARD)

    if tests_root is None:
        tests_dir = os.path.join(project_root, "tests")
    else:
        tests_dir = tests_root
    build_dir = os.path.join(tests_dir, "build")
    output_dir = os.path.join(build_dir, "stagehand_tests")
    stagehand_dir = os.path.join(project_root, "stagehand")
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

        target = "debug" if not int(ARGUMENTS.get("release", 0)) else "release"

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
            return f"{plat}.{target}.{arch}"
        return f"{plat}.{target}"

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
    stagehand_sources = [
        os.path.join(stagehand_dir, "registry.cpp"),
    ]
    flecs_source = os.path.join(flecs_distr, "flecs.c")
    gtest_source = os.path.join(gtest_dir, "src", "gtest-all.cc")
    all_cpp_sources = test_sources + stagehand_sources

    is_msvc_toolchain = root_env.get("is_msvc", False)
    if is_msvc_toolchain:
        cxx_flags = ["/std:c++latest", "/W4", "/EHsc"]
        c_flags = ["/TC"]
        cc_compiler = root_env.get("CXX", root_env.get("CXXCOM", "cl"))
        cxx_compiler = root_env.get("CXX", root_env.get("CXXCOM", "cl"))
    else:
        cxx_flags = [f"-std={cpp_std}", "-Wall", "-Wextra", "-Wpedantic", "-Wno-unused-parameter", "-fexceptions"]
        c_flags = ["-std=gnu99"]
        cxx_compiler = root_env.get("CXX", "g++")
        cc_compiler = str(cxx_compiler).replace("g++", "gcc").replace("clang++", "clang")

    test_env = Environment(
        CXX=cxx_compiler,
        CC=cc_compiler,
        CXXFLAGS=cxx_flags,
        CFLAGS=c_flags,
        CPPPATH=[
            tests_dir,
            project_root,
            flecs_distr,
            os.path.join(gtest_dir, "include"),
            gtest_dir,
            os.path.join(godotcpp_dir, "include"),
            os.path.join(godotcpp_dir, "gen", "include"),
        ],
        CPPDEFINES=[
            "FLECS_CPP_NO_AUTO_REGISTRATION",
            "FLECS_DEBUG",
        ],
        OBJPREFIX="",
    )

    if sys.platform.startswith("win"):
        test_env.Append(LIBS=["Ws2_32", "Dbghelp"])

    if sys.platform == "darwin":
        for var in ["CCFLAGS", "LINKFLAGS"]:
            if var in test_env:
                new_flags = []
                skip = False
                for flag in test_env[var]:
                    if skip:
                        skip = False
                        continue
                    if flag == "-arch":
                        skip = True
                        continue
                    new_flags.append(flag)
                test_env[var] = new_flags
        test_env.Append(LINKFLAGS=["-Wl,-undefined,dynamic_lookup"])

    if release_build:
        test_env.Append(CXXFLAGS=["-O2", "-DNDEBUG"])
        test_env.Append(CFLAGS=["-O2", "-DNDEBUG"])
    else:
        test_env.Append(CXXFLAGS=["-g", "-O0"])
        test_env.Append(CFLAGS=["-g", "-O0"])

    flecs_obj = test_env.Object(
        target=os.path.join(build_dir, "flecs"),
        source=flecs_source,
    )
    gtest_obj = test_env.Object(
        target=os.path.join(build_dir, "gtest-all"),
        source=gtest_source,
    )

    cpp_objs = []
    for src in all_cpp_sources:
        rel = os.path.relpath(src, project_root)
        obj_path = os.path.join(build_dir, rel.replace(".cpp", ""))
        cpp_objs.append(test_env.Object(target=obj_path, source=src))

    godotcpp_lib = File(godotcpp_lib_path)
    return test_env.Program(
        target=output_dir,
        source=cpp_objs + [flecs_obj, gtest_obj, godotcpp_lib],
    )

Default(library)

# Unit tests target
test_program = SConscript(
    "tests/SConstruct",
    exports={
        "root_env": env,
        "build_unit_tests": build_unit_tests,
        "project_root": os.path.normpath(os.path.abspath(".")),
    },
)
Alias("unit_tests", test_program)
