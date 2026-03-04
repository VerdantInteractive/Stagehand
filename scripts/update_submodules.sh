#!/usr/bin/env bash
set -e

for submodule in godot-cpp flecs googletest; do
    git submodule update  --remote --recursive "dependencies/${submodule}"
done
