#!/usr/bin/env bash

git remote add -f godot-cpp https://github.com/godotengine/godot-cpp.git
git subtree add --prefix dependencies/godot-cpp godot-cpp master --squash

git remote add -f googletest https://github.com/google/googletest.git
git subtree add --prefix dependencies/googletest googletest main --squash

git remote add -f flecs git@github.com:VerdantInteractive/flecs.git
git subtree add --prefix dependencies/flecs flecs master --squash
