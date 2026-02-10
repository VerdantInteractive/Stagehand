#!/usr/bin/env bash

for submodule in godot-cpp flecs googletest; do
    git submodule update  --remote --recursive "dependencies/${submodule}"
done
