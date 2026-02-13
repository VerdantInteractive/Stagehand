#!/usr/bin/env bash
set -e

# ─── Stagehand Integration Test Runner ──────────────────────────────────────────────
#
# Builds and runs the unit test suite (GoogleTest). Run from the project root:
#     scripts/run_integration_tests.sh [options] [path_to/single_scene.tscn]
#
# Options:
#     -q, --quiet      Suppress output (default)
#     -v, --verbose    Show output
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
    : ${GODOT:=Godot_console.exe}
else
    : ${GODOT:=./bin/godot}  # Default to a local Godot binary if not specified
fi

if [[ ! -f "$GODOT" ]] && ! command -v "$GODOT" >/dev/null 2>&1; then
    echo "Error: Godot binary not found at '$GODOT'" >&2
    exit 1
fi

# Parse arguments
QUIET=true
POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -q|--quiet)
            QUIET=true
            shift
            ;;
        -v|--verbose)
            QUIET=false
            shift
            ;;
        *)
            POSITIONAL_ARGS+=("$1")
            shift
            ;;
    esac
done

set -- "${POSITIONAL_ARGS[@]}"

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
        echo "Test scene file '$1' not found. Running all integration test scenes."
        TESTS_TO_RUN=("${SCENE_FILES[@]}")
    fi
else
    TESTS_TO_RUN=("${SCENE_FILES[@]}")
fi

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

for test_scene in "${TESTS_TO_RUN[@]}"; do
    ((TOTAL_TESTS++))
    TEST_NAME=$(basename "$test_scene")

    if [ "$QUIET" = true ]; then
        echo -n "Running test: $TEST_NAME ... "
        if "$GODOT" --headless --no-header --quit --path "${PROJECT_DIR}" --scene "$test_scene" > /dev/null 2>&1; then
            echo "PASSED"
            ((PASSED_TESTS++))
        else
            echo "FAILED"
            echo "Re-running test $TEST_NAME with output..."
            "$GODOT" --headless --no-header --quit --path "${PROJECT_DIR}" --scene "$test_scene" || true
            ((FAILED_TESTS++))
            break
        fi
    else
        echo "Running test: $TEST_NAME"
        if "$GODOT" --headless --no-header --quit --path "${PROJECT_DIR}" --scene "$test_scene"; then
            ((PASSED_TESTS++))
        else
            ((FAILED_TESTS++))
            break
        fi
    fi
done

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Summary"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Total:  $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"

if [ "$FAILED_TESTS" -gt 0 ]; then
    exit 1
fi
