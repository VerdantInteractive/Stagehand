#!/usr/bin/env bash

git fetch godot-cpp master
git subtree pull --prefix dependencies/godot-cpp godot-cpp master --squash

git fetch flecs master
git subtree pull --prefix dependencies/flecs flecs master --squash

git fetch googletest v1.17.x
git subtree pull --prefix dependencies/googletest googletest v1.17.x --squash
