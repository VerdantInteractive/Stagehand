# Stagehand: take Godot to a bigger stage.

Stagehand brings Flecs, a modern, high-performance Entity Component System to Godot, enabling large-scale simulations, complex gameplay logic, and performance-critical systems that would otherwise be impractical or impossible.

## Features

- An ECS world node to integrate seamlessly into a Godot scene
- Entity and Component authoring workflows that leverage Godot Resources and/or Flecs Script for frictionless, data-oriented design.
- Multiple integration paths for 3D/2D rendering: Instanced, MultiMesh and compute-shader based fully custom rendering pipelines.

## Getting Started

### Install the Prerequisites

You'll need some development tools installed on the system that you'll build your project:

- A Godot 4 executable
- A C++ compiler. The build system and the rest of this document assumes that you have LLVM-Clang installed.
- SCons as a build tool - install via `pip install SCons`.
- A command-line or graphical `git` client.

If not using LLVM-Clang, you will need to edit the `build_*.sh` scripts (and remember to explicitly pass if running `scons` directly) and set `use_llvm=no`.

Stagehand is a [C++ GDExtension](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/about_godot_cpp.html), and a more comprehensive set of instructions can be found in its [Getting Started section](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/gdextension_cpp_example.html) if needed.

### Add Stagehand into your Godot project

Note: On Windows, symbolic links must be enabled both at OS and git configuration level for the best development experience, and for opening the demo project or running the integration tests.

Change into your Godot project directory and then run the following commands
```
# you can also manually place the contents of the Stagehand repository into addons/stagehand
git submodule add git@github.com:VerdantInteractive/Stagehand.git addons/stagehand
git submodule update --init --recursive addons/stagehand
```

### Build it

During development iteration, perform a debug build:
```
# `addons/stagehand/scripts/build_debug.sh` also works
scons -C addons/stagehand
```

To generate optimised binaries for a release, use:
```
# `scons -C addons/stagehand target=template_release production=yes optimize=speed lto=full` also works
addons/stagehand/scripts/build_release.sh
```

### VSCode IDE integration

Make sure the following extensions are installed:
- [clangd extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
- [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)

#### Task & Launch Configuration

1. copy the`tasks.json` and `launch.json` files from `addons/stagehand/.vscode/` into the `.vscode/` directory at the project root, creating it if necessary.
2. Edit `launch.json` and replace the occurences of `demos` with `.`.
3. Edit `tasks.json` and prefix the `command` field of the "Build (Debug)" task with `addons/stagehand`, so it becomes `"command": "addons/stagehand/scripts/build_debug.sh"`.

Now you can launch the editor or the default scene of your Godot project with the debugger attached using the appropriate shortcuts in the "Run and Debug" section of VSCode.

## Usage

Place your project's C++ ECS code into the `ecs` subdirectory under your Godot project's root. You can use any file/subdirectory hierarchy within `ecs/`, but the `ecs` directory itself must be at the project root with this exact name to be picked up by the build system.

### Upgrading Stagehand

Simply run `git submodule update  --remote --recursive addons/stagehand` to pull in the latest version along with the updated dependencies (if any).

## Flecs Explorer

Flecs' built-in visualisation and statistics interface Flecs Explorer is enabled in debug builds and can be viewed at [this URL](https://www.flecs.dev/explorer/?page=stats&host=localhost) when a Godot scene with a `FlecsWorld` node is loaded.
