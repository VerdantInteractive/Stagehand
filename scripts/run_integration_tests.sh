#!/usr/bin/env bash
set -e

# ─── Stagehand Integration Test Runner ──────────────────────────────────────────────
#
# Builds and runs the unit test suite (GoogleTest). Run from the project root:
#     scripts/run_integration_tests.sh [path_to/single_scene.tscn]
#
#
# Environment variables:
#     GODOT            Path to the Godot binary

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$REPOSITORY_ROOT/tests/integration"
TESTS_DIR="$PROJECT_DIR/tests"

if [[ "$(uname)" == "Darwin" ]]; then
    : ${GODOT:=/Applications/Godot.app/Contents/MacOS/Godot}
elif [[ "$(uname)" == *"MINGW"* ]] || [[ "$(uname)" == *"MSYS"* ]] || [[ "$(uname)" == *"CYGWIN"* ]]; then
    : ${GODOT:=./bin/Godot_console.exe}
else
    : ${GODOT:=./bin/godot}  # Default to a local Godot binary if not specified
fi


echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Running Stagehand integration tests..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Find all integration test scenes
SCENE_FILES=()
while IFS= read -r scene_file; do
    SCENE_FILES+=("$scene_file")
done < <(find "$TESTS_DIR" -iname "*.tscn" | sort)

if [[ -n "$1" ]]; then
    if [[ -f "$1" ]]; then
        [[ "$1" == /* ]] && TESTS_TO_RUN=("$1") || TESTS_TO_RUN=("$(pwd)/$1")
    elif [[ -f "$TESTS_DIR/$1" ]]; then
        TESTS_TO_RUN=("$TESTS_DIR/$1")
    else
        echo "Test file '$1' not found. Running all integration tests."
        TESTS_TO_RUN=("${SCENE_FILES[@]}")
    fi
else
    TESTS_TO_RUN=("${SCENE_FILES[@]}")
fi

for test_scene in "${TESTS_TO_RUN[@]}"; do
    echo "Running test: $(basename "$test_scene")"
    "$GODOT" --headless --no-header --quit --path "${PROJECT_DIR}" --scene "$test_scene"
done
