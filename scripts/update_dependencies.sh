#!/usr/bin/env bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "${SCRIPT_DIR}/.."

# Configurable target branches for each dependency (can be overridden via env)
: ${GODOT_CPP_BRANCH:=master}
: ${FLECS_BRANCH:=master}
: ${GOOGLETEST_BRANCH:=v1.17.x}


git fetch godot-cpp "${GODOT_CPP_BRANCH}"
git subtree pull --prefix dependencies/godot-cpp godot-cpp "${GODOT_CPP_BRANCH}" --squash

git fetch flecs "${FLECS_BRANCH}"
git subtree pull --prefix dependencies/flecs flecs "${FLECS_BRANCH}" --squash

git fetch googletest "${GOOGLETEST_BRANCH}"
git subtree pull --prefix dependencies/googletest googletest "${GOOGLETEST_BRANCH}" --squash
