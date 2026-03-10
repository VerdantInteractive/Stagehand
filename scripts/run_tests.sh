#!/usr/bin/env bash
set -e

# Save the current working directory and return to it on exit.
ORIGINAL_PWD="$PWD"
back_to_initial_directory() {
	cd "$ORIGINAL_PWD" || true
}
trap back_to_initial_directory EXIT

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

./run_unit_tests.py
./run_integration_tests.py
