#!/usr/bin/env bash
set -e

# ─── Stagehand Unit Test Runner ──────────────────────────────────────────────
#
# Builds and runs the unit test suite (GoogleTest). Run from the project root:
#     scripts/run_unit_tests.sh [options]
#
# Options are forwarded to the GoogleTest binary:
#     --gtest_filter=PATTERN   Run only tests matching PATTERN (wildcard match)
#     --gtest_list_tests       List available tests without running them
#     --gtest_repeat=N         Repeat tests N times
#     --help                   Show GoogleTest help
#
# Environment variables:
#     CXX              C++ compiler (default: g++-15)
#     TARGET           Build target (template_debug, template_release, etc.)
#     SCONS_ARGS       Extra arguments to pass to scons

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TESTS_DIR="$PROJECT_ROOT/tests"
TEST_BINARY="$TESTS_DIR/build/stagehand_tests"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Building Stagehand unit tests..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Build the test binary
cd "$PROJECT_ROOT"

# Use the same build settings as the main build for consistency
# Default to debug build if no target is specified
if [[ -z "$TARGET" ]]; then
    SCONS_CMD="scons unit_tests debug_symbols=yes optimize=debug"
else
    SCONS_CMD="scons unit_tests target=$TARGET"
fi

[[ -n "$CXX" ]]        && SCONS_CMD="$SCONS_CMD CXX=$CXX"
[[ -n "$SCONS_ARGS" ]] && SCONS_CMD="$SCONS_CMD $SCONS_ARGS"

eval "$SCONS_CMD"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Running tests..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Run the test binary, forwarding all script arguments
exec "$TEST_BINARY" "$@"

