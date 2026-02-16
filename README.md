# Stagehand: take Godot to a bigger stage.

Stagehand brings Flecs, a modern, high-performance Entity Component System to Godot, enabling large-scale simulations, complex gameplay logic, and performance-critical systems that would otherwise be impractical or impossible.

## Features

- An ECS world node to integrate seamlessly into a Godot scene
- Entity and Component authoring workflows that leverage Godot Resources and/or Flecs Script for frictionless, data-oriented design.
- Multiple integration paths for 3D/2D rendering: Instanced, MultiMesh and compute-shader based fully custom rendering pipelines.

## Requirements

TBC

## Usage

```
git remote add -f stagehand https://github.com/VerdantInteractive/Stagehand.git
git subtree add --prefix addons/stagehand stagehand alpha --squash

scons -C addons/stagehand
```

## Flecs Explorer

Flecs' built-in visualisation and statistics interface Flecs Explorer is enabled and can be viewed at [this URL](https://www.flecs.dev/explorer/?page=stats&host=localhost) whilst the simulation is running.

## Demo Video

https://github.com/user-attachments/assets/c3863c63-0730-4891-a293-2de0180505e2
