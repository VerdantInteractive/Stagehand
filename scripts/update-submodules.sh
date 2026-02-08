#!/usr/bin/env bash

for submodule in godot-cpp flecs; do
    git submodule update --remote "${submodule}"
done
