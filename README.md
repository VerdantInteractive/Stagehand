<p align="center">
  <img src="assets/images/logo.svg" alt="Stagehand logo" width="220" />
</p>
<p align="center">
  <a href="https://github.com/VerdantInteractive/Stagehand/actions/workflows/build.yml"><img src="https://github.com/VerdantInteractive/Stagehand/actions/workflows/build.yml/badge.svg" alt="Build"></a>
  &nbsp;&nbsp;&nbsp;
  <a href="https://github.com/VerdantInteractive/Stagehand/actions/workflows/unit-tests.yml"><img src="https://github.com/VerdantInteractive/Stagehand/actions/workflows/unit-tests.yml/badge.svg" alt="Unit Tests"></a>
  &nbsp;&nbsp;&nbsp;
  <a href="https://github.com/VerdantInteractive/Stagehand/actions/workflows/integration-tests.yml"><img src="https://github.com/VerdantInteractive/Stagehand/actions/workflows/integration-tests.yml/badge.svg" alt="Integration Tests"></a>
</p>

# Stagehand: take Godot to a bigger stage.

Stagehand brings the [Flecs](https://www.flecs.dev) Entity Component System to Godot, enabling large-scale, complex gameplay logic and simulations.

You write your performance-critical code in C++, while still having the freedom to choose where to use Godot's scripting languages. Stagehand integrates deeply with the Godot engine and has facilities that enhance developer ergonomics.

The `FlecsWorld` node <img src="assets/node_icons/FlecsWorld.svg" alt="FlecsWorld" width="16" /> seamlessly integrates an ECS world into Godot's scene hierarchy and lifecycle.

`InstancedRenderer3D`<img src="assets/node_icons/InstancedRenderer3D.svg" alt="InstancedRenderer3D" width="16" />, `MultiMeshRenderer2D`<img src="assets/node_icons/MultiMeshRenderer2D.svg" alt="MultiMeshRenderer2D" width="16" />, `MultiMeshRenderer3D`<img src="assets/node_icons/MultiMeshRenderer3D.svg" alt="MultiMeshRenderer3D" width="16" /> and `ComputeRenderer`<img src="assets/node_icons/ComputeRenderer.svg" alt="ComputeRenderer" width="16" /> (WIP) nodes provide high-performance bridges between the engine and ECS.

For defining and registering ECS components without any boilerplate, a variety of macros are provided for all Godot variants in addition to  C++ primitives and collections (vector & array) - see [the Components manual](documentation/Components.md). Change detection can also be enabled on each component, which allows writing efficient systems that avoid redundant work via simple inclusion of the desired tags in queries - see the [Change Detection manual](documentation/ChangeDetection.md) for details.

For two-way data flow between ECS and the engine, Godot Signals, Flecs events are first-class citizens wrapped in helper methods - see the [Signals & Events manual](documentation/SignalsEvents.md). `FlecsWorld` also provides methods to read and write data of both entity and world (singleton) components.

Flecs Script is supported with an automatic loader that is also Flecs module-aware. 

🚧 **Early Development Notice** 🚧  
Stagehand is in the early stages of development. APIs may change, features may be incomplete, and documentation is still being written. Use at your own risk and expect breaking changes.

## Roadmap

### alpha (current)

Most of the core features are already implemented.

### beta (Q2 2026)

- ComputeRenderer: Build fully custom rendering pipelines with compute shaders that tightly integrate with ECS code
- Documentation: Complete set of manuals and other documentation.
- Demo #3 with InstancedRenderer3D.

### v1.0 (Q3 2026)
- Entity Composer: Design entity (prefab) hierarchies visually in the Godot editor using graphs, similar to a visual shader.
- Demo #4 with ComputeRenderer.

## Demos

Stagehand currently ships with two demos that cover different aspects of the framework: a compact, data-oriented simulation and a full single-level game with deeper Godot scene integration and orchestration. These are excellent for learning the framework's workflows and patterns.

The demos are contained in a single Godot project in the `demos` subdirectory, which can be imported into the Project Manager. Remember to build the binaries for them to work after a fresh checkout - see the Getting Started section below for details.

### Game of Life

![Demo-GameOfLife](https://github.com/user-attachments/assets/7d51182b-b183-44ee-b507-c917e866c6a0)

Implements Conway's Game of Life in ECS.

Demonstrates `FlecsWorld` node properties and the `WorldConfiguration` singleton, module loading, tag components, prefab-based entity creation, cached queries, scene-child access from ECS, ordered system phases, modeling grid cells as entities, precomputing neighbour references, using tags for state transitions, optimisation via selective matching of components in systems, rendering ECS state into a Godot texture each frame.

### Surwave

![Demo-Surwave](https://github.com/user-attachments/assets/d0df9c47-cd29-4496-86dd-635762e51540)

A 2D survivor-style action game where thousands of enemies chase the player with ECS-driven behaviour and gameplay systems. An older version of the game is [hosted on itch.io](https://60k41p.itch.io/surwave).

Demonstrates mixing GDScript orchestration, UI and audio logic with C++ ECS code, `MultiMeshRenderer2D`, Flecs Script loading, prefab authoring, hierarchy and inheritance, per-frame ECS->Godot data sync with dictionary-backed event payloads.

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

### Code Layout

Place your project's C++ ECS code into the `ecs` subdirectory under your Godot project's root. You can use any file/subdirectory hierarchy within `ecs/`, but the `ecs` directory itself must be at the project root with this exact name to be picked up by the build system.

### Upgrading Stagehand

Simply run `git submodule update  --remote --recursive addons/stagehand` to pull in the latest version along with the updated dependencies (if any).

### Flecs Explorer

Flecs' built-in visualisation and statistics interface Flecs Explorer is enabled in debug builds and can be viewed at [this URL](https://www.flecs.dev/explorer/?page=stats&host=localhost) when a Godot scene with a `FlecsWorld` node is loaded.

### VSCode IDE integration

Make sure the following extensions are installed:
- [clangd extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
- [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)

#### Task & Launch Configuration

1. copy the`tasks.json` and `launch.json` files from `addons/stagehand/.vscode/` into the `.vscode/` directory at the project root, creating it if necessary.
2. Edit `launch.json` and replace the occurences of `demos` with `.`.
3. Edit `tasks.json` and prefix the `command` field of the "Build (Debug)" task with `addons/stagehand`, so it becomes `"command": "addons/stagehand/scripts/build_debug.sh"`.

Now you can launch the editor or the default scene of your Godot project with the debugger attached using the appropriate shortcuts in the "Run and Debug" section of VSCode.
